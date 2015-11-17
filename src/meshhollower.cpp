#include "meshhollower.h"
#include "MeshExtender.h"


double MeshHollower::flapSize = 0.2;//[0 >> 1]
double MeshHollower::shiftAmount = 0;//[-1 >> 1]

HDS_Mesh* MeshHollower::thismesh = nullptr;
vector<HDS_Vertex*> MeshHollower::vertices_new;
vector<HDS_HalfEdge*> MeshHollower::hes_new;

void MeshHollower::hollowMesh(HDS_Mesh* mesh, double newFlapSize, int type, double shift)
{
	/*ignore cut edges*/
	flapSize = newFlapSize;//Flap size needed in export function
	thismesh = mesh;
	shiftAmount = shift;

	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_Face face_t;

	unordered_set<he_t*> old_edges;
	for (auto he: thismesh->heSet) {
		if (old_edges.find(he->flip) == old_edges.end())
			old_edges.insert(he);
	}

	for (auto f : thismesh->faceSet) {
		f->setScaleFactor(HDS_Bridger::getScale());
	}

//	for (auto f : old_faces) {
//		thismesh->deleteFace(f);
//	}


	thismesh->heSet.clear();
	thismesh->vertSet.clear();
	thismesh->faceSet.clear();
	thismesh->heMap.clear();
	thismesh->vertMap.clear();
	thismesh->faceMap.clear();

	HDS_Vertex::resetIndex();
	HDS_HalfEdge::resetIndex();
	HDS_Face::resetIndex();


	//set new bridger on each edge
	for (auto he : old_edges)
	{
		cout << "bridger based on original edge " << he->index << endl;
		//get edge vertex, calculate scaled vertex
		vert_t* he_v = he->v;
		vert_t* he_flip_v = he->flip->v;
		face_t* he_f = he->f;
		face_t* he_flip_f = he->flip->f;

		vert_t* he1_v1 = new vert_t(he_f->scaleCorner(he_v));
		vert_t* he1_v2 = new vert_t(he_f->scaleCorner(he_flip_v));
		vert_t* he2_v1 = new vert_t(he_flip_f->scaleCorner(he_v));
		vert_t* he2_v2 = new vert_t(he_flip_f->scaleCorner(he_flip_v));

		// Assign id and refid
		he1_v1->refid = he2_v1->refid = he_v->refid;
		//	= HDS_Common::assignRef_ID(he_v->index, HDS_Common::FROM_VERTEX);
		he1_v2->refid = he2_v2->refid = he_flip_v->refid;
		//	= HDS_Common::assignRef_ID(he_flip_v->index, HDS_Common::FROM_VERTEX);

		vertices_new.push_back(he1_v1);
		vertices_new.push_back(he1_v2);
		vertices_new.push_back(he2_v1);
		vertices_new.push_back(he2_v2);

		//new edge pair based on new vertex position
		he_t* he1 = thismesh->insertEdge(he1_v1, he1_v2);
		he_t* he2 = thismesh->insertEdge(he2_v2, he2_v1);

		// Assign refid for edges
		// *	      *
		// ||		/ || \
		// ||	->	| || |
		// ||		\ || /
		// *		  *
		he1->refid = he1->flip->refid = he->refid;
		//	= HDS_Common::assignRef_ID(he->index, HDS_Common::FROM_EDGE);
		he2->refid = he2->flip->refid = he->flip->refid;
		//	= HDS_Common::assignRef_ID(he->flip->index, HDS_Common::FROM_EDGE);

		hes_new.push_back(he1);
		hes_new.push_back(he2);

		//set edge cut face
		face_t * cutFace = new face_t;
		cutFace->index = HDS_Face::assignIndex();
		cutFace->isCutFace = true;
		cutFace->he = he1;
		//cutFace->refid = he_f->refid;
		he1->setCutEdge(true);
		he2->f = cutFace;
		he2->setCutEdge(true);
		thismesh->addFace(cutFace);

		//add bridger
		he1->f = he_f;//pass original face to addBridger function

		vector<vert_t*> verts = MeshExtender::addBridger(thismesh, he1->flip, he2->flip, cutFace);
		vertices_new.insert( vertices_new.end(), verts.begin(), verts.end() );
		he1->f = cutFace;

		if (flapSize < 0.01)
		{
			continue;
		}
		else
		{
			he1->setCutEdge(false);
			he2->setCutEdge(false);

			thismesh->addFace(addFlapFace(type, he, he1, cutFace));
			thismesh->addFace(addFlapFace(type, he->flip, he2, cutFace));
		}
	}

	//add new vertices and edges
	for (auto v: vertices_new) {
		v->index = HDS_Vertex::assignIndex();
		thismesh->addVertex(v);
	}
	for (auto he: hes_new) {
		he->index = HDS_HalfEdge::assignIndex();
		he->flip->index = HDS_HalfEdge::assignIndex();
		thismesh->addHalfEdge(he);
		thismesh->addHalfEdge(he->flip);

	}
	/// update the curvature of each vertex
	for (auto &v : thismesh->vertSet) {
		v->computeNormal();
		v->computeCurvature();
		//cout << v->index << ": " << (*v) << endl;
	}


	// Set mark for hollowed mesh
	thismesh->updatePieceSet();
}

HDS_Face* MeshHollower::addFlapFace(int type,
	HDS_HalfEdge* originalHE, HDS_HalfEdge* startHE, HDS_Face* cutFace )
{
	vector<QVector3D> vertPos;
	HDS_Face* he_f = originalHE->f;

	QVector3D v0 = he_f->scaleCorner(originalHE->prev->v);
	QVector3D v3 = he_f->scaleCorner(originalHE->next->flip->v);
	QVector3D v1 = startHE->v->pos;
	QVector3D v2 = startHE->flip->v->pos;

	//calculate the center of flap face(quad)
	QVector3D center = he_f->center();

	QVector3D v1_flap, v2_flap;
	if (type == 0){
		v1_flap = (1.0 - flapSize) * v1 + flapSize * v0;
		v2_flap = (1.0 - flapSize) * v2 + flapSize * v3;
	}else {
		//get parallel flaps
		//linear area incresement using sqrt(flapSize)
		v1_flap = (1.0 - flapSize) * v1 + flapSize * center;
		v2_flap = (1.0 - flapSize) * v2 + flapSize * center;
	}


	vector<HDS_Vertex*> vertices;
	vertices.push_back(startHE->flip->v);
	switch(type)
	{
	case 0://one flap
	{
		thismesh->processType = HDS_Mesh::HOLLOWED_PROC;

		HDS_Vertex* hv1_flap = new HDS_Vertex(v1_flap);
		HDS_Vertex* hv2_flap = new HDS_Vertex(v2_flap);

		hv1_flap->refid = startHE->v->refid;
		hv2_flap->refid = startHE->flip->v->refid;
		vertices.push_back(hv2_flap);
		vertices.push_back(hv1_flap);

		break;
	}
	case 1://mult flap
	{
		thismesh->processType = HDS_Mesh::HOLLOWED_MF_PROC;

		QVector3D v0_flap = (1.0 - flapSize) * v0 + flapSize * center;
		QVector3D v3_flap = (1.0 - flapSize) * v3 + flapSize * center;


		float right_scale = (shiftAmount + 1)/2;
		float left_scale = 1 - right_scale;

		if(shiftAmount != -1)
		{
			//right flap

			HDS_Vertex* hv3_scaled
				= new HDS_Vertex((1.0 - right_scale) * v2 + right_scale * v3);
			HDS_Vertex* hv3_flap_scaled
				= new HDS_Vertex((1.0 - right_scale) * v2_flap + right_scale * v3_flap);
			HDS_Vertex* hv2_flap = new HDS_Vertex(v2_flap);

			vertices.push_back(hv3_scaled);
			vertices.push_back(hv3_flap_scaled);
			vertices.push_back(hv2_flap);
		}
		else
		{
			vertPos.push_back(v2_flap);
		}

		if(shiftAmount != 1)
		{
			//left flap

			HDS_Vertex* hv1_flap
				= new HDS_Vertex(v1_flap);
			HDS_Vertex* hv0_flap_scaled
				= new HDS_Vertex((1.0 - left_scale) * v1_flap + left_scale * v0_flap);
			HDS_Vertex* hv0_scaled
				= new HDS_Vertex((1.0 - left_scale) * v1 + left_scale * v0);

			vertices.push_back(hv1_flap);
			vertices.push_back(hv0_flap_scaled);
			vertices.push_back(hv0_scaled);
		}
		else
		{
			HDS_Vertex* hv1_flap = new HDS_Vertex(v1_flap);

			vertices.push_back(hv1_flap);
		}

		break;
	}
	case 2://bind
	{
		thismesh->processType = HDS_Mesh::BINDED_PROC;
		auto curHE = originalHE->next;
		do
		{
			HDS_Vertex* newV = new HDS_Vertex;
			newV->pos = he_f->scaleCorner(curHE->flip->v);
			newV->refid = curHE->flip->v->refid;
			vertices.push_back(newV);

			curHE = curHE->next;
		} while (curHE != originalHE->prev);
		break;
	}
	default:
		break;
	}
	vertices.push_back(startHE->v);

	HDS_Face* newFace = createFace(vertices, cutFace);
	newFace->he = startHE;
	newFace->refid = he_f->refid;
	startHE->f = newFace;

	return newFace;
}

//vertPos: [new verts..]


HDS_Face* MeshHollower::createFace(vector<HDS_Vertex*> vertices, HDS_Face* cutFace)
{
	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_Face face_t;

	face_t * newFace = new face_t;
	newFace->index = HDS_Face::assignIndex();
	//newFace->refid = cutFace->refid;

	auto preV = vertices.front();
	for (int i = 1; i < vertices.size(); i++)
	{
		auto& curV = vertices[i];
		if(i != vertices.size() - 1)
			vertices_new.push_back(curV);
		he_t* newHE = thismesh->insertEdge(preV, curV);
		newHE->f = newFace;
		newHE->flip->f = cutFace;
		newHE->setCutEdge(true);
		hes_new.push_back(newHE);
		preV = curV;
	}



	return newFace;
}
