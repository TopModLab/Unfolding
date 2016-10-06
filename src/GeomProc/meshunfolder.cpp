#include "GeomProc/meshunfolder.h"
#include "HDS/hds_mesh.h"

#include "Utils/utils.h"
#include "Utils/mathutils.h"

#include <QDebug>

MeshUnfolder* MeshUnfolder::instance = nullptr;

MeshUnfolder::MeshUnfolder()
{
}

MeshUnfolder* MeshUnfolder::getInstance()
{
	if (!instance)
	{
		instance = new MeshUnfolder;
	}

	return instance;
}

void MeshUnfolder::unfoldFace(hdsid_t fprev, hdsid_t fcur,
	HDS_Mesh *unfolded_mesh, const HDS_Mesh *ref_mesh,
	const QVector3D &uvec, const QVector3D &vvec)
{
	typedef HDS_Face face_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_HalfEdge he_t;

	auto &vertSet = unfolded_mesh->vertSet;
	auto &heSet   = unfolded_mesh->heSet;
	auto &faceSet = unfolded_mesh->faceSet;
	auto &refVertSet = ref_mesh->vertSet;
	auto &refHeSet   = ref_mesh->heSet;
	auto &refFaceSet = ref_mesh->faceSet;

	// use the previous face as reference, expand current face
	face_t *face_prev = &faceSet[fprev];
	face_t *face_cur = &faceSet[fcur];

	// print the corners of the previous face
	//auto corners_prev = unfolded_mesh->faceCorners(fprev);
	//auto corners_prev = face_prev->corners();

	// find the shared half edge
	he_t *he_share = &heSet[face_prev->heid];
	do {
		if (he_share->flip()->fid == face_cur->index)
		{
			break;
		}
		he_share = he_share->next();
	} while( he_share->index != face_prev->heid );

	// change the position of the vertices in the current face, except for the end points
	// of he_share
	vert_t *vs = &vertSet[he_share->vid];
	vert_t *ve = &vertSet[he_share->flip()->vid];

	// compute the spanning vectors for the unfolded mesh
	QVector3D xvec = vs->pos - ve->pos;
	xvec.normalize();
	QVector3D yvec = uvec - QVector3D::dotProduct(uvec, xvec) * xvec;
	const qreal eps = 1e-6;
	// To avoid precision issue, check if yvec is too short
	if( yvec.length() < eps )
	{
		yvec = vvec - QVector3D::dotProduct(vvec, xvec) * xvec;
	}
	yvec.normalize();
	// TODO: replace by hds_mesh function
	QVector3D cface_prev;// = face_prev->center();
	if (QVector3D::dotProduct(cface_prev - vs->pos, yvec) > 0)
	{
		yvec = -yvec;
	}

	// shared edge in the reference mesh
	auto he_share_ref = &refHeSet[he_share->index];
	auto face_cur_ref = &refFaceSet[fcur];
	auto vs_ref = &refVertSet[he_share_ref->vid];
	auto ve_ref = &refVertSet[he_share_ref->flip()->vid];

	// compute the spanning vectors for the face in the reference mesh
	QVector3D cface_ref = unfolded_mesh->faceCenter(fcur);//face_cur_ref->center();

	QVector3D xvec_ref = vs_ref->pos - ve_ref->pos;
	xvec_ref.normalize();
	QVector3D yvec_ref = (cface_ref - vs_ref->pos) - QVector3D::dotProduct(cface_ref - vs_ref->pos, xvec_ref) * xvec_ref;
	yvec_ref.normalize();

	he_t *he = he_share->flip();
	he_t *curHE = he;
	do {
		if( curHE->vid == vs->index || curHE->vid == ve->index )
		{
			//continue;
			// nothing to do, these two should not be modified
		}
		else
		{
			// compute the new position of the vertex
			vert_t *v = &vertSet[curHE->vid];
			auto v_ref = &refVertSet[curHE->vid];

			// compute the coordinates of this vertex using the reference mesh
			QVector3D dvec_ref = v_ref->pos - vs_ref->pos;
			qreal xcoord = QVector3D::dotProduct(dvec_ref, xvec_ref);
			qreal ycoord = QVector3D::dotProduct(dvec_ref, yvec_ref);
			
			v->pos = vs->pos + xcoord * xvec + ycoord * yvec;

		}
		curHE = curHE->next();
	} while( curHE != he );
}

bool MeshUnfolder::unfoldable(const HDS_Mesh* ref_mesh)
{
	// for each vertex in the cutted_mesh, check the condition
	auto isBadVertex = [&](const HDS_Vertex &v) -> bool {
		auto &vertSet = ref_mesh->vertSet;
		auto &heSet   = ref_mesh->heSet;
		auto &faceSet = ref_mesh->faceSet;
		vector<double> sums;
		double sum = 0;
		auto he = &heSet[v.heid];
		auto curHE = he->flip()->next();
		bool hasCutFace = false;
		do {
			if( faceSet[curHE->fid].isCutFace ) {
				QVector3D v1 = vertSet[he->flip()->vid].pos - vertSet[he->vid].pos;
				QVector3D v2 = vertSet[curHE->flip()->vid].pos
							 - vertSet[curHE->vid].pos;
				double nv1pnv2 = v1.length() * v2.length();
				double inv_nv1pnv2 = 1.0 / nv1pnv2;
				double cosVal = QVector3D::dotProduct(v1, v2) * inv_nv1pnv2;
				double angle = acos(cosVal);
				sum += angle;
			}
			else hasCutFace = true;

			he = curHE;
			curHE = he->flip()->next();
		} while(he->index != v.heid);

		// The sum of angles of an unfoldable vertex is smaller than pi*2 with CutFace
		// Or equal to 2*pi without cutface
		return sum > (PI2 + PI_EPS) || (sum < (PI2 - PI_EPS) && !hasCutFace);
	};
	if( any_of(ref_mesh->vertSet.begin(), ref_mesh->vertSet.end(), isBadVertex) ) return false;
	else return true;
}

void MeshUnfolder::reset_layout(HDS_Mesh *unfolded_mesh)
{
	// Define alias to HDS buffers
	auto &vertSet = unfolded_mesh->vertSet;
	auto &heSet   = unfolded_mesh->heSet;
	auto &faceSet = unfolded_mesh->faceSet;

	unfolded_mesh->bound.reset(new BBox3(0));
	auto& bound = unfolded_mesh->bound;

	int row_len_limit = static_cast<int>(sqrt(unfolded_mesh->pieceSet.size()));
	int cur_row_count = 0;
	QVector3D piece_offset;
	BBox3 global_bound(0);

	//vector<bool> visitedVerts(vertSet.size(), false);
	for (auto piece : unfolded_mesh->pieceSet)
	{
		unordered_set<hdsid_t> verts;
		//vector<hdsid_t> vids;
		/************************************************************************/
		/* Calculate Piece Orientation                                          */
		/************************************************************************/
		bool checkedOrientation = false;
		
		QVector3D newOrigin, newX, newY;
		for (auto fid : piece)
		{
			auto face = &unfolded_mesh->faceSet[fid];
			auto corners = unfolded_mesh->faceCorners(fid);
			verts.insert(corners.begin(), corners.end());
			if (!face->isCutFace && !checkedOrientation)
			{
				auto he = &heSet[face->heid];
				auto curHE = he;
				do
				{
					if (!curHE->isCutEdge)
					{
						break;
					}
					curHE = curHE->next();
				} while (curHE != he);
				newY = (vertSet[curHE->next()->vid].pos
					  - vertSet[curHE->vid].pos).normalized();
				newX = QVector3D(newY.y(), -newY.x(), 0);
				newOrigin = vertSet[curHE->vid].pos;
				checkedOrientation = true;
			}
		}
		// Apply orientation
		for (auto vid : verts)
		{
			// Apply Orientation to all vertices
			auto& pos = vertSet[vid].pos;
			pos -= newOrigin;
			pos = QVector3D(QVector3D::dotProduct(pos, newX),
				QVector3D::dotProduct(pos, newY), 0)
				+ newOrigin;
		}
		// calculate bouding for current piece
		BBox3 curBound;
		for (auto vid : verts)
		{
			curBound.Union(vertSet[vid].pos);
		}
		// if current row is too long, move to second row
		if (cur_row_count > row_len_limit)
		{
			global_bound = BBox3(QVector3D(global_bound.pMin.x(), global_bound.pMax.y(), 0));
			piece_offset = global_bound.pMin - curBound.pMin;
			cur_row_count = 1;
		}
		else// keep panning from original bound
		{
			piece_offset = QVector3D(global_bound.pMax.x(), global_bound.pMin.y(), 0) - curBound.pMin;
			cur_row_count++;
		}
		/************************************************************************/
		/* Assembling Offset                                                    */
		/************************************************************************/
		curBound.pMin += piece_offset;
		curBound.pMax += piece_offset;
		for (auto vid : verts)
		{
			// Apply Orientation to all vertices
			vertSet[vid].pos += piece_offset;
		}
		global_bound.Union(curBound);
		bound->Union(curBound);
	}
}


bool MeshUnfolder::unfold(
	HDS_Mesh* unfolded_mesh, const HDS_Mesh *ref_mesh,
	set<hdsid_t> fixedFaces)
{
	//progress dialog
	QProgressDialog unfoldingProgress("Unfolding...", "", 0, 100);
	unfoldingProgress.setWindowModality(Qt::WindowModal);
	unfoldingProgress.setValue(0);
	unfoldingProgress.setAutoClose(true);
	unfoldingProgress.setCancelButton(0);
	unfoldingProgress.setMinimumDuration(0);
	
	// Check if model is properly cut and unfoldable
	if( !unfoldable(ref_mesh) )
	{
		cout << "Mesh can not be unfolded. Check if the cuts are well defined." << endl;
		return false;
	}

	auto &vertSet = unfolded_mesh->vertSet;
	auto &heSet   = unfolded_mesh->heSet;
	auto &faceSet = unfolded_mesh->faceSet;
	auto &refVertSet = ref_mesh->vertSet;
	auto &refHeSet   = ref_mesh->heSet;
	auto &refFaceSet = ref_mesh->faceSet;

#ifdef _DEBUG
	cout << "Unfold Piece Count:\t" << ref_mesh->pieceSet.size() << endl;
#endif

	int progressIndex = 0; // Qt display progress
	// Hash Table for marking visited face
	vector<bool> visitedFaces(faceSet.size(), false);
	for (auto piece : ref_mesh->pieceSet)
	{
		auto it_fid = piece.begin();
		if (ref_mesh->faceSet[*it_fid].isCutFace)
		{
			it_fid++;
		}
		hdsid_t initFID = *it_fid;

		// Start from a face, expand all faces
		queue<hdsid_t> ProcQueue;
		//unfolded_mesh->printInfo();
		ProcQueue.push(initFID);

		vector<hdsid_t> expSeq;     // sequence of expansion
		unordered_map<hdsid_t, hdsid_t> parentMap;
		//set<hdsid_t> visited;
		//set<hdsid_t> frontier;
		while( !ProcQueue.empty() )
		{
			auto cur_fid = ProcQueue.front();
			ProcQueue.pop();
			if (faceSet[cur_fid].isCutFace)
			{
				continue;
			}
			visitedFaces[cur_fid] = true;
			
			expSeq.push_back(cur_fid);
			// Get all neighbor faces
			auto neighborFaces = unfolded_mesh->incidentFaceIDs(cur_fid);
			// Get all non-cut neighbor faces
			//vector<HDS_Face*> nonCutNeighborFaces = Utils::filter(neighborFaces,
			//	[](HDS_Face* f) { return !(f->isCutFace); });

			for( auto adjFid : neighborFaces )
			{
				if(!visitedFaces[adjFid] && !faceSet[adjFid].isCutFace)
				{
					ProcQueue.push(adjFid);
					parentMap.insert(make_pair(adjFid, cur_fid));
				}
			}
		}
		
		// Qt display progress
		unfoldingProgress.setValue(10 + (++progressIndex * 45.0f / fixedFaces.size()));

		// Print out the sequence of performing unfolding
		//Utils::print(expSeq);
		
		if (!expSeq.empty())
		{
			hdsid_t frontFid = expSeq.front();
			// Compute the spanning vectors for the first face
			QVector3D uvec, vvec;
			auto face0_ref = &refFaceSet[frontFid];
			auto he0_ref = &refHeSet[face0_ref->heid];
			QVector3D cface0_ref = ref_mesh->faceCenter(frontFid);//face0_ref->center();
			
			uvec = (refVertSet[he0_ref->next()->vid].pos
				  - refVertSet[he0_ref->vid].pos).normalized();
			vvec = QVector3D::crossProduct(ref_mesh->faceNormal(frontFid), uvec);

			// Project the first face to XY plane
			HDS_Face *face0_unf = &faceSet[frontFid];
			auto oriP = refVertSet[he0_ref->vid].pos;
			auto he_unf = &heSet[face0_unf->heid];
			auto curHE = he_unf;
			do 
			{
				vertSet[curHE->vid].pos = QVector3D(
					QVector3D::dotProduct(vertSet[curHE->vid].pos - oriP, uvec),
					QVector3D::dotProduct(vertSet[curHE->vid].pos - oriP, vvec), 0);
				curHE = curHE->next();
			} while (curHE != he_unf);
			QVector3D cface0_unf = unfolded_mesh->faceCenter(frontFid);
			uvec = vertSet[he_unf->vid].pos - cface0_unf;
			vvec = vertSet[he_unf->flip()->vid].pos - cface0_unf;
			// Unfold the mesh using the sequence
			// Update the vertex positions of the unfolded mesh base on the geometry of the reference mesh
			for (size_t i = 1; i < expSeq.size(); ++i)
			{
				unfoldFace(parentMap.at(expSeq[i]), expSeq[i], unfolded_mesh, ref_mesh, uvec, vvec);
			}
		}
		// Qt display progress
		unfoldingProgress.setValue(10 + (++progressIndex * 45.0f / fixedFaces.size()));
	}

	//unfolded_mesh->updatePieceSet();
	// Layout pieces
	//unfolded_mesh->bound.reset(new BBox3);
	reset_layout(unfolded_mesh);
	// Qt display progress
	unfoldingProgress.setValue(100);

	return true;
}
