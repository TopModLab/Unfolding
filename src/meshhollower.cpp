#include "meshhollower.h"


double MeshHollower::flapSize = 0.2;//[0 >> 1]
double MeshHollower::shiftAmount = 0;//[-1 >> 1]


void MeshHollower::
hollowMesh(HDS_Mesh* mesh, double newFlapSize, int type, double shift)
{
	initiate();
    cur_mesh = mesh;

	flapSize = newFlapSize;//Flap size needed in export function
	shiftAmount = shift;
	//HDS_Bridger::setSamples(3);

	unordered_map <hdsid_t, vert_t*> ori_map = ori_mesh->vertsMap();
	unordered_set<he_t*> old_edges;
	for (auto he: cur_mesh->halfedges()) {
		if (old_edges.find(he->flip) == old_edges.end())
			old_edges.insert(he);
	}

	for (auto f : cur_mesh->faces()) {
		f->setScaleFactor(HDS_Bridger::getScale());
	}


	//set new bridger on each edge
	for (auto he : old_edges)
	{
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

		verts_new.push_back(he1_v1);
		verts_new.push_back(he1_v2);
		verts_new.push_back(he2_v1);
		verts_new.push_back(he2_v2);

		//new edge pair based on new vertex position
		he_t* he1 = HDS_Mesh::insertEdge(he1_v1, he1_v2);
		he_t* he2 = HDS_Mesh::insertEdge(he2_v2, he2_v1);

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
		//cutFace->index = HDS_Face::assignIndex();
		cutFace->isCutFace = true;
		cutFace->he = he1;
		//cutFace->refid = he_f->refid;
		he1->setCutEdge(true);
		he2->f = cutFace;
		he2->flip->f = cutFace;
		he2->setCutEdge(true);

		he1->f = cutFace;
		he1->flip->f = cutFace;
		faces_new.push_back(cutFace);


		vector <QVector3D> vpair = scaleBridgerEdge(he->flip, HDS_Bridger::getScale());

		//add bridger
		addBridger(he1->flip, he2->flip, vpair);

		if (flapSize < 0.01)
		{
			continue;
		}
		else
		{
			he1->setCutEdge(false);
			he2->setCutEdge(false);

			faces_new.push_back(addFlapFace(type, he, he1, cutFace));
			faces_new.push_back(addFlapFace(type, he->flip, he2, cutFace));
		}
	}

	updateNewMesh();
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
	switch(type)
	{
	case 0://one flap
	{
		cur_mesh->setProcessType(HDS_Mesh::HOLLOWED_PROC);

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
		cur_mesh->setProcessType(HDS_Mesh::HOLLOWED_MF_PROC);

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
			HDS_Vertex* hv2_flap = new HDS_Vertex(v2_flap);
			vertices.push_back(hv2_flap);
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
		cout<<"binding..."<<endl;
		cur_mesh->setProcessType(HDS_Mesh::BINDED_PROC);
		auto curHE = originalHE->next;
		do
		{
			HDS_Vertex* newV = new HDS_Vertex;
			newV->pos = he_f->scaleCorner(curHE->flip->v);
			cout<<"new v :"<<newV->pos<<endl;
			newV->refid = curHE->flip->v->refid;
			vertices.push_back(newV);

			curHE = curHE->next;
		} while (curHE != originalHE->prev);
		break;
	}
	default:
		break;
	}

	for (auto v: vertices)
		verts_new.push_back(v);

	vertices.insert(vertices.begin(),startHE->flip->v);
	vertices.push_back(startHE->v);

	HDS_Face* newFace = createFace(vertices, cutFace);
	//newFace->he = startHE;
	newFace->refid = he_f->refid;
	startHE->f = newFace;

	return newFace;
}
