#include "MeshFactory/MeshUnfolder.h"
#include "HDS/HDS_Mesh.h"

#include "Utils/utils.h"
#include "Utils/mathutils.h"

#include <QDebug>
#include <QVector3D>
#include <QMessageBox>
#include <QProgressDialog>

// Unfold a Face Along a Given Edge
// sharedHE is the edge on current face,
// whose flip is in parent face
void MeshUnfolder::unfoldFace(
	hdsid_t sharedHEid, hdsid_t curFid,
	HDS_Mesh* unfolded_mesh, const HDS_Mesh* ref_mesh,
	vector<bool> &visitedVerts, vector<hdsid_t> &dirtyEdges)
{
	auto &verts = unfolded_mesh->verts();
	auto &refVerts = ref_mesh->verts();

	he_t* he_share = &unfolded_mesh->halfedges()[sharedHEid];
	const QVector3D &ori_ref = refVerts[he_share->vid].pos;
	const QVector3D &ori_unf = verts[he_share->vid].pos;
	// Local axis of the original mesh, aka reference mesh
	const QVector3D xvec = ref_mesh->edgeVector(sharedHEid).normalized();
	const QVector3D yvec = QVector3D::crossProduct(ref_mesh->faceNormal(curFid), xvec);
	// Local axis of unfolded mesh, aka the 2D plane
	// Normal is always vector(0, 0, 1), aka Z-axes in world space
	const QVector3D udir = unfolded_mesh->edgeVector(sharedHEid).normalized();
	const QVector3D vdir = QVector3D(-udir.y(), udir.x(), 0);

	// No need to 
	//he_t* he = he_share->next()->next();
	he_t* curHE = he_share->next()->next();
	do
	{
		if (visitedVerts[curHE->vid])
		{
			dirtyEdges.push_back(curHE->index);
			curHE->isPicked = true;
			curHE->flip()->isPicked = true;
		}
		else
		{
			visitedVerts[curHE->vid] = true;
			// compute the new position of the vertex
			vert_t* v = &verts[curHE->vid];
			const vert_t* v_ref = &refVerts[curHE->vid];

			// compute the coordinates of this vertex using the reference mesh
			QVector3D dvec_ref = v_ref->pos - ori_ref;
			// Vector length from projection onto local axis
			qreal xcoord = QVector3D::dotProduct(dvec_ref, xvec);
			qreal ycoord = QVector3D::dotProduct(dvec_ref, yvec);

			// Project position onto unfolded 2D plane
			v->pos = ori_unf + xcoord * udir + ycoord * vdir;
		}
		curHE = curHE->next();
	} while (curHE != he_share);
}

void MeshUnfolder::unfoldSeparateFace(
	const QVector3D &new_pos, const QVector3D &udir,
	hdsid_t curFid, HDS_Mesh* inMesh)
{
	auto &verts = inMesh->verts();
	
	he_t* he_share = inMesh->heFromFace(curFid);
	QVector3D &ori_pos = verts[he_share->vid].pos;
	// Local axis of the original mesh, aka reference mesh
	const QVector3D xvec = inMesh->edgeVector(*he_share).normalized();
	const QVector3D yvec = QVector3D::crossProduct(inMesh->faceNormal(curFid), xvec);
	// Local axis of unfolded mesh, aka the 2D plane
	// Normal is always vector(0, 0, 1), aka Z-axes in world space
	// Assume udir is normalized and on X-Y plane
	const QVector3D vdir = QVector3D(-udir.y(), udir.x(), 0);
	
	he_t* curHE = he_share->next();
	do
	{
		// compute the new position of the vertex
		vert_t* v = &verts[curHE->vid];

		// compute the coordinates of this vertex using the reference mesh
		QVector3D dvec_ref = v->pos - ori_pos;
		// Vector length from projection onto local axis
		qreal xcoord = QVector3D::dotProduct(dvec_ref, xvec);
		qreal ycoord = QVector3D::dotProduct(dvec_ref, yvec);

		// Project position onto unfolded 2D plane
		v->pos = new_pos + xcoord * udir + ycoord * vdir;
		
		curHE = curHE->next();
	} while (curHE != he_share);
	ori_pos = new_pos;
}

// Check if it's able to unfold any vertex in the mesh
// When a vertex is unfoldable,
// the sum of corner angles around it satisfies either below
// 1. smaller than 2*Pi with CutFace,
// 2. equal to 2*PI without cutface
bool MeshUnfolder::hasBadVertex(const HDS_Mesh* ref_mesh)
{
	auto &faces = ref_mesh->faces();

	// for each vertex in the cutted_mesh, check the condition
	return any_of(ref_mesh->vertSet.begin(), ref_mesh->vertSet.end(),
		[&ref_mesh, &faces](const HDS_Vertex &v) {
		qreal sum = 0;
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
	});
}

// Assemble Mesh Pieces using Axis-Aligned Bounding Box
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

HDS_Mesh* MeshUnfolder::unfold(const HDS_Mesh * ref_mesh)
{
	vector<hdsid_t> dirtyEdges;
	HDS_Mesh* ret_mesh = unfold(ref_mesh, dirtyEdges);
	
	if (!ret_mesh) return nullptr;

	if (!dirtyEdges.empty())
	{
		QMessageBox msgBox(QMessageBox::Warning, QString("Warning"),
			QString(
				"Current mesh has at least one bad edge!\n"\
				"Unfolding will lead to unexpected result.\n\n"\
				"Do you still want to unfold it?"),
			QMessageBox::Yes | QMessageBox::Cancel,
			nullptr,
			Qt::WindowStaysOnTopHint);

		// If user insists unfolding, do it.
		// Otherwise, return null.
		if (msgBox.exec() == QMessageBox::Cancel) return nullptr;
	}
	for (auto heid : dirtyEdges)
	{
		ret_mesh->heSet[heid].isPicked = true;
	}
	return ret_mesh;
}

// Unfold an Input Mesh
HDS_Mesh* MeshUnfolder::unfold(
	const HDS_Mesh* ref_mesh, vector<hdsid_t> &dirtyEdges)
{
	cout << "started to unfold mesh!\n";
	//progress dialog
	QProgressDialog unfoldingProgress("Unfolding...", "", 0, 100);
	unfoldingProgress.setWindowModality(Qt::WindowModal);
	unfoldingProgress.setValue(0);
	unfoldingProgress.setAutoClose(true);
	unfoldingProgress.setCancelButton(0);
	unfoldingProgress.setMinimumDuration(0);
	
	// Check if model is properly cut and unfoldable
	if (hasBadVertex(ref_mesh))
	{
		QMessageBox msgBox(QMessageBox::Warning, QString("Warning"),
			QString(
				"Current mesh has at least one bad vertex!\n"\
				"Unfolding will lead to unexpected result.\n\n"\
				"Do you still want to unfold it?"),
			QMessageBox::Yes | QMessageBox::Cancel,
			nullptr,
			Qt::WindowStaysOnTopHint);

		// If user insists unfolding, do it.
		// Otherwise, return null.
		if (msgBox.exec() == QMessageBox::Cancel) return nullptr;
	}

	mesh_t* unfolded_mesh = new mesh_t(*ref_mesh);
	auto &refFaces = ref_mesh->faces();

	// Qt display progress
	int progressIndex = 0;
	// Hash Table for marking visited face
	vector<bool> visitedFaces(refFaces.size(), false);
	// Avoid unfold same edge twice
	vector<bool> visitedVerts(ref_mesh->verts().size(), false);
	// Connected edge id to its parent face
	vector<hdsid_t> parentEdgeMap(refFaces.size(), -1);
	if (unfolded_mesh->pieceSet.empty())
	{
		unfolded_mesh->updatePieceSet();
	}

	for (auto piece : unfolded_mesh->pieceSet)
	{
		// Find the first non cutface in piece
		auto it_fid = piece.begin();
		while (refFaces[*it_fid].isCutFace) it_fid++;
		hdsid_t initFID = *it_fid;

		// Start from a face, expand all faces
		queue<hdsid_t> ProcQueue;
		ProcQueue.push(initFID);

		// sequence of expansion
		deque<hdsid_t> expSeq;

		// Find spanning tree from first face
		while (!ProcQueue.empty())
		{
			auto cur_fid = ProcQueue.front();
			ProcQueue.pop();
			// pass all cutface
			if (refFaces[cur_fid].isCutFace) continue;

			visitedFaces[cur_fid] = true;
			// push current 
			expSeq.push_back(cur_fid);

			// Go through neighboring faces,
			// record shared edge to current face
			auto he = ref_mesh->heFromFace(cur_fid);
			auto curHE = he;
			do
			{
				hdsid_t adjFid = curHE->flip()->fid;
				if (!visitedFaces[adjFid] && !refFaces[adjFid].isCutFace)
				{
					ProcQueue.push(adjFid);
					parentEdgeMap[adjFid] = curHE->flip()->index;
				}
				curHE = curHE->next();
			} while (curHE != he);
		}
		
		// Qt display progress
		unfoldingProgress.setValue(10 + (++progressIndex * 45.0f / unfolded_mesh->pieceSet.size()));
		
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
			QVector3D ori_ref = ref_mesh->verts()[he0_ref->vid].pos;
			he_t* he_unf = unfolded_mesh->heFromFace(frontFid);
			he_t* curHE = he_unf;
			do
			{
				if (visitedVerts[curHE->vid])
				{
					dirtyEdges.push_back(curHE->index);
				}
				else
				{
					visitedVerts[curHE->vid] = true;
					auto &vpos = unfolded_mesh->vertFromHe(curHE->index)->pos;
					auto vec = vpos - ori_ref;
					vpos = QVector3D(
						QVector3D::dotProduct(vec, uvec),
						QVector3D::dotProduct(vec, vvec),
						0);
				}
				curHE = curHE->next();
			} while (curHE != he_unf);

			// Unfold the mesh using the sequence
			// Update the vertex positions of the unfolded mesh
			// base on the geometry of the reference mesh
			for (size_t i = 1; i < expSeq.size(); ++i)
			{
				unfoldFace(
					parentEdgeMap[expSeq[i]], expSeq[i],
					unfolded_mesh, ref_mesh,
					visitedVerts, dirtyEdges
				);
			}
		}
		// Qt display progress
		unfoldingProgress.setValue(10 + (++progressIndex * 45.0f / unfolded_mesh->pieceSet.size()));
	}

	// Layout pieces
	reset_layout(unfolded_mesh);
	// Qt display progress
	unfoldingProgress.setValue(100);

	return unfolded_mesh;
}
