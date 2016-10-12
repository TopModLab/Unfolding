#include "MeshFactory/MeshUnfolder.h"
#include "HDS/hds_mesh.h"

#include "Utils/utils.h"
#include "Utils/mathutils.h"

#include <QDebug>
#include <QVector3D>
#include <QMessageBox>
#include <QProgressDialog>

void MeshUnfolder::unfoldFace(
	hdsid_t prevFid, hdsid_t curFid,
	HDS_Mesh *unfolded_mesh, const HDS_Mesh *ref_mesh)
{
	auto &verts = unfolded_mesh->verts();
	auto &hes = unfolded_mesh->halfedges();
	auto &faces = unfolded_mesh->faces();
	auto &refVerts = ref_mesh->verts();
	auto &refHEs = ref_mesh->halfedges();
	auto &refFaces = ref_mesh->faces();

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

	const QVector3D udir = unfolded_mesh->edgeVector(he_share->index).normalized();
	const QVector3D vdir = QVector3D(-udir.y(), udir.x(), 0);


	he_t* he = he_share->next()->next();
	he_t* curHE = he;
	do
	{
		// compute the new position of the vertex
		vert_t* v = &verts[curHE->vid];
		const vert_t* v_ref = &refVerts[curHE->vid];

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
	auto &vertSet = unfolded_mesh->verts();
	auto &heSet   = unfolded_mesh->halfedges();
	auto &faceSet = unfolded_mesh->faces();

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


HDS_Mesh* MeshUnfolder::unfold(const HDS_Mesh* ref_mesh)
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
		return nullptr;
	}

	mesh_t* unfolded_mesh = new mesh_t(*ref_mesh);

	auto &vertSet = unfolded_mesh->verts();
	auto &heSet   = unfolded_mesh->halfedges();
	auto &faceSet = unfolded_mesh->faces();
	auto &refVertSet = ref_mesh->verts();
	auto &refHeSet   = ref_mesh->halfedges();
	auto &refFaceSet = ref_mesh->faces();
	
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
		ProcQueue.push(initFID);

		vector<hdsid_t> expSeq;     // sequence of expansion
		unordered_map<hdsid_t, hdsid_t> parentMap;

		while (!ProcQueue.empty())
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
		unfoldingProgress.setValue(10 + (++progressIndex * 45.0f / ref_mesh->pieceSet.size()));
		
		if (!expSeq.empty())
		{
			hdsid_t frontFid = expSeq.front();

			//auto face0_ref = &refFaceSet[frontFid];
			auto he0_ref = ref_mesh->heFromFace(frontFid);
			QVector3D cface0_ref = ref_mesh->faceCenter(frontFid);
			
			// Compute the spanning vectors for the first face
			// ^ vvec
			// | 
			// *---> uvec
			QVector3D uvec = ref_mesh->edgeVector(*he0_ref).normalized();
			QVector3D vvec = QVector3D::crossProduct(ref_mesh->faceNormal(frontFid), uvec);

			// Project the first face to XY plane
			QVector3D oriP = refVertSet[he0_ref->vid].pos;
			he_t* he_unf = unfolded_mesh->heFromFace(frontFid);
			he_t* curHE = he_unf;
			do 
			{
				vertSet[curHE->vid].pos = QVector3D(
					QVector3D::dotProduct(vertSet[curHE->vid].pos - oriP, uvec),
					QVector3D::dotProduct(vertSet[curHE->vid].pos - oriP, vvec),
					0);
				curHE = curHE->next();
			} while (curHE != he_unf);

			// Unfold the mesh using the sequence
			// Update the vertex positions of the unfolded mesh
			// base on the geometry of the reference mesh
			for (size_t i = 1; i < expSeq.size(); ++i)
			{
				unfoldFace(parentMap.at(expSeq[i]), expSeq[i],
					unfolded_mesh, ref_mesh);
			}
		}
		// Qt display progress
		unfoldingProgress.setValue(10 + (++progressIndex * 45.0f / ref_mesh->pieceSet.size()));
	}

	// Layout pieces
	reset_layout(unfolded_mesh);
	// Qt display progress
	unfoldingProgress.setValue(100);

	return unfolded_mesh;
}
