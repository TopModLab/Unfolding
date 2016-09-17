#include "meshunfolder.h"
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

void MeshUnfolder::unfoldFace(int fprev, int fcur,
	HDS_Mesh *unfolded_mesh, const HDS_Mesh *ref_mesh,
	const QVector3D &uvec, const QVector3D &vvec)
{
	typedef HDS_Face face_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_HalfEdge he_t;

	// use the previous face as reference, expand current face
	face_t *face_prev = unfolded_mesh->faceMap.at(fprev);
	face_t *face_cur = unfolded_mesh->faceMap.at(fcur);

	// print the corners of the previous face
	auto corners_prev = face_prev->corners();

	//kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk comment
	/*qDebug() << "corners of face_prev";
	for(auto p : corners_prev) {
		qDebug() << p->index << " @ " << p->pos;
	}*/

	// find the shared half edge
	he_t *he_share = face_prev->he;
	do {
		if (he_share->flip->f == face_cur)
		{
			break;
		}
		he_share = he_share->next;
	} while( he_share != face_prev->he );

	// change the position of the vertices in the current face, except for the end points
	// of he_share
	vert_t *vs = he_share->v;
	vert_t *ve = he_share->flip->v;

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
	QVector3D cface_prev = face_prev->center();
	if (QVector3D::dotProduct(cface_prev - vs->pos, yvec) > 0)
	{
		yvec = -yvec;
	}

	// shared edge in the reference mesh
	he_t *he_share_ref = ref_mesh->heMap.at(he_share->index);
	face_t *face_cur_ref = ref_mesh->faceMap.at(face_cur->index);
	vert_t *vs_ref = he_share_ref->v;
	vert_t *ve_ref = he_share_ref->flip->v;

	// compute the spanning vectors for the face in the reference mesh
	QVector3D cface_ref = face_cur_ref->center();

	QVector3D xvec_ref = vs_ref->pos - ve_ref->pos;
	xvec_ref.normalize();
	QVector3D yvec_ref = (cface_ref - vs_ref->pos) - QVector3D::dotProduct(cface_ref - vs_ref->pos, xvec_ref) * xvec_ref;
	yvec_ref.normalize();

	he_t *he = he_share->flip;
	he_t *curHE = he;
	do {
		if( curHE->v == vs || curHE->v == ve )
		{
			//continue;
			// nothing to do, these two should not be modified
		}
		else
		{
			// compute the new position of the vertex
			vert_t *v = curHE->v;
			vert_t *v_ref = ref_mesh->vertMap.at(curHE->v->index);

			// compute the coordinates of this vertex using the reference mesh
			QVector3D dvec_ref = v_ref->pos - vs_ref->pos;
			qreal xcoord = QVector3D::dotProduct(dvec_ref, xvec_ref);
			qreal ycoord = QVector3D::dotProduct(dvec_ref, yvec_ref);
			
			v->pos = vs->pos + xcoord * xvec + ycoord * yvec;

		}
		curHE = curHE->next;
	} while( curHE != he );
}

bool MeshUnfolder::unfoldable(const HDS_Mesh *ref_mesh) {
	// for each vertex in the cutted_mesh, check the condition
	auto isBadVertex = [](const HDS_Vertex* v) -> bool {
		vector<double> sums;
		double sum = 0;
		auto he = v->he;
		auto curHE = he->flip->next;
		bool hasCutFace = false;
		do {
			if( !curHE->f->isCutFace ) {
				QVector3D v1 = he->flip->v->pos - he->v->pos;
				QVector3D v2 = curHE->flip->v->pos - curHE->v->pos;
				double nv1pnv2 = v1.length() * v2.length();
				double inv_nv1pnv2 = 1.0 / nv1pnv2;
				double cosVal = QVector3D::dotProduct(v1, v2) * inv_nv1pnv2;
				double angle = acos(cosVal);
				sum += angle;
			}
			else hasCutFace = true;

			he = curHE;
			curHE = he->flip->next;
		} while(he != v->he);

		// The sum of angles of an unfoldable vertex is smaller than pi*2 with CutFace
		// Or equal to 2*pi without cutface
		return sum > (PI2 + PI_EPS) || (sum < (PI2 - PI_EPS) && !hasCutFace);
	};
	if( any_of(ref_mesh->vertSet.begin(), ref_mesh->vertSet.end(), isBadVertex) ) return false;
	else return true;
}

void MeshUnfolder::reset_layout(HDS_Mesh *unfolded_mesh)
{
	delete unfolded_mesh->bound;
	unfolded_mesh->bound = new BBox3(0);
	auto& bound = unfolded_mesh->bound;

	int row_len_limit = static_cast<int>(sqrt(unfolded_mesh->pieceSet.size()));
	int cur_row_count = 0;
	QVector3D piece_offset;
	BBox3 global_bound(0);

	for (auto piece : unfolded_mesh->pieceSet)
	{
		unordered_set<HDS_Vertex *> verts;
		/************************************************************************/
		/* Calculate Piece Orientation                                          */
		/************************************************************************/
		bool checkedOrientation = false;
		
		QVector3D newOrigin, newX, newY;
		for (auto fid : piece)
		{
			auto face = unfolded_mesh->faceMap[fid];
			auto corners = face->corners();
			verts.insert(corners.begin(), corners.end());
			if (!face->isCutFace && !checkedOrientation)
			{
				auto he = face->he;
				auto curHE = he;
				do
				{
					if (!curHE->isCutEdge)
					{
						break;
					}
					curHE = curHE->next;
				} while (curHE != he);
				newY = (curHE->next->v->pos - curHE->v->pos).normalized();
				newX = QVector3D(newY.y(), -newY.x(), 0);
				newOrigin = curHE->v->pos;
				checkedOrientation = true;
			}
		}
		// Apply orientation
		for (auto vert : verts)
		{
			// Apply Orientation to all vertices
			auto& pos = vert->pos;
			pos -= newOrigin;
			pos = QVector3D(QVector3D::dotProduct(pos, newX),
				QVector3D::dotProduct(pos, newY), 0)
				+ newOrigin;
		}
		// calculate bouding for current piece
		BBox3 curBound;
		for (auto vert : verts)
		{
			curBound.Union(vert->pos);
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
/*
		if (unfolded_mesh->processType == HDS_Mesh::FBWALK_PROC)
		{
			piece_offset *= 1.5;
		}*/
		curBound.pMin += piece_offset;
		curBound.pMax += piece_offset;
		for (auto vert : verts)
		{
			// Apply Orientation to all vertices
			vert->pos += piece_offset;
		}
		global_bound.Union(curBound);
		bound->Union(curBound);
	}
}


bool MeshUnfolder::unfold(
	HDS_Mesh* unfolded_mesh, const HDS_Mesh *ref_mesh,
	set<int> fixedFaces)
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
	// ref_mesh->updatePieceSet();
	cout << "Unfold Piece Count:\t" << ref_mesh->pieceSet.size() << endl;
	// If no face is selected, find one face in each piece and push into fixedFaces
	if( fixedFaces.empty() )
	{
#ifdef _DEBUG
		cout << "No face is selected, finding fixed faces..." << endl;
#endif
		// find all fixed faces
		unordered_set<int> visitedFaces;

		int progressIndex = 0;
		// Find all faces
		for(auto f : ref_mesh->faceSet)
		{
			if (f->isCutFace)
			{
				continue;
			}

			// If f has not been visited yet
			// Add to selected faces
			if( visitedFaces.find(f->index) == visitedFaces.end() )
			{
				visitedFaces.insert(f->index);
				fixedFaces.insert(f->index);
				// Find all linked faces except cut face
				set<HDS_Face*> connectedFaces = Utils::filter_set(f->linkedFaces(), [](HDS_Face* f){
						return !(f->isCutFace);
				});
				// record the pieces
				//pieces.insert(&connectedFaces);

				for(auto cf : connectedFaces)
				{
					visitedFaces.insert(cf->index);
				}
			}
			unfoldingProgress.setValue((double)progressIndex/(double)ref_mesh->faceSet.size()*10);
		}

#ifdef _DEBUG
		cout << "Fixed faces found: ";
		Utils::print(fixedFaces);
#endif
	}

	unfoldingProgress.setValue(10);

	int progressIndex = 0; // Qt display progress
	for (auto piece : ref_mesh->pieceSet)
	{
		auto it_fid = piece.begin();
		if (ref_mesh->faceMap.at(*it_fid)->isCutFace)
		{
			it_fid++;
		}
		int fid = *it_fid;

		// Start from a face, expand all faces
		queue<HDS_Face*> Q;
		//unfolded_mesh->printInfo();
		Q.push(unfolded_mesh->faceMap[fid]);

		int32s_t expSeq;     // sequence of expansion
		map<int, int> parentMap;
		set<int> visited;
		set<int> frontier;
		while( !Q.empty() )
		{
			auto cur = Q.front();
			Q.pop();
			visited.insert(cur->index);
			frontier.erase(cur->index);

			expSeq.push_back(cur->index);
			// Get all neighbor faces
			vector<HDS_Face*> neighborFaces = unfolded_mesh->incidentFaces(cur);
			// Get all non-cut neighbor faces
			vector<HDS_Face*> nonCutNeighborFaces = Utils::filter(neighborFaces, [](HDS_Face* f) {
					return !(f->isCutFace);
			});


			for( auto f : nonCutNeighborFaces )
			{
				if( visited.find(f->index) == visited.end() && frontier.find(f->index) == frontier.end() )
				{
					Q.push(f);
					parentMap.insert(make_pair(f->index, cur->index));
					frontier.insert(f->index);
				}
			}
		}
		
		// Qt display progress
		unfoldingProgress.setValue(10+((double)++progressIndex/(double)(fixedFaces.size()*2)*90));

		// Print out the sequence of performing unfolding
		//Utils::print(expSeq);
		
		if (!expSeq.empty())
		{
			
			// Compute the spanning vectors for the first face
			QVector3D uvec, vvec;
			HDS_Face *face0_ref = ref_mesh->faceMap.at(expSeq.front());
			HDS_HalfEdge *he0_ref = face0_ref->he;
			QVector3D cface0_ref = face0_ref->center();
			
			uvec = (he0_ref->next->v->pos - he0_ref->v->pos).normalized();
			vvec = QVector3D::crossProduct(face0_ref->computeNormal(), uvec);

			// Project the first face to XY plane
			HDS_Face *face0_unf = unfolded_mesh->faceMap.at(expSeq.front());
			auto oriP = he0_ref->v->pos;// +QVector3D(rand() % 5, rand() % 5, 0);
			auto he_unf = face0_unf->he;
			auto curHE = he_unf;
			do 
			{
				curHE->v->pos = QVector3D(QVector3D::dotProduct(curHE->v->pos - oriP, uvec),
					QVector3D::dotProduct(curHE->v->pos - oriP, vvec), 0);
				curHE = curHE->next;
			} while (curHE != he_unf);
			uvec = he_unf->v->pos - face0_unf->center();
			vvec = he_unf->flip->v->pos - face0_unf->center();
			// Unfold the mesh using the sequence
			// Update the vertex positions of the unfolded mesh base on the geometry of the reference mesh
			for (int i = 1; i < expSeq.size(); ++i)
			{
				unfoldFace(parentMap.at(expSeq[i]), expSeq[i], unfolded_mesh, ref_mesh, uvec, vvec);
			}

		}
		// Qt display progress
		unfoldingProgress.setValue(10+((double)++progressIndex/(double)(fixedFaces.size()*2)*90));
	}

	//unfolded_mesh->updatePieceSet();
	// Layout pieces
	reset_layout(unfolded_mesh);
	// Qt display progress
	unfoldingProgress.setValue(100);

	return true;
}
