#include "meshrimface.h"
#include "MeshExtender.h"

HDS_Mesh* MeshRimFace::thismesh = nullptr;
vector<HDS_Vertex*> MeshRimFace::vertices_new;
vector<HDS_HalfEdge*> MeshRimFace::hes_new;

void MeshRimFace::rimMesh(HDS_Mesh *mesh)
{
	thismesh = mesh;
	float edgePlaneAngle = 0.2;
	float innerRadius = 0.2;

	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_Face face_t;

	for(auto v: mesh->verts()) {
		face_t* cutFace = new face_t;

		for (auto he: mesh->incidentEdges(v)) {
			vert_t* v0 = he->flip->prev->v;
			vert_t* v1 = he->next->v;
			vert_t* v2 = he->next->next->v;
			QVector3D v10 = (1.0 - edgePlaneAngle/2.0) * v1->pos + edgePlaneAngle/2.0 * v0->pos;
			QVector3D v12 = (1.0 - edgePlaneAngle/2.0) * v1->pos + edgePlaneAngle/2.0 * v2->pos;

			vert_t* v12_inner = new vert_t((1.0 - innerRadius) * v->pos + innerRadius * v12);
			vert_t* v10_inner = new vert_t((1.0 - innerRadius) * v->pos + innerRadius * v10);
			vert_t* v12_outer = new vert_t((1.0 - innerRadius) * v12 + innerRadius * v->pos);
			vert_t* v10_outer = new vert_t((1.0 - innerRadius) * v10 + innerRadius * v->pos);

			v12_inner->refid = v->refid;
			v12_outer->refid = v1->refid;
			v10_inner->refid = v->refid;
			v10_outer->refid = v1->refid;



		}
		//assign cutFace edge and index
	}
}

HDS_Face* MeshRimFace::createFace(vector<HDS_Vertex*> vertices, HDS_Face* cutFace)
{
	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_Face face_t;

	face_t * newFace = new face_t;
	//newFace->index = HDS_Face::assignIndex();

	auto preV = vertices.front();
	for (int i = 1; i < vertices.size() - 1; i++)
	{
		auto& curV = vertices[i];
		vertices_new.push_back(curV);
		he_t* newHE = thismesh->insertEdge(preV, curV);
		newHE->f = newFace;
		newHE->flip->f = cutFace;
		newHE->setCutEdge(true);
		hes_new.push_back(newHE);
		hes_new.push_back(newHE->flip);
		preV = curV;
	}

	//link last edge of the face
	he_t* lastHE = thismesh->insertEdge(preV, vertices.front());
	lastHE->f = newFace;
	lastHE->flip->f = cutFace;
	lastHE->setCutEdge(true);
	hes_new.push_back(lastHE);
	hes_new.push_back(lastHE->flip);

	return newFace;
}

MeshRimFace::MeshRimFace()
{

}


MeshRimFace::~MeshRimFace()
{

}

