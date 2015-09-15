#include "meshhollower.h"
#include "MeshExtender.h"


void MeshHollower::hollowMesh(HDS_Mesh* thismesh, double flapSize)
{
	/*ignore cut edges*/

	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_Face face_t;

	vector<face_t*> old_faces;
	unordered_set<he_t*> old_edges;
	for (auto he: thismesh->heSet) {
		if (old_edges.find(he->flip) == old_edges.end())
		old_edges.insert(he);
	}

	for (auto f: thismesh->faceSet) {
		f->setScaledCorners(HDS_Connector::getScale());
		old_faces.push_back(f);
	}
	for (auto f: old_faces) {
		thismesh->deleteFace(f);
	}
	//clear old mesh
	thismesh->vertSet.clear();
	thismesh->vertMap.clear();
	for (auto he: old_edges) {
		thismesh->deleteHalfEdge(he);
		thismesh->deleteHalfEdge(he->flip);
	}


	HDS_Vertex::resetIndex();
	HDS_HalfEdge::resetIndex();
	HDS_Face::resetIndex();

	vector<vert_t*> vertices_new;
	vector<he_t*> hes_new;
	//set new connector on each edge
	for (auto he: old_edges) {
		cout<<"connector based on original edge "<<he->index<<endl;
		//get edge vertex, calculate scaled vertex
		vert_t* he_v = he->v;
		vert_t* he_flip_v = he->flip->v;
		face_t* he_f = he->f;
		face_t* he_flip_f = he->flip->f;

		vert_t* he1_v1 = new vert_t(he_f->scaleCorner(he_v));
		vert_t* he1_v2 = new vert_t(he_f->scaleCorner(he_flip_v));
		vert_t* he2_v1 = new vert_t(he_flip_f->scaleCorner(he_v));
		vert_t* he2_v2 = new vert_t(he_flip_f->scaleCorner(he_flip_v));

		vertices_new.push_back(he1_v1);
		vertices_new.push_back(he1_v2);
		vertices_new.push_back(he2_v1);
		vertices_new.push_back(he2_v2);

		//new edge pair based on new vertex position
		he_t* he1 = thismesh->bridging(he1_v1, he1_v2);
		he_t* he2 = thismesh->bridging(he2_v1, he2_v2);
		hes_new.push_back(he1);
		hes_new.push_back(he1->flip);
		hes_new.push_back(he2);
		hes_new.push_back(he2->flip);

		//set edge cut face
		face_t * cutFace = new face_t;
		cutFace->index = HDS_Face::assignIndex();
		cutFace->isCutFace = true;
		cutFace->he = he1;
		he1->flip->f = cutFace;
		he1->setCutEdge(true);
		he2->flip->f = cutFace;
		he2->setCutEdge(true);
		thismesh->addFace(cutFace);

		//add connector
		he1->f = he_f;//pass original face to addConnector function

		vector<vert_t*> verts = MeshExtender::addConnector(thismesh, he1->flip, he2);
		vertices_new.insert( vertices_new.end(), verts.begin(), verts.end() );
		he1->f = cutFace;

		//add additional flaps on hollow face
		if (flapSize > 0.01) {
			QVector3D he1_v0 = he_f->scaleCorner(he->prev->v);
			QVector3D he1_v3 = he_f->scaleCorner(he->next->flip->v);
			QVector3D he2_v0 = he_flip_f->scaleCorner(he->flip->next->flip->v);
			QVector3D he2_v3 = he_flip_f->scaleCorner(he->flip->prev->v);

			vert_t* he_flap_v1 = new vert_t((1.0-flapSize)*he1_v1->pos + flapSize*he1_v0);
			vert_t* he_flap_v2 = new vert_t((1.0-flapSize)*he1_v2->pos + flapSize*he1_v3);
			vert_t* hef_flap_v1 = new vert_t((1.0-flapSize)*he2_v1->pos + flapSize*he2_v0);
			vert_t* hef_flap_v2 = new vert_t((1.0-flapSize)*he2_v2->pos + flapSize*he2_v3);

			//new edge pair based on new vertex position
			he_t* he1_flap = thismesh->bridging(he_flap_v1, he_flap_v2);
			he_t* he2_flap = thismesh->bridging(hef_flap_v1, hef_flap_v2);

			he1_flap->f = cutFace;
			he2_flap->flip->f = cutFace;
			//set he1 and he2 to be non cut edge, flaps to be cut edge
			he1->setCutEdge(false);
			he2->setCutEdge(false);
			he1_flap->setCutEdge(true);
			he2_flap->setCutEdge(true);

			hes_new.push_back(he1_flap);
			hes_new.push_back(he1_flap->flip);
			hes_new.push_back(he2_flap);
			hes_new.push_back(he2_flap->flip);

			vertices_new.push_back(he_flap_v1);
			vertices_new.push_back(he_flap_v2);
			vertices_new.push_back(hef_flap_v1);
			vertices_new.push_back(hef_flap_v2);

			HDS_Face* bridgeFace_he1 = thismesh->bridging(he1_flap->flip, he1, cutFace);
			bridgeFace_he1->index = HDS_Face::assignIndex();
			bridgeFace_he1->isCutFace = false;
			bridgeFace_he1->isConnector = true;
			thismesh->addFace(bridgeFace_he1);
			HDS_Face* bridgeFace_he2 = thismesh->bridging(he2->flip, he2_flap, cutFace);
			bridgeFace_he2->index = HDS_Face::assignIndex();
			bridgeFace_he2->isCutFace = false;
			bridgeFace_he2->isConnector = true;
			thismesh->addFace(bridgeFace_he2);

		}


	}



	//add new vertices and edges
	for (auto v: vertices_new) {
		v->index = HDS_Vertex::assignIndex();
		thismesh->addVertex(v);
	}
	for (auto he: hes_new) {
		he->index = HDS_HalfEdge::assignIndex();
		thismesh->addHalfEdge(he);
	}

	/// update the curvature of each vertex
	for (auto &v : thismesh->vertSet) {
		v->computeNormal();
		v->computeCurvature();
		//cout << v->index << ": " << (*v) << endl;
	}
	cout<<"hollow mesh he size "<<thismesh->halfedges().size()<<endl;
}
