#include "MeshExtender.h"
#include "hds_mesh.h"

#include "utils.hpp"
#include "mathutils.hpp"
bool MeshExtender::hasBridgeEdge = false;
bool MeshExtender::hasCutEdge = false;
bool MeshExtender::isHollow = false;
HDS_Mesh* MeshExtender::ori_mesh;

void MeshExtender::setOriMesh(HDS_Mesh* mesh)
{
	ori_mesh = mesh;
}

vector<HDS_Vertex*> MeshExtender::addBridger(HDS_Mesh* thismesh, HDS_HalfEdge* he1, HDS_HalfEdge* he2, HDS_Vertex* v1, HDS_Vertex* v2, HDS_Face* cutFace)
{
	//new Bridger object
	HDS_Bridger* Bridger = new HDS_Bridger(he1, he2, v1, v2);
	//add all internal edges and vertices to mesh
	vector<HDS_HalfEdge*> hes = Bridger->hes;
	if (!Bridger->hes.empty()) {
		for (auto he : hes) {
			thismesh->addHalfEdge(he);
		}

	}

	hes.insert(hes.begin(),he1);
	hes.push_back(he2);
	for (auto he = hes.begin(); he != hes.end(); he+=2) {
		if (he != prev(hes.end())) {
			auto he_next = next(he);
			//bridge each pair of edges
			//get bridge faces, set to Bridger->faces
			HDS_Face* bridgeFace = thismesh->bridging(*he, *he_next, cutFace);
			//fix face
			bridgeFace->index = HDS_Face::assignIndex();
			bridgeFace->isCutFace = false;
			bridgeFace->isBridger = true;
			//add face to mesh
			thismesh->addFace(bridgeFace);

			Bridger->faces.push_back(bridgeFace);

		}
	}
	return Bridger->verts;

}
void MeshExtender::scaleFaces(HDS_Mesh* mesh)
{
	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;

	set<int> oldFaces;
	for (auto &f : mesh->faceSet) {
		if (f->isCutFace) continue;
		else oldFaces.insert(f->index);
	}
	cout << "number of old faces = " << oldFaces.size() << endl;
	vector<vector<vert_t*> > corners_new(oldFaces.size());
	vector<vert_t*> corners_tmp;
	vector<vector<he_t*> > edges_new(oldFaces.size());
	HDS_Vertex::resetIndex();
	HDS_HalfEdge::resetIndex();

	for (auto fidx : oldFaces) {
		auto face = mesh->faceMap[fidx];
		// update each face with the scaling factor
		face->setScaleFactor(HDS_Bridger::getScale());
		int numOfCorners = face->corners().size();
		for (int i = 0; i < numOfCorners; i++) {
			//build new corners
			vert_t* v_new = new vert_t(*face->corners()[i]);
			v_new->pos = face->getScaledCorners()[i];
			v_new->refid = face->corners()[i]->refid;
			//find incident half edge
			vert_t * vs = face->corners()[i];
			vert_t * ve = i < numOfCorners-1? face->corners()[i+1] : face->corners()[0];
			he_t* old_edge = mesh->incidentEdge(vs, ve);

			//record old cut edges
			v_new->he = old_edge;
			v_new->index = HDS_Vertex::assignIndex();
			cout<<v_new->index<<"  refid: "<<(v_new->refid>>2)<<endl;

			if (old_edge->flip->f->isCutFace) {
				v_new->he->f = old_edge->flip->f;
			}
			corners_new[fidx].push_back(v_new);
			corners_tmp.push_back(v_new);
		}
	}


	for (auto v1 : corners_tmp) {
		for (auto v2 : corners_tmp) {
			if (v1->index != v2->index) {
				if(v1->he->index == v2->he->flip->index) {
					//non cut edge
					v1->bridgeTwin = v2;
					hasBridgeEdge = true;
					cout<<"non cut edge twin pair vertices:"<<v1->index<<" and "<<v2->index<<endl;

				}else if (v1->he->flip->cutTwin != nullptr && v2->he->flip->cutTwin != nullptr) {
					if (v1->he->flip->index == v2->he->flip->cutTwin->index){
						//cut edge, duplicate edge
						v1->flapTwin = v2;
						hasCutEdge = true;
						cout<<"cut edge twin pair vertices:"<<v1->index<<" and "<<v2->index<<endl;

					}
				}
			}
		}
	}


	for (auto fidx : oldFaces) {
		auto face = mesh->faceMap[fidx];
		int numOfCorners = face->corners().size();
		for (int i = 0; i < numOfCorners; i++) {
			//build new edges, check if it's cut edge or not
			he_t* he_new = new he_t;
			he_t* hef_new = new he_t;
			he_new->index = HDS_HalfEdge::assignIndex();
			hef_new->index = HDS_HalfEdge::assignIndex();

			he_new->setFlip(hef_new);
			he_new->f = face;
			he_new->setCutEdge(corners_new[fidx][i]->he->isCutEdge);
			if (he_new->isCutEdge) {
				hef_new->f = corners_new[fidx][i]->he->f;
			}
			edges_new[fidx].push_back(he_new);
		}


		//set half edge's vertices
		for (int i = 0; i < numOfCorners; i++) {
			he_t* curHE = edges_new[fidx][i];
			curHE->flip->v = i < edges_new[fidx].size()-1? corners_new[fidx][i+1] : corners_new[fidx][0];
			curHE->v = corners_new[fidx][i];
		}
		if (!isHollow){
		//link edge loop
		for (int i = 0; i < numOfCorners; i++) {
			he_t* curHE = edges_new[fidx][i];
			he_t* prevHE = i > 0? edges_new[fidx][i-1] : edges_new[fidx][edges_new[fidx].size()-1];
			he_t* nextHE = i < edges_new[fidx].size()-1? edges_new[fidx][i+1] : edges_new[fidx][0];
			curHE->prev = prevHE;
			curHE->next = nextHE;
			curHE->flip->prev = nextHE->flip;
			curHE->flip->next = prevHE->flip;
		}
		}


		//link corners to edges
		for (int i = 0; i < numOfCorners; i++) {
			edges_new[fidx][i]->refid = corners_new[fidx][i]->he->refid;
			edges_new[fidx][i]->flip->refid = corners_new[fidx][i]->he->refid;
			corners_new[fidx][i]->he = edges_new[fidx][i];
		}
		//link to current face
		face->he = edges_new[fidx][0];
	}

	//clean all old vertices/edges
	mesh->vertSet.clear();
	mesh->vertMap.clear();
	mesh->heSet.clear();
	mesh->heMap.clear();

	//add vertices, edges to mesh
	for (auto fidx: oldFaces) {
		auto face = mesh->faceMap[fidx];
		int numOfCorners = face->corners().size();

		for (int i = 0; i < numOfCorners; i++) {
			vert_t* vertex = corners_new[fidx][i];
			he_t* he = edges_new[fidx][i];
			he_t* hef = he->flip;
			mesh->addVertex(vertex);
			mesh->addHalfEdge(he);
			mesh->addHalfEdge(hef);

		}
	}
}

bool MeshExtender::extendMesh(HDS_Mesh *mesh)
{

	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;

	unordered_map<int, vert_t*> ori_map = ori_mesh->vertMap;
	scaleFaces(mesh);


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

				cout<<"v1_ori: "<<((he1->v->refid)>>2)<<endl;
				cout<<"v2_ori: "<<((he2->v->refid)>>2)<<endl;

				cout<<ori_map.size();
				HDS_Vertex* v1_ori = ori_map[(he1->v->refid)>>2];
				HDS_Vertex* v2_ori = ori_map[(he2->v->refid)>>2];
				cout<<"v1_ori at "<<v1_ori->pos<<endl;
				cout<<"v2_ori at "<<v2_ori->index<<" "<<v2_ori->pos<<endl;

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
				cout<<"v1_ori: "<<((he1->v->refid)>>2)<<endl;
				cout<<"v2_ori: "<<((he1->flip->v->refid)>>2)<<endl;
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

