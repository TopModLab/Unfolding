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
	const float patchScale = conf.at("patchScale");
	const float foldDepth = conf.at("foldDepth");

	HDS_Mesh* outMesh = MeshOrigami::createOrigami(ref, patchScale, foldDepth);
	bridgeMap bridges = bridgeOrigami(ref, outMesh);

	//while dist can be further reduced
	vector<float> curDist(faceSize, INT_MAX);
	vector<QVector3D> nxtPos = pos;
	vector<QVector3D> nxtOrient = orient;
	while (1) {
		MeshOrigami::processOrigami(ref, outMesh, bridges);
		//MeshViewer::getInstance()->unfoldView(outMesh);
		MeshViewer::getInstance()->bindHalfEdgeMesh(outMesh);
		MeshViewer::getInstance()->repaint();

		//calculate next step rotation

		//calculate next step distance
		vector<float> dist(faceSize, 0);
		vector<QVector3D> movingDir(faceSize);
		vector<QVector3D> rotateDir(faceSize);

		MeshOrigami::evaluateOrigami(ref, outMesh, dist, movingDir, rotateDir);

		bool stopSim = true;
		//move pos in movingDir
		//step size 0.05
		for (int i = 0; i < faceSize; i++) {
			cout << i << "::" << curDist[i] << "  " << dist[i] << endl;
			if (curDist[i] >= dist[i] && curDist[i] - dist[i] > 0.0001) {
				stopSim = false;
				nxtPos[i] = pos[i] + movingDir[i] / 50;
				nxtOrient[i] = (orient[i] + rotateDir[i] / 50).normalized();
				curDist[i] = dist[i];
			}

		}
		if (stopSim) break;

		pos = nxtPos;
		orient = nxtOrient;
		Sleep(uint(100));
	}
	return outMesh;
}

// create faces of origami and unfold them using BFS
HDS_Mesh* MeshOrigami::createOrigami(
	const mesh_t* ref_mesh, float patchScale, float foldDepth)
{
	if (!ref_mesh) return nullptr;

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
		if (curIdx == 5) {
			pos[curIdx] += QVector3D(1, 1, 0);
			orient[5] += QVector3D::crossProduct(orient[5], QVector3D(0, 0, 1)).normalized() / 9;
			orient[5].normalize();
		}
		MeshUnfolder::unfoldSeparateFace(pos[curIdx], orient[curIdx], curHEID, ret);
		heid[curIdx] = curHEID;
	}

	return ret;
}


// move faces and bridges positions
MeshOrigami::bridgeMap MeshOrigami::bridgeOrigami(
	const mesh_t* ref_mesh, mesh_t* ret) {
	auto &ref_hes = ref_mesh->halfedges();
	size_t heSize = ref_hes.size();

	//add bridges based on ref_mesh flip twins
	bridgeMap bridges;
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
			size_t vOriSize = ret->verts().size();
			// generate bridge
			if (!ref_hes[i].isPicked) {
				generateBridge(i, flipid, ret, vpos1, vpos2);
				bridges[i] = { vOriSize, vpos1.size() * 2 };
			}
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
	return bridges;
}

void MeshOrigami::processOrigami(const mesh_t* ref_mesh, mesh_t* ret, bridgeMap bridges)
{
	//update face pieces
	for (int i = 0; i < ref_mesh->faces().size(); i++) {

		MeshUnfolder::unfoldSeparateFace(pos[i], orient[i], heid[i], ret);
	}
	auto &ref_hes = ref_mesh->halfedges();

	//update bridges
	for (auto it : bridges) {
		hdsid_t heid = it.first;
		hdsid_t vid = it.second.first;
		size_t vsize = it.second.second;
		hdsid_t flipid = ref_hes[heid].flip()->index;
		he_t* he1 = &ret->halfedges()[heid];
		he_t* he2 = &ret->halfedges()[flipid];

		vector<QVector3D> vpos1 = {
			(ret->vertFromHe(heid)->pos
			+ ret->vertFromHe(he2->next()->index)->pos) / 2.0 };
		vector<QVector3D> vpos2 = {
			(ret->vertFromHe(flipid)->pos
			+ ret->vertFromHe(he1->next()->index)->pos) / 2.0 };
		for (int i = 0; i < vsize; i += 2) {
			ret->verts()[vid + i].pos = vpos1[i / 2];
			ret->verts()[vid + i + 1].pos = vpos2[i / 2];
		}
	}
}

// evaluate origami (calculate bridge length, and moving direction of faces)
void MeshOrigami::evaluateOrigami(
	const mesh_t* ref_mesh, mesh_t* eval_mesh,
	vector<float> &dist, vector<QVector3D> &movingDir, vector<QVector3D> &rotateDir)
{
	vector<QVector3D> faceCenters(ref_mesh->faces().size());
	vector<int> cutCounts(ref_mesh->faces().size(), 0);

	for (int i = 0; i < ref_mesh->faces().size(); i++) {
		faceCenters[i] = eval_mesh->faceCenter(i);
		he_t* he = eval_mesh->heFromFace(i);
		do {
			if (ref_mesh->halfedges()[he->index].isPicked) cutCounts[i]++;
			he = he->next();
		} while (he != eval_mesh->heFromFace(i));
	}

	for (int i = 0; i < ref_mesh->faces().size(); i++) {

		he_t* he = eval_mesh->heFromFace(i);
		do {
			if (!ref_mesh->halfedges()[he->index].isPicked) {
				hdsid_t flipheid = ref_mesh->halfedges()[he->index].flip()->index;
				//find vector between two face centers
				hdsid_t f1 = i;
				hdsid_t f2 = eval_mesh->faceFromHe(flipheid)->index;
				cout << "face " << i << " flip face " << f2 << endl;
				QVector3D fc1 = faceCenters[f1];
				QVector3D fc2 = faceCenters[f2];
				fc1.setZ(0); fc2.setZ(0);
				QVector3D curDir = fc2 - fc1;
				QVector3D orient1 = eval_mesh->edgeVector(he->index).normalized();
				QVector3D orient2 = -eval_mesh->edgeVector(flipheid).normalized();

				QVector3D optimalDir;
				if (QVector3D::dotProduct(orient1, orient2) < 0)
					optimalDir = (orient2 - orient1).normalized();
				else
					optimalDir = QVector3D::crossProduct((orient1 + orient2).normalized(),
						QVector3D(0, 0, 1));
				if (cutCounts[f1] >= cutCounts[f2]) {
					movingDir[i] += fc2 - (optimalDir*curDir.length() + fc1);
					dist[i] += 1 - QVector3D::dotProduct(optimalDir, curDir.normalized());
					cout << 1 - QVector3D::dotProduct(optimalDir, curDir.normalized()) << " of " << i << endl;
					//cout << dir.x() << ":::" << dir.y() << ":::" << dir.z() << "==" << dir.length() << endl;
				}
			}
			he = he->next();
		} while (he != eval_mesh->heFromFace(i));
		//if (i > 0)
		//{
// 			movingDir[i] = dir;
// 			cout << "::::" << dir.length() << endl;
// 			rotateDir[i] = rot;
		//}
	}

}
