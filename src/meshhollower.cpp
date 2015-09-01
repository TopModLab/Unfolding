#include "meshhollower.h"
#include "MeshExtender.h"


void MeshHollower::hollowMesh(HDS_Mesh* thismesh)
{
	/*ignore cut edges*/

	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_Face face_t;

	//scale down each face to get connector edges
	set<int> oldFaces;
	for (auto &f : thismesh->faceSet) {
		if (f->isCutFace) continue;
		else oldFaces.insert(f->index);
	}
	//get corners

	//set new edges

	//delete old mesh

	//set new connector on each edge

	//seperate each connector by reassigning the edge next and prev
//	for (auto &f : mesh->faces()) {
//		if (!f->isConnector && !f->isCutFace) {
//			//set original faces' edges to be cut edge
//			vector<HDS_HalfEdge*> edges;
//			HDS_HalfEdge* curHE = f->he;
//			cout<<"curhe index:"<<curHE->index<<endl;
//			do {
//				edges.push_back(curHE);
//				curHE->setCutEdge(true);
//				//mesh->linkToCutFace(curHE);
//				curHE = curHE->next;
//			}while (curHE != f->he);
//			cout<<"link edge loop..."<<endl;
//			//link edge loop
//			for (int i = 0; i < edges.size(); i++) {
//				cout<<"link edge index:"<<edges[i]->index<<endl;
//				edges[i]->next = edges[i]->flip->prev;
//				edges[i]->prev = edges[i]->flip->next;
//			}
//			//delete non-connector faces, make the mesh hollow
//			cout<<"deleted face index:"<<f->index<<endl;
//			//mesh->deleteFace(f);
//		}

//	}



	cout<<"mesh set"<<endl;
	//add additional flaps on hollow face
	thismesh->updateSortedFaces();

}
