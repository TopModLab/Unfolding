#include "MeshExtender.h"
#include "hds_mesh.h"

#include "utils.hpp"
#include "mathutils.hpp"

bool MeshExtender::isHollow = false;

HDS_Mesh* MeshExtender::ori_mesh;
HDS_Mesh* MeshExtender::cur_mesh;

vector<HDS_HalfEdge*> MeshExtender::hes_new;
vector<HDS_Vertex*> MeshExtender::verts_new;
vector<HDS_Face*> MeshExtender::faces_new;


void MeshExtender::setOriMesh(HDS_Mesh* mesh)
{
	ori_mesh = mesh;
}

void MeshExtender::addBridger(HDS_HalfEdge* he1, HDS_HalfEdge* he2, HDS_Vertex* v1, HDS_Vertex* v2)
{

	//new Bridger object
	HDS_Bridger* Bridger = new HDS_Bridger(he1, he2, v1, v2);
	Bridger->setCutFace(he1->f, he2->f);
	Bridger->createBridge();

	hes_new.insert( hes_new.end(), Bridger->hes.begin(), Bridger->hes.end());
	faces_new.insert( faces_new.end(), Bridger->faces.begin(), Bridger->faces.end());
	verts_new.insert( verts_new.end(), Bridger->verts.begin(), Bridger->verts.end() );

}

void MeshExtender::scaleFaces()
{
	for (auto f: cur_mesh->faces()) {
		if (!f->isCutFace){
			f->setScaleFactor(HDS_Bridger::getScale());
			int numOfCorners = f->corners().size();
			vector<vert_t*> vertices;
			for (int i = 0; i < numOfCorners; i++) {
				vert_t* v_new = new vert_t;
				v_new->pos = f->getScaledCorners()[i];
				v_new->refid = f->corners()[i]->refid;
				vertices.push_back(v_new);
				verts_new.push_back(v_new);
			}

			face_t* newFace = createFace(vertices);
			newFace->refid = f->refid;
			newFace->isCutFace = false;
			newFace->isBridger = false;
			faces_new.push_back(newFace);

			//assign half edge's refid
			//get current face's edges
			he_t* newHE = newFace->he;
			he_t* curHE = f->he;

			do {
				newHE->refid = curHE->refid;
				newHE->flip->refid = curHE->flip->refid;
				newHE->setCutEdge(curHE->isCutEdge);
				if (newHE->isCutEdge)
					newHE->flip->f = curHE->flip->f;
				newHE = newHE->next;
				curHE = curHE->next;
			}while(newHE!= newFace->he);
		}
	}
}

HDS_Face* MeshExtender::createFace(vector<HDS_Vertex*> vertices)
{


	face_t * newFace = new face_t();
	vertices.push_back(vertices.front());//form a loop

	auto preV = vertices.front();
	for (int i = 1; i < vertices.size(); i++)
	{
		auto& curV = vertices[i];
		he_t* newHE = HDS_Mesh::insertEdge(preV, curV);
		if(newFace->he == nullptr)
			newFace->he = newHE;
		newHE->f = newFace;
		hes_new.push_back(newHE);
		preV = curV;
	}

	return newFace;
}



bool MeshExtender::extendMesh(HDS_Mesh *mesh)
{
	cur_mesh = mesh;
	unordered_map<int, vert_t*> ori_map = ori_mesh->vertMap;
	scaleFaces();

	//get bridge pairs
	unordered_map<int, he_t*> refidMap;
	for (auto he: hes_new) {
		if (refidMap.find(he->refid) == refidMap.end())
			refidMap.insert(make_pair(he->refid, he));
		else {
			he->flip->setBridgeTwin(refidMap[he->refid]->flip);

		}
	}

	for (auto f: cur_mesh->faces()) {
		if(f->isCutFace){
			faces_new.push_back(f);
			cout<<"old cut faces:"<<f->index<<endl;
		}

	}

	//assign flip s face
	for (auto& he_inner: hes_new){
		he_t* he = he_inner->flip;

		if (he->f == nullptr){


			//find nearest cut face, if not found set a new one
			he_t* curHE = he;
			do {
				curHE = curHE->bridgeTwin->next;
				if (curHE->f != nullptr) {
					cout<<"cut face: "<<curHE->f->index<<endl;
					he->f = curHE->f;
					break;
				}
			}while (curHE != he);

			if (he->f == nullptr) {
				face_t* cutFace = new face_t;
				cutFace->isCutFace = true;
				he->f = cutFace;
				cutFace->he = he;
				cout<<"new cut face"<<endl;
				faces_new.push_back(cutFace);
			}
		}
	}


	for (auto& heMap: refidMap) {
		he_t* he = heMap.second->flip;
		if (!he->isCutEdge){
			///for all non-cut-edge edges, create bridge faces

			HDS_Vertex* v1_ori = ori_map[(he->v->refid)>>2];
			HDS_Vertex* v2_ori = ori_map[(he->bridgeTwin->v->refid)>>2];
			addBridger(he, he->bridgeTwin, v1_ori, v2_ori);

		}else {
			/// for all cut-edge edges, create flaps
//			he_t* twin_he = he->bridgeTwin;
//			vert_t* flap_vs = new vert_t;
//			vert_t* flap_ve = new vert_t;
//			flap_vs->pos = twin_he->v->pos;
//			flap_ve->pos = twin_he->flip->v->pos;

//			//warning. assign refid, to be tested
//			flap_vs->refid = twin_he->v->refid;
//			flap_ve->refid = twin_he->flip->v->refid;

//			verts_new.push_back(flap_vs);
//			verts_new.push_back(flap_ve);

//			he_t* flap_he = HDS_Mesh::insertEdge(flap_vs, flap_ve);
//			flap_he->setCutEdge(true);
//			flap_he->f = twin_he->f;
//			flap_he->flip->f = twin_he->f;
//			flap_he->refid = twin_he->refid;
//			hes_new.push_back(flap_he);

//			HDS_Vertex* v1_ori = ori_map[(he->v->refid)>>2];
//			HDS_Vertex* v2_ori = ori_map[(flap_he->v->refid)>>2];


//			vert_t* twin_flap_vs = new vert_t;
//			vert_t* twin_flap_ve = new vert_t;
//			twin_flap_vs->pos = he->v->pos;
//			twin_flap_ve->pos = he->flip->v->pos;

//			//warning. assign refid, to be tested
//			twin_flap_vs->refid = he->v->refid;
//			twin_flap_ve->refid = he->flip->v->refid;

//			verts_new.push_back(twin_flap_vs);
//			verts_new.push_back(twin_flap_ve);

//			he_t* twin_flap_he = HDS_Mesh::insertEdge(twin_flap_vs, twin_flap_ve);
//			twin_flap_he->setCutEdge(true);
//			twin_flap_he->f = he->f;
//			twin_flap_he->flip->f = he->f;

//			twin_flap_he->refid = he->refid;
//			hes_new.push_back(twin_flap_he);

//			addBridger(he, flap_he, v1_ori, v2_ori);
//			addBridger(twin_he, twin_flap_he, v2_ori, v1_ori);

		}

	}

	cur_mesh->heSet.clear();
	cur_mesh->vertSet.clear();
	cur_mesh->faceSet.clear();
	cur_mesh->heMap.clear();
	cur_mesh->vertMap.clear();
	cur_mesh->faceMap.clear();

	HDS_Vertex::resetIndex();
	HDS_HalfEdge::resetIndex();
	HDS_Face::resetIndex();

	for (auto f: faces_new) {
		f->index = HDS_Face::assignIndex();
		cur_mesh->addFace(f);
	}

	//add new vertices and edges
	for (auto v: verts_new) {
		v->index = HDS_Vertex::assignIndex();
		cur_mesh->addVertex(v);
	}
	for (auto he: hes_new) {
		he->index = HDS_HalfEdge::assignIndex();
		he->flip->index = HDS_HalfEdge::assignIndex();
		cur_mesh->addHalfEdge(he);
		cur_mesh->addHalfEdge(he->flip);

	}

	cur_mesh->validate();
	/// update the curvature of each vertex
	for (auto &v : mesh->vertSet) {
		v->computeNormal();
		v->computeCurvature();
		//cout << v->index << ": " << (*v) << endl;
	}


	cout<<"extend succeed............."<<endl;
	return true;

}

/*
bool MeshExtender::extendMesh(HDS_Mesh *mesh)
{

	cur_mesh = mesh;
	unordered_map<int, vert_t*> ori_map = ori_mesh->vertMap;
	scaleFaces();


	vector<vert_t*> verts_new;


	//add bridges
	unordered_set<HDS_Vertex*> visited;
	if (hasBridgeEdge) {
		for(auto v: mesh->vertSet) {
			if (v->bridgeTwin != nullptr && visited.find(v) == visited.end()) {
				///for all non-cut-edge edges, create bridge faces
				//get half edges that are "hidden", no face assigned
				HDS_HalfEdge* h1 = v->he;
				HDS_HalfEdge* h2 = v->bridgeTwin->he;
				HDS_HalfEdge *he1, *he2;

				he1 = h1->f == nullptr? h1:h1->flip;
				he2 = h2->f == nullptr? h2:h2->flip;
				HDS_Face* cutFace;
				//find nearest cut face, if not found set to nullptr
				HDS_HalfEdge* curHE = h1;
				do {
					curHE = curHE->v->bridgeTwin->he->prev;
					if (curHE->isCutEdge) {
						cutFace = curHE->flip->f;
						break;
					}
				}while (curHE != h1);
				if (cutFace == nullptr) {
					cutFace = new HDS_Face;
					cutFace->index = HDS_Face::assignIndex();
					cutFace->isCutFace = true;
					mesh->addFace(cutFace);

				}


				HDS_Vertex* v1_ori = ori_map[(he1->v->refid)>>2];
				HDS_Vertex* v2_ori = ori_map[(he2->v->refid)>>2];


				vector<HDS_Vertex*> verts = addBridger(mesh, he1, he2, v1_ori, v2_ori, cutFace);
				verts_new.insert( verts_new.end(), verts.begin(), verts.end() );

				visited.insert(v->bridgeTwin);


			}
		}
	}

	if (hasCutEdge) {

		for(auto v: mesh->vertSet) {
			if(v->flapTwin != nullptr){

				/// for all cut-edge edges, create flaps
				//get v->he boundary
				HDS_HalfEdge* he1;
				he1 = (v->he->f->isCutFace)? v->he:v->he->flip;
				//duplicate v->flapTwin->he as new he

				he_t* twin_he = v->flapTwin->he;
				he_t* flap_he = new he_t;
				he_t* flap_he_flip = new he_t;

				//warning. to be tested
				flap_he->refid = twin_he->refid;
				flap_he_flip->refid = twin_he->refid;

				flap_he->index = HDS_HalfEdge::assignIndex();
				flap_he_flip->index = HDS_HalfEdge::assignIndex();

				flap_he->setFlip(flap_he_flip);
				flap_he->setCutEdge(true);

				//connect edge loop
				flap_he->prev = flap_he_flip;
				flap_he->next = flap_he_flip;
				flap_he_flip->prev = flap_he;
				flap_he_flip->next = flap_he;

				flap_he->f = he1->f;

				vert_t* flap_vs = new vert_t;
				vert_t* flap_ve = new vert_t;
				flap_vs->pos = twin_he->v->pos;
				flap_ve->pos = twin_he->flip->v->pos;

				//warning. assign refid, to be tested
				flap_vs->refid = twin_he->v->refid;
				flap_ve->refid = twin_he->flip->v->refid;

				flap_vs->index = HDS_Vertex::assignIndex();
				flap_ve->index = HDS_Vertex::assignIndex();

				flap_vs->he = flap_he;
				flap_ve->he = flap_he_flip;
				flap_he->v = flap_vs;
				flap_he_flip->v = flap_ve;
				cout<<"new flap vertices vs: "<<flap_vs->index<<" ve: "<<flap_ve->index
				   <<" based on original flap pair: "<<v->index<<" and "<<v->flapTwin->index<<endl;

				//add edges
				verts_new.push_back(flap_vs);
				verts_new.push_back(flap_ve);

				mesh->addHalfEdge(flap_he);
				mesh->addHalfEdge(flap_he_flip);
				twin_he->setCutEdge(false);

				//bridge v->he and new he
				HDS_Vertex* v1_ori = ori_map[(he1->v->refid)>>2];
				HDS_Vertex* v2_ori = ori_map[(he1->flip->v->refid)>>2];
				vector<HDS_Vertex*> verts = addBridger(mesh, he1, flap_he_flip, v1_ori, v2_ori, he1->f);
				verts_new.insert( verts_new.end(), verts.begin(), verts.end() );

			}

		}

	}
	//add new vertices
	for (auto v: verts_new) {
		mesh->addVertex(v);
	}

	/// update the curvature of each vertex
	for (auto &v : mesh->vertSet) {
		v->computeNormal();
		v->computeCurvature();
		//cout << v->index << ": " << (*v) << endl;
	}


	cout<<"extend succeed............."<<endl;
	return true;
}
*/
