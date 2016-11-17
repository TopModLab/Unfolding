#include "MeshOrigami.h"
#include "Utils/utils.h"
#include "MeshFactory/MeshUnfolder.h"
#include "UI/MeshViewer.h"

vector<QVector3D> MeshOrigami::pos = vector<QVector3D>();
vector<QVector3D> MeshOrigami::orient = vector<QVector3D>();
vector<hdsid_t> MeshOrigami::heid = vector<hdsid_t>();
HDS_Mesh* MeshOrigami::create(
	const mesh_t* ref, const confMap &conf)
{
	int faceSize = ref->faces().size();
	pos = vector<QVector3D>(faceSize, QVector3D(0, 0, 0));
	orient = vector<QVector3D>(faceSize, QVector3D(0, 1, 0));
	heid = vector<hdsid_t>(faceSize);
	HDS_Mesh* outMesh = MeshOrigami::createOrigami(ref, conf);
	//while dist can be further reduced
	float curDist = INT_MAX;

	while (1) {

		//calculate next step distance
		float dist = 0;
		vector<QVector3D> movingDir(faceSize);
		MeshOrigami::evaluateOrigami(ref, outMesh, dist, movingDir);
		if (curDist < dist || fabsf(curDist - dist) < 0.01) break;
		else {
			//move pos in movingDir
			//step size 0.05
			for (int i = 0; i < faceSize; i++) {
				pos[i] += movingDir[i] / 20;
			}
			curDist = dist;
		}

		MeshOrigami::processOrigami(ref, outMesh);
		//MeshViewer::getInstance()->unfoldView(outMesh);
		MeshViewer::getInstance()->bindHalfEdgeMesh(outMesh);
		MeshViewer::getInstance()->repaint();
		Sleep(uint(100));

	}
	return outMesh;
}

// create faces of origami and unfold them using BFS
HDS_Mesh* MeshOrigami::createOrigami(
	const mesh_t* ref_mesh, const confMap &conf)
{
	if (!ref_mesh) return nullptr;
	const float patchScale = conf.at("patchScale");
	const float foldDepth = conf.at("foldDepth");

	HDS_Vertex::resetIndex();
	auto &ref_verts = ref_mesh->verts();
	auto &ref_hes = ref_mesh->halfedges();
	size_t refHeCount = ref_hes.size();
	size_t refFaceCount = ref_mesh->faces().size();

	vector<vert_t> verts(refHeCount);
	vector<he_t> hes(ref_hes);
	vector<face_t> faces(ref_mesh->faces());
	mesh_t* ret = new HDS_Mesh(verts, hes, faces);
	

	hdsid_t nulledge = ret->halfedges().size();
	for (auto &he : ret->halfedges())
	{
		if (he.isCutEdge)
		{
			nulledge = he.index;
			break;
		}
		he.flip_offset = 0;
		auto v = &ret->verts()[he.index];
		v->pos = ref_verts[he.vid].pos;
		v->heid = he.vid = he.index;
	}
	ret->halfedges().resize(nulledge);
	bridgeOrigami(ref_mesh, ret, patchScale, foldDepth);
	return ret;
}


// move faces and bridges positions
void MeshOrigami::bridgeOrigami(
	const mesh_t* ref_mesh, mesh_t* ret, float patchScale, float foldDepth) {
	auto &ref_hes = ref_mesh->halfedges();
	size_t heSize = ref_hes.size();
	ret->matchIndexToSize();
	int faceSize = ret->faces().size();

	//BFS to get an initial layout
	vector<hdsid_t> parentEdgeMap(faceSize, 0);
	queue<pair<hdsid_t, int>> Q;
	vector<bool> visitedFaces(faceSize, false);
	deque<pair<hdsid_t, int>> expSeq;
	Q.push({ 0, 0 });
		//BFS to assign z position of each piece
		while (!Q.empty()) {
		hdsid_t curIdx = Q.front().first;
		hdsid_t curHEID;
		int z = Q.front().second;
		Q.pop();
		visitedFaces[curIdx] = true;
		expSeq.push_back({ curIdx, z });

		// Go through neighboring faces,
		// record shared edge to current face
		auto he = ref_mesh->heFromFace(curIdx);
		auto curHE = he;
		do
		{
			hdsid_t adjFid = curHE->flip()->fid;
			if (!visitedFaces[adjFid] && !curHE->isPicked)
			{
				Q.push({ adjFid, z + 1 });
				parentEdgeMap[adjFid] = curHE->index;
			}
			curHE = curHE->next();
		} while (curHE != he);
		
	}

		//expand faces
		for (auto cur : expSeq) {
			hdsid_t curIdx = cur.first;
			hdsid_t z = cur.second;
			hdsid_t curHEID;
			if (curIdx == 0) {
				pos[curIdx] = QVector3D(0, 0, 0);
				orient[curIdx] = QVector3D(0, 1, 0);
				curHEID = ret->faces()[curIdx].heID();
			}
			else {
				hdsid_t ref_he = parentEdgeMap[curIdx];

				hdsid_t ref_he_nxt = ret->halfedges()[ref_he].next()->index;
				QVector3D ref_he_pos = ret->vertFromHe(ref_he)->pos;
				QVector3D ref_he_nxt_pos = ret->vertFromHe(ref_he_nxt)->pos;

				QVector3D ref_he_orient = (ref_he_pos - ref_he_nxt_pos).normalized();
				//create gap between faces
				QVector3D curPos = ref_he_nxt_pos
					+ QVector3D::crossProduct(QVector3D(0, 0, 1), ref_he_orient) * foldDepth;

				pos[curIdx] = curPos;
				orient[curIdx] = ref_he_orient;//QVector3D(0,-1,0);
				curHEID = ref_mesh->halfedges()[ref_he].flip()->index;
			}
			pos[curIdx].setZ(z);
			MeshUnfolder::unfoldSeparateFace(pos[curIdx], orient[curIdx], curHEID, ret);
			heid[curIdx] = curHEID;
		}
	//add bridges based on ref_mesh flip twins

	for (int i = 0; i < heSize; i++)
	{
		hdsid_t flipid = ref_hes[i].flip()->index;
		if (flipid > i)
		{
			he_t* he1 = &ret->halfedges()[i];
			he_t* he2 = &ret->halfedges()[flipid];

			vector<QVector3D> vpos1 = {
				(ret->vertFromHe(i)->pos
				+ ret->vertFromHe(he2->next()->index)->pos) / 2.0 };
			vector<QVector3D> vpos2 = {
				(ret->vertFromHe(flipid)->pos
				+ ret->vertFromHe(he1->next()->index)->pos) / 2.0 };
			size_t heOriSize = ret->halfedges().size();
			
			// generate bridge
			if (!ref_hes[i].isPicked) generateBridge(i, flipid, ret, vpos1, vpos2);

			//if selected edge, unlink bridge edge pairs to make flaps
			if (ref_hes[i].isPicked) {
				//for (int index = 0; index < vpos1.size(); index++)
				//{
				//ret->splitHeFromFlip(heOriSize + 2);
				//}

			}
		}
	}

	unordered_set<hdsid_t> exposedHEs;
	for (auto &he : ret->halfedges())
	{
		if (!he.flip_offset)
			exposedHEs.insert(he.index);
	}
	fillNullFaces(ret->halfedges(), ret->faces(), exposedHEs);
}

void MeshOrigami::processOrigami(const mesh_t* ref_mesh, mesh_t* ret)
{
	//update face pieces
	for (int i = 0; i < ref_mesh->faces().size(); i++) {

		MeshUnfolder::unfoldSeparateFace(pos[i], orient[i], heid[i], ret);
	}
	auto &ref_hes = ref_mesh->halfedges();

	//update bridges
	for (int i = 0; i < ref_hes.size(); i++) {
		hdsid_t flipid = ref_hes[i].flip()->index;
		if (flipid > i) {

			he_t* he1 = &ret->halfedges()[i];
			he_t* he2 = &ret->halfedges()[flipid];

			vector<QVector3D> vpos1 = {
				(ret->vertFromHe(i)->pos
				+ ret->vertFromHe(he2->next()->index)->pos) / 2.0 };
			vector<QVector3D> vpos2 = {
				(ret->vertFromHe(flipid)->pos
				+ ret->vertFromHe(he1->next()->index)->pos) / 2.0 };
			for (int i = 0; i < vpos1.size(); i++) {
				//ret->verts()[he1->flip()->prev()->vertID()].pos = vpos2[i];
				//ret->verts()[he1->flip()->prev()->prev()->vertID()].pos = vpos1[i];
				//he1 = he1->prev()->prev();
			}
		}
	}
}

// evaluate origami (calculate bridge length, and moving direction of faces)
void MeshOrigami::evaluateOrigami(
	const mesh_t* ref_mesh, mesh_t* eval_mesh, 
	float &dist, vector<QVector3D> &movingDir)
{
	for (int i = 0; i < ref_mesh->faces().size(); i++) {
		QVector3D dir(QVector3D(0,0,0));
		he_t* he = eval_mesh->heFromFace(i);
		do {
			if (!ref_mesh->halfedges()[he->index].isPicked) {
				hdsid_t flipid = ref_mesh->halfedges()[he->index].flip()->index;
				he_t* flip = &eval_mesh->halfedges()[flipid];
				QVector3D v1 = eval_mesh->vertFromHe(he->index)->pos;
				QVector3D v2 = eval_mesh->vertFromHe(he->flip()->index)->pos;
				QVector3D fv2 = eval_mesh->vertFromHe(flip->index)->pos;
				QVector3D fv1 = eval_mesh->vertFromHe(flip->flip()->index)->pos;
				v1.setZ(0); v2.setZ(0); fv2.setZ(0); fv1.setZ(0);
				float dist1 = (v1 - fv1).length();
				float dist2 = (v2 - fv2).length();
				if (dist1 < dist2) {
					dist += dist1;
					dir += fv1 - v1;
				}
				else {
					dist += dist2;
					dir += fv2 - v2;
				}
			}
			he = he->next();
		} while (he != eval_mesh->heFromFace(i));
		movingDir[i] = dir;
	}

}
