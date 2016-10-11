#include "GeomProc/meshunfolder.h"
#include "HDS/hds_mesh.h"

#include "Utils/utils.h"
#include "Utils/mathutils.h"

#include <QDebug>
#include <QMessageBox>

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
#if 0
void MeshUnfolder::unfoldFace(hdsid_t fprev, hdsid_t fcur,
	HDS_Mesh *unfolded_mesh, const HDS_Mesh *ref_mesh,
	const QVector3D &uvec, const QVector3D &vvec)
{
	auto verts = unfolded_mesh->verts();
	auto hes   = unfolded_mesh->halfedges();
	auto faces = unfolded_mesh->faces();
	auto refVerts = ref_mesh->verts();
	auto refHEs   = ref_mesh->halfedges();
	auto refFaces = ref_mesh->faces();

	// use the previous face as reference, expand current face
	face_t* face_prev = &faces[fprev];
	face_t* face_cur = &faces[fcur];

	// start from he in previous face
	// find the shared half edge
	he_t* he_share = &hes[face_prev->heid];
	while (he_share->flip()->fid != fcur)
	{
		he_share = he_share->next();
	}

	// change the position of the vertices in the current face
	// except for the end points of he_share
	vert_t* vs = &verts[he_share->vid];
	vert_t* ve = &verts[he_share->flip()->vid];

	// compute the spanning vectors for the unfolded mesh
	QVector3D xvec = vs->pos - ve->pos;
	xvec.normalize();
	QVector3D yvec = uvec - QVector3D::dotProduct(uvec, xvec) * xvec;
	const qreal eps = 1e-6;
	// To avoid precision issue, check if yvec is too short
	if (yvec.length() < eps)
	{
		yvec = vvec - QVector3D::dotProduct(vvec, xvec) * xvec;
	}
	yvec.normalize();

	QVector3D cface_prev = unfolded_mesh->faceCenter(fprev);
	if (QVector3D::dotProduct(cface_prev - vs->pos, yvec) > 0)
	{
		yvec = -yvec;
	}

	// shared edge in the reference mesh
	auto he_share_ref = &refHEs[he_share->index];
	auto face_cur_ref = &refFaces[fcur];
	auto vs_ref = &refVerts[he_share_ref->vid];

	// compute the spanning vectors for the face in the reference mesh
	QVector3D cface_ref = ref_mesh->faceCenter(fcur);
	QVector3D faceN_ref = ref_mesh->faceNormal(fcur);

	QVector3D xvec_ref = ref_mesh->edgeVector(*he_share_ref).normalized();
	QVector3D yvec_ref = (cface_ref - vs_ref->pos)
		- QVector3D::dotProduct(cface_ref - vs_ref->pos, xvec_ref) * xvec_ref;
	yvec_ref.normalize();

	he_t* he = he_share->flip();
	he_t* curHE = he;
	do
	{
		if (curHE->vid == vs->index || curHE->vid == ve->index)
		{
			//continue;
			// nothing to do, these two should not be modified
		}
		else
		{
			// compute the new position of the vertex
			vert_t *v = &verts[curHE->vid];
			auto v_ref = &refVerts[curHE->vid];

			// compute the coordinates of this vertex using the reference mesh
			QVector3D dvec_ref = v_ref->pos - vs_ref->pos;
			qreal xcoord = QVector3D::dotProduct(dvec_ref, xvec_ref);
			qreal ycoord = QVector3D::dotProduct(dvec_ref, yvec_ref);
			
			v->pos = vs->pos + xcoord * xvec + ycoord * yvec;
		}
		curHE = curHE->next();
	} while (curHE != he);
}
#endif
void MeshUnfolder::unfoldFace(hdsid_t prevFid, hdsid_t curFid,
	HDS_Mesh *unfolded_mesh, const HDS_Mesh *ref_mesh,
	const QVector3D &uvec, const QVector3D &vvec)
{
	auto verts = unfolded_mesh->verts();
	auto hes = unfolded_mesh->halfedges();
	auto faces = unfolded_mesh->faces();
	auto refVerts = ref_mesh->verts();
	auto refHEs = ref_mesh->halfedges();
	auto refFaces = ref_mesh->faces();

	// use the previous face as reference, expand current face
	face_t* face_prev = &faces[prevFid];
	face_t* face_cur = &faces[curFid];

	// start from he in previous face
	// find the shared half edge
	he_t* he_share = &hes[face_cur->heid];
	while (he_share->flip()->fid != prevFid)
	{
		he_share = he_share->next();
	}
	const QVector3D ori_ref = refVerts[he_share->vid].pos;
	const QVector3D ori_unf = verts[he_share->vid].pos;
	const QVector3D xvec = ref_mesh->edgeVector(*he_share).normalized();
	const QVector3D yvec = QVector3D::crossProduct(ref_mesh->faceNormal(curFid), xvec);

	const QVector3D udir = -unfolded_mesh->edgeVector(he_share->flip()->index).normalized();
	const QVector3D vdir = QVector3D::crossProduct(QVector3D(0, 0, 1), udir);


	he_t* he = he_share->next()->next();
	he_t* curHE = he;
	do
	{
		// compute the new position of the vertex
		vert_t* v = &verts[curHE->vid];
		vert_t* v_ref = &refVerts[curHE->vid];

		// compute the coordinates of this vertex using the reference mesh
		QVector3D dvec_ref = v_ref->pos - ori_ref;
		qreal xcoord = QVector3D::dotProduct(dvec_ref, xvec);
		qreal ycoord = QVector3D::dotProduct(dvec_ref, yvec);

		v->pos = ori_unf + xcoord * udir + ycoord * vdir;
		curHE = curHE->next();
	} while (curHE != he_share);
}

bool MeshUnfolder::unfoldable(const HDS_Mesh* ref_mesh)
{
	auto &faces = ref_mesh->faces();

	// for each vertex in the cutted_mesh, check the condition
	auto isBadVertex = [&ref_mesh, &faces](const HDS_Vertex &v) {
		double sum = 0;
		auto he = ref_mesh->heFromVert(v.index);
		auto curHE = he;
		auto nextHE = he->rotCW();
		bool hasCutFace = false;
		do
		{
			if (faces[curHE->fid].isCutFace) hasCutFace = true;
			else sum += acos(QVector3D::dotProduct(
				ref_mesh->edgeVector(curHE->index).normalized(),
				ref_mesh->edgeVector(nextHE->index).normalized()));

			curHE = nextHE;
			nextHE = nextHE->rotCW();
		} while (curHE != he);

		// The sum of angles of an unfoldable vertex is smaller than pi*2 with CutFace
		// Or equal to 2*PI without cutface
		return sum > (PI2 + PI_EPS) || (sum < (PI2 - PI_EPS) && !hasCutFace);
	};
	if (any_of(ref_mesh->vertSet.begin(), ref_mesh->vertSet.end(), isBadVertex))
	{
		QMessageBox msgBox(QMessageBox::Warning, QString("Warning"),
			QString(
				"Current mesh has at least one bad vertex!\n"\
				"Unfolding will results in overlapping.\n\n"\
				"Do you still want to unfold it?"),
			QMessageBox::Yes | QMessageBox::Cancel,
			nullptr,
			Qt::WindowStaysOnTopHint);

		return msgBox.exec() == QMessageBox::Yes;
	}
	return true;
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
	if (!unfoldable(ref_mesh))
	{
		cout << "Mesh can not be unfolded. Check if the cuts are well defined." << endl;
		return false;
	}

	auto vertSet = unfolded_mesh->verts();
	auto heSet   = unfolded_mesh->halfedges();
	auto faceSet = unfolded_mesh->faces();
	auto refVertSet = ref_mesh->verts();
	auto refHeSet   = ref_mesh->halfedges();
	auto refFaceSet = ref_mesh->faces();

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
			
			// ^ vvec
			// | 
			// *---> uvec
			uvec = ref_mesh->edgeVector(*he0_ref).normalized();
			vvec = QVector3D::crossProduct(ref_mesh->faceNormal(frontFid), uvec);

			// Project the first face to XY plane
			face_t* face0_unf = &faceSet[frontFid];
			QVector3D oriP = refVertSet[he0_ref->vid].pos;
			he_t* he_unf = &heSet[face0_unf->heid];
			he_t* curHE = he_unf;
			do 
			{
				vertSet[curHE->vid].pos = QVector3D(
					QVector3D::dotProduct(vertSet[curHE->vid].pos - oriP, uvec),
					QVector3D::dotProduct(vertSet[curHE->vid].pos - oriP, vvec),
					0);
				curHE = curHE->next();
			} while (curHE != he_unf);
			QVector3D cface0_unf = unfolded_mesh->faceCenter(frontFid);
			uvec = vertSet[he_unf->vid].pos - cface0_unf;
			vvec = vertSet[he_unf->flip()->vid].pos - cface0_unf;
			// Unfold the mesh using the sequence
			// Update the vertex positions of the unfolded mesh base on the geometry of the reference mesh
			for (size_t i = 1; i < expSeq.size(); ++i)
			{
				unfoldFace(
					parentMap.at(expSeq[i]), expSeq[i],
					unfolded_mesh, ref_mesh, uvec, vvec
				);
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
