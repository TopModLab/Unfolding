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
<<<<<<< HEAD
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

=======
	//end of splitting edge

	mesh->printInfo();
	mesh->validate();

	int woohoos = 0;

	// need to split each vertex in multiple way like cutting the mesh
#if 0
	cout << "cut vertices = " << cutVerts.size() << endl;
	::system("pause");
#endif

	/// check each cut vertices, merge faces if necessary
	for (auto cv : cutVerts)
	{
		if (cv.second > 1)
		{
			cout << "splitting cut vertex #" << cv.first->index << endl;

			/// get all incident faces
			vector<face_t*> incidentFaces = mesh->incidentFaces(cv.first);
			vector<face_t*> connectorFaces = Utils::filter(incidentFaces, [](face_t* f) {
					return f->isConnector;
			});

			/// get all outgoing halfedges
			vector<he_t*> incidentHEs = mesh->incidentEdges(cv.first);
			/// outgoing cut edges
			vector<he_t*> cutEdges = Utils::filter(incidentHEs, [](he_t* e) {
					return e->f->isConnector;
			});

			/// the degree of the cut vertex
			int nConnectors = connectorFaces.size();//replace k with nConnectors

#if 0
			/// check the relationship between the cut edges and connector faces
			for (int i = 0; i < k; ++i) {
				/// the face of cut edge i is connector face i
				cout << cutEdges[i]->f << " and " << connectorFaces[i] << endl;
			}
			::system("pause");
#endif
			cout << "cut vertex degree = " << nConnectors << endl;
			cout << "cut faces number = " << connectorFaces.size() << endl;
			cout << "incident edges = " << incidentHEs.size() << endl;
			for (auto x : incidentHEs)
			{
				cout << x->index << " @ " << x->f->index
					<< "[" << x->v->index << ", " << x->flip->v->index << "]" << endl;
			}
			cout << "cut edges = " << cutEdges.size() << endl;
			for (auto x : cutEdges)
			{
				cout << x->index << " @ " << x->f->index
					<< "[" << x->v->index << ", " << x->flip->v->index << "]" << endl;
			}
#if 0
			/// verify incident edges
			for (int i = 0; i < incidentHEs.size(); ++i) {
				int j = (i + 1) % incidentHEs.size();
				if (incidentHEs[i]->flip->next != incidentHEs[j]) cout << "failed @" << i << endl;
			}
#endif

			vector<vert_t*> cv_new(nConnectors);
			for (int i = 0; i < nConnectors; ++i)
			{
				cv_new[i] = new vert_t(*cv.first);
				cv_new[i]->he = cutEdges[i];

				//Update vertices
				cutEdges[i]->v = cv_new[i];
				cutEdges[i]->flip->next->v = cv_new[i];
				/// assign a new id to the vertex
				cv_new[i]->index = HDS_Vertex::assignIndex();
>>>>>>> origin/GroupComponet
			}
	}
}
	if (hasCutEdge) {
	vector<vert_t*> verts_new;
	for(auto v: mesh->vertSet) {
		if(v->flapTwin != nullptr){

<<<<<<< HEAD
			/// for all cut-edge edges, create flaps
			//get v->he boundary
			HDS_HalfEdge* he1;
			he1 = v->he->f->index == mesh->finalCutFaceIndex? v->he:v->he->flip;
			//duplicate v->flapTwin->he as new he
=======
			/// divide all incident half edges into nConnectors group
			vector<vector<he_t*>> heGroup(nConnectors);
			int groupIdx = 0, nextGroup = 1;
			he_t *he = cutEdges[groupIdx];
			he_t *curHE = he;
			do {
				cout << "gid = " << groupIdx << endl;
				/// put curHE into current group
				heGroup[groupIdx].push_back(curHE);

				curHE = curHE->flip->next;
				if (curHE == cutEdges[nextGroup]) {
					/// switch to the next group
					++groupIdx;
					nextGroup = (groupIdx + 1) % nConnectors;
				}
>>>>>>> origin/GroupComponet

			he_t* twin_he = v->flapTwin->he;
				he_t* flap_he = new he_t;
				he_t* flap_he_flip = new he_t;

<<<<<<< HEAD
				flap_he->index = HDS_HalfEdge::assignIndex();
				flap_he_flip->index = HDS_HalfEdge::assignIndex();

				flap_he->setFlip(flap_he_flip);
				flap_he->setCutEdge(true);
=======
			/*for (int i = 0; i < nConnectors; ++i) {
				cout << "Group #" << i << " has " << heGroup[i].size() << " half edges." << endl;
				for (auto x : heGroup[i])
					cout << x->index << endl;
			}*/

			/// k-way split the vertex, assign new vertex to the k groups
			/*for (int i = 0; i < nConnectors; ++i) {
				for (auto x : heGroup[i]) {
					x->v = cv_new[i];
					cout << x->flip->v->pos.x() << ", "
						 << x->flip->v->pos.y() << ", "
						 << x->flip->v->pos.z() << endl;
				}

				/// for this group, connect its tail with head
				heGroup[i].front()->prev = heGroup[i].back()->flip;
				heGroup[i].back()->flip->next = heGroup[i].front();
			}*/
>>>>>>> origin/GroupComponet

				//connect edge loop
				flap_he->prev = flap_he_flip;
				flap_he->next = flap_he_flip;
				flap_he_flip->prev = flap_he;
				flap_he_flip->next = flap_he;



				flap_he->f = mesh->faceMap[mesh->finalCutFaceIndex];

<<<<<<< HEAD
				vert_t* flap_vs = new vert_t;
				vert_t* flap_ve = new vert_t;
				flap_vs->pos = twin_he->v->pos;
				flap_ve->pos = twin_he->flip->v->pos;

				flap_vs->index = HDS_Vertex::assignIndex();
				flap_ve->index = HDS_Vertex::assignIndex();
=======
			/// add new edges to connect new vertices
			vector<he_t*> newedges;
			for (int i = 0; i < nConnectors; ++i)
			{
				he_t* newhe = new he_t;
				newhe->v = cv_new[i];
				newhe->index = HDS_HalfEdge::assignIndex();

				he_t* newhe_flip = newhe->flip = new he_t;
				newhe_flip->v = cv_new[(i + (nConnectors - 1)) % nConnectors];
				newhe_flip->index = HDS_HalfEdge::assignIndex();
>>>>>>> origin/GroupComponet

				flap_vs->he = flap_he;
				flap_ve->he = flap_he_flip;

<<<<<<< HEAD
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
=======
				/// fix the flip's face
				newhe_flip->f = holeface;
				newhe->f = connectorFaces[(nConnectors + 1) % nConnectors];

				newedges.push_back(newhe_flip);

				mesh->heSet.insert(newhe);
				mesh->heSet.insert(newhe_flip);
				mesh->heMap.insert(make_pair(newhe->index, newhe));
				mesh->heMap.insert(make_pair(newhe_flip->index, newhe_flip));

				/// connect new edges with original edges
				auto prevEdge = cv_new[i]->he->flip->next->flip;
				prevEdge->next->prev = newhe;
				newhe->next = prevEdge->next;
				prevEdge->next = newhe;
				newhe->prev = prevEdge;

				//newhe->isCutEdge(true);
			}
			/// connect the new edges' flips
			for (int i = 0; i < nConnectors; ++i)
			{
				newedges[i]->next = newedges[(i + (nConnectors - 1)) % nConnectors];
				newedges[i]->prev = newedges[(i + 1) % nConnectors];
			}

#if 0
			/// verify the topology
			for (int i = 0; i < k; ++i) {
				cout << heGroup[i].front()->f << ", " << heGroup[i].back()->flip->f << " <-> " << connectorFaces[i] << endl;
			}
			::system("pause");
#endif
			/// update old faces
			/*for (int i = 0; i < nConnectors; ++i)
			{
				heGroup[i].back()->flip->next = newedges[i];
				newedges[i]->prev = heGroup[i].back()->flip;

				heGroup[(i + 1) % nConnectors].front()->prev = newedges[i];
				newedges[i]->next = heGroup[(i + 1) % nConnectors].front();

				connectorFaces[i]->isCutFace = false;
				connectorFaces[i]->isConnector = true;
			}*/
>>>>>>> origin/GroupComponet

				//bridge v->he and new he
				cout<<"build bridge flap between "<<he1->v->index<<" and "<<flap_he_flip->v->index<<endl;
				mesh->bridging(he1, flap_he_flip);

		}

<<<<<<< HEAD
	}
	//add new vertices
	for (auto v: verts_new) {
		mesh->vertSet.insert(v);
		mesh->vertMap.insert(make_pair(v->index, v));
	}
}

=======
			for (int i = 0; i < k; ++i) {
				auto corners = connectorFaces[i]->corners();
				for (auto &x : corners) cout << x->index << ", ";
				cout << endl;
			}
			system("pause");
#endif

			/// for all cut edges, make them normal edges
//			for (auto ce : cutEdges) {
//				ce->isCutEdge = false;
//				ce->flip->isCutEdge = false;
//			}

			/// add a new cut face
			face_t *nf = holeface;
			nf->index = HDS_Face::assignIndex();
			cout << "new cut face index = " << nf->index << endl;
			nf->isCutFace = true;
			nf->isConnector = false;
			//nf->isFlap = false;
			nf->he = newedges.front();// ->flip;
			mesh->faceSet.insert(nf);
			mesh->faceMap.insert(make_pair(nf->index, nf));

			/// fix the hole if it is adjacent to a cut face
			/*auto nfcorners = holeface->corners();
			for (auto corner : nfcorners) {
				auto cutfaces = Utils::filter(mesh->incidentFaces(corner), [](face_t* f){
						return f->isCutFace;
				});
				if (cutfaces.size() >= 2) {
					cout << "Woohoo!" << cutfaces.size() << endl;
					++woohoos;

					// fix it here, the faces are already in correct order
					for (int i = 0; i < cutfaces.size(); ++i) {

					}
				}
			}*/
>>>>>>> origin/GroupComponet



	/// update the curvature of each vertex
	for (auto &v : mesh->vertSet) {
		v->computeNormal();
		v->computeCurvature();
		//cout << v->index << ": " << (*v) << endl;
	}


	cout<<"extend succeed............."<<endl;
	return true;
}
