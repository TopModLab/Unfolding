#include "MeshExtender.h"
#include "hds_mesh.h"

#include "utils.hpp"
#include "mathutils.hpp"

bool MeshExtender::extendMesh(HDS_Mesh *mesh)
{
	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_Face face_t;
	typedef HDS_Connector con_t;

	//HDS_Mesh *extended_mesh = new HDS_Mesh;


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
		face->setScaledCorners(HDS_Connector::getScale());
		int numOfCorners = face->corners().size();
		for (int i = 0; i < numOfCorners; i++) {
			//build new corners
			vert_t* v_new = new vert_t(*face->corners()[i]);
			v_new->pos = face->getScaledCorners()[i];
			//find incident half edge
			vert_t * vs = face->corners()[i];
			vert_t * ve = i < numOfCorners-1? face->corners()[i+1] : face->corners()[0];
			he_t* old_edge = mesh->incidentEdge(vs, ve);
			//record old cut edges
			v_new->he = old_edge;
			v_new->index = HDS_Vertex::assignIndex();
			corners_new[fidx].push_back(v_new);
			corners_tmp.push_back(v_new);
		}
	}

	bool hasCutEdge, hasBridgeEdge = false;

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
			if (he_new->isCutEdge)
				hef_new->f = mesh->faceMap[mesh->finalCutFaceIndex];
			edges_new[fidx].push_back(he_new);
		}


		//set half edge's vertices
		for (int i = 0; i < numOfCorners; i++) {
			he_t* curHE = edges_new[fidx][i];
			curHE->flip->v = i < edges_new[fidx].size()-1? corners_new[fidx][i+1] : corners_new[fidx][0];
			curHE->v = corners_new[fidx][i];
		}
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
		//link corners to edges
		for (int i = 0; i < numOfCorners; i++) {
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
			mesh->vertSet.insert(vertex);
			mesh->vertMap.insert(make_pair(vertex->index, vertex));
			mesh->heSet.insert(he);
			mesh->heMap.insert(make_pair(he->index, he));
			mesh->heSet.insert(hef);
			mesh->heMap.insert(make_pair(hef->index, hef));

		}
	}
	//add bridges
	if (hasBridgeEdge) {
	for(auto v: mesh->vertSet) {
		if (v->bridgeTwin != nullptr) {
			///for all non-cut-edge edges, create bridge faces
				//get half edges that are "hidden", no face assigned
				HDS_HalfEdge* h1 = v->he;
				HDS_HalfEdge* h2 = v->bridgeTwin->he;
				HDS_HalfEdge *he1, *he2;
				he1 = h1->f == nullptr? h1:h1->flip;
				he2 = h2->f == nullptr? h2:h2->flip;
				mesh->bridging(he1, he2);
				v->bridgeTwin->bridgeTwin = nullptr;

			}
	}
}
	if (hasCutEdge) {
	vector<vert_t*> verts_new;
	for(auto v: mesh->vertSet) {
		if(v->flapTwin != nullptr){

			/// for all cut-edge edges, create flaps
			//get v->he boundary
			HDS_HalfEdge* he1;
			he1 = v->he->f->index == mesh->finalCutFaceIndex? v->he:v->he->flip;
			//duplicate v->flapTwin->he as new he

			he_t* twin_he = v->flapTwin->he;
				he_t* flap_he = new he_t;
				he_t* flap_he_flip = new he_t;

				flap_he->index = HDS_HalfEdge::assignIndex();
				flap_he_flip->index = HDS_HalfEdge::assignIndex();

				flap_he->setFlip(flap_he_flip);
				flap_he->setCutEdge(true);

				//connect edge loop
				flap_he->prev = flap_he_flip;
				flap_he->next = flap_he_flip;
				flap_he_flip->prev = flap_he;
				flap_he_flip->next = flap_he;



				flap_he->f = mesh->faceMap[mesh->finalCutFaceIndex];

				vert_t* flap_vs = new vert_t;
				vert_t* flap_ve = new vert_t;
				flap_vs->pos = twin_he->v->pos;
				flap_ve->pos = twin_he->flip->v->pos;

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

				mesh->heSet.insert(flap_he);
				mesh->heMap.insert(make_pair(flap_he->index, flap_he));
				mesh->heSet.insert(flap_he_flip);
				mesh->heMap.insert(make_pair(flap_he_flip->index, flap_he_flip));

				twin_he->setCutEdge(false);

				//bridge v->he and new he
				cout<<"build bridge flap between "<<he1->v->index<<" and "<<flap_he_flip->v->index<<endl;
				mesh->bridging(he1, flap_he_flip);

		}

	}
	//add new vertices
	for (auto v: verts_new) {
		mesh->vertSet.insert(v);
		mesh->vertMap.insert(make_pair(v->index, v));
	}
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
