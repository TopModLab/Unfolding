#include "meshorigamizer.h"

typedef HDS_Face face_t;
typedef HDS_HalfEdge he_t;
typedef HDS_Vertex vert_t;

pair<vector<he_t*>, HDS_Face*> MeshOrigamizer::bridging(HDS_HalfEdge* he1, HDS_HalfEdge* he2, face_t* cutFace1, face_t* cutFace2) {
	//get 4 vertices from h1 h2
	HDS_Vertex* v1s, *v1e, *v2s, *v2e;
	v1s = he1->v;
	v1e = he1->flip->v;
	v2s = he2->v;
	v2e = he2->flip->v;


	//build new face
	face_t * bridgeFace = new face_t;

	//link he1 and he2 to face
	he1->f = bridgeFace;
	he2->f = bridgeFace;
	bridgeFace->he = he1;

	//insert two cut edges
	he_t* he_v1e_v2s = HDS_Mesh::insertEdge(v1e, v2s, he1, he2);
	he_t* he_v2e_v1s = HDS_Mesh::insertEdge(v2e, v1s, he2, he1);

	he_v1e_v2s->f = bridgeFace;
	he_v2e_v1s->f = bridgeFace;



	he_v2e_v1s->flip->f = cutFace1;
	he_v1e_v2s->flip->f = cutFace2;
	cutFace1->he = he_v2e_v1s->flip;
	cutFace2->he = he_v1e_v2s->flip;

	he_v1e_v2s->setCutEdge(true);
	he_v2e_v1s->setCutEdge(true);

	vector<he_t*> hes;

	hes.push_back(he_v1e_v2s);
	hes.push_back(he_v2e_v1s);

	return make_pair(hes, bridgeFace);
}

void MeshOrigamizer::addBridger(HDS_HalfEdge* he1, HDS_HalfEdge* he2, double theta, double length) 
{

	QVector3D normal1 = he1->flip->computeNormal();
	QVector3D normal2 = he2->flip->computeNormal();
	//get vector perpendicular to the edge and point into the mesh
	QVector3D vertical = -(normal1 + normal2) / 2;
	vertical.normalize();
	HDS_Vertex* vs = new HDS_Vertex;
	HDS_Vertex* ve = new HDS_Vertex;
	vs->pos = he1->flip->v->pos + vertical * length;
	ve->pos = he2->flip->v->pos + vertical * length;
	HDS_HalfEdge* he_new = HDS_Mesh::insertEdge(vs, ve);
	face_t* cutFace1 = he1->f;
	face_t* cutFace2 = he2->f;

	//create bridge face 1
	pair<vector<he_t*>, HDS_Face*> bridger1 = bridging(he1, he_new, cutFace1, cutFace2);
	//create bridge face 2
	pair<vector<he_t*>, HDS_Face*> bridger2 = bridging(he_new->flip, he2, cutFace1, cutFace2);
	
	//save added bridger info
	verts_new.push_back(vs);
	verts_new.push_back(ve);

	hes_new.push_back(he_new);
	hes_new.push_back(bridger1.first[0]);
	hes_new.push_back(bridger1.first[1]);
	hes_new.push_back(bridger2.first[0]);
	hes_new.push_back(bridger2.first[1]);

	faces_new.push_back(bridger1.second);
	faces_new.push_back(bridger2.second);
}

bool MeshOrigamizer::origamiMesh( HDS_Mesh * mesh )
{
	//////////////////////////////////////////////////////////
	/*just for origami functional test, must be removed later*/
	confMap origamiConfig;
	origamiConfig["shape"] = 0;
	origamiConfig["curv"] = 0.5;
	origamiConfig["samples"] = 2;
	origamiConfig["size"] = 1;
	origamiConfig["cp"] = 0.5;
	HDS_Bridger::setBridger(origamiConfig);
	

	initiate();
	cur_mesh = mesh;
	unordered_map<hdsid_t, vert_t*> ori_map = ori_mesh->vertMap;

	seperateFaces();
	//scaleFaces();

	//get bridge pairs
	unordered_map<hdsid_t, he_t*> refidMap;
	for (auto he : hes_new) {
		if (refidMap.find(he->flip->refid) == refidMap.end()) {
			refidMap.insert(make_pair(he->refid, he));
		}
		else {
			he->flip->setBridgeTwin(refidMap[he->refid]->flip);
		}
	}

	//assign flip s face
	for (auto& he_inner : hes_new) {
		he_t* he = he_inner->flip;
		if (he->f == nullptr) {
			//find nearest cut face, if not found set a new one
			he_t* curHE = he;

			//rotate around vertex to check if already exist a cutface
			do {
				curHE = curHE->bridgeTwin->next;
				if (curHE->f != nullptr) {
					he->f = curHE->f;
					break;
				}
			} while (curHE != he);

			//after check, if no cut face, create one
			if (he->f == nullptr) {
				face_t* cutFace = new face_t;
				cutFace->isCutFace = true;
				he->f = cutFace;
				cutFace->he = he;
				faces_new.push_back(cutFace);
			}
		}
	}

	for (auto& heMap : refidMap) {
		he_t* he = heMap.second->flip;
		if (!he->isCutEdge) {
			///for all non-cut-edge edges, create bridge faces
			HDS_Vertex* v1_ori = ori_map[(he->v->refid) >> 2];	//refid >> 2  ==> vertexIndex
			HDS_Vertex* v2_ori = ori_map[(he->bridgeTwin->v->refid) >> 2];	
			addBridger(he, he->bridgeTwin, Pi / 2, 0.2);
		}
		else {
			// for all cut-edge edges, create flaps
			cout << "cut edge bridger" << endl;
			he_t* twin_he = he->bridgeTwin;
			vert_t* flap_vs = new vert_t;
			vert_t* flap_ve = new vert_t;
			flap_vs->pos = twin_he->v->pos;
			flap_ve->pos = twin_he->flip->v->pos;

			//warning. assign refid, to be tested
			flap_vs->refid = twin_he->v->refid;
			flap_ve->refid = twin_he->flip->v->refid;

			verts_new.push_back(flap_vs);
			verts_new.push_back(flap_ve);

			he_t* flap_he = HDS_Mesh::insertEdge(flap_vs, flap_ve);
			flap_he->setCutEdge(true);
			flap_he->f = he->f;
			flap_he->flip->f = he->f;
			flap_he->refid = twin_he->refid;
			hes_new.push_back(flap_he);

			HDS_Vertex* v1_ori = ori_map[(he->v->refid) >> 2];
			HDS_Vertex* v2_ori = ori_map[(flap_he->v->refid) >> 2];
			vector <QVector3D> vpair = scaleBridgerEdge(v1_ori, v2_ori);


			vert_t* twin_flap_vs = new vert_t;
			vert_t* twin_flap_ve = new vert_t;
			twin_flap_vs->pos = he->v->pos;
			twin_flap_ve->pos = he->flip->v->pos;

			//warning. assign refid, to be tested
			twin_flap_vs->refid = he->v->refid;
			twin_flap_ve->refid = he->flip->v->refid;

			verts_new.push_back(twin_flap_vs);
			verts_new.push_back(twin_flap_ve);

			he_t* twin_flap_he = HDS_Mesh::insertEdge(twin_flap_vs, twin_flap_ve);
			twin_flap_he->setCutEdge(true);
			twin_flap_he->f = twin_he->f;
			twin_flap_he->flip->f = twin_he->f;

			twin_flap_he->refid = he->refid;
			hes_new.push_back(twin_flap_he);

			addBridger(he, flap_he, Pi / 2, 2);
			vector<QVector3D> vpair_reverse = scaleBridgerEdge(v2_ori, v1_ori);
			addBridger(twin_he, twin_flap_he, Pi / 2, 2);
		}
	}



	updateNewMesh();
	cur_mesh->setProcessType(HDS_Mesh::ORIGAMI_PROC);
#ifdef _DEBUG
	cout << "origami succeed............." << endl;
#endif
	return true;
}

void MeshOrigamizer::seperateFaces() {
	for (auto f : cur_mesh->faces()) {
		if (!f->isCutFace) {
			auto fCorners = f->corners();
			vector<vert_t*> vertices;
			for (int i = 0; i < fCorners.size(); ++i) {
				vert_t* v_new = new vert_t;
				v_new->pos = fCorners[i]->pos;
				v_new->refid = fCorners[i]->refid;
				vertices.push_back(v_new);
				verts_new.push_back(v_new);
			}

			face_t* newFace = createFace(vertices);
			newFace->refid = f->refid;
			newFace->isCutFace = false;
			newFace->isBridger = false;
			faces_new.push_back(newFace);

			he_t* newHE = newFace->he;
			he_t* curHE = f->he;

			do {
				newHE->refid = curHE->refid;
				newHE->flip->refid = curHE->flip->refid;
				newHE->setCutEdge(curHE->isCutEdge);
				if (newHE->isCutEdge) {
					face_t* newCutFace = new face_t(*(curHE->flip->f));
					newHE->flip->f = newCutFace;
					faces_new.push_back(newCutFace);
				}
				newHE = newHE->next;
				curHE = curHE->next;
			} while (newHE != newFace->he);
		}
	}
}


