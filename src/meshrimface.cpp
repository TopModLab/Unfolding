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

	unordered_set<vert_t*> old_verts = thismesh->verts();

	thismesh->releaseMesh();
	HDS_Vertex::resetIndex();
	HDS_HalfEdge::resetIndex();
	HDS_Face::resetIndex();

	for(auto v: old_verts) {
		face_t* cutFace = new face_t;
		cutFace->index = HDS_Face::assignIndex();
		cutFace->isCutFace = true;

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

			vector<vert_t*> vertices;
			vertices.push_back(v12_inner);
			vertices.push_back(v10_inner);
			vertices.push_back(v10_outer);
			vertices.push_back(v12_outer);

			HDS_Face* newFace = createFace(vertices, cutFace);
			newFace->refid = he->refid;


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
	vertices.push_back(vertices.front());//form a loop

	auto preV = vertices.front();
	for (int i = 1; i < vertices.size(); i++)
	{
		auto& curV = vertices[i];
		if (i != vertices.size()-1)
			vertices_new.push_back(curV);
		he_t* newHE = HDS_Mesh::insertEdge(preV, curV);
		if(newFace->he == nullptr)
			newFace->he = newHE;
		if(cutFace->he == nullptr)
			cutFace->he = newHE->flip;
		newHE->f = newFace;
		newHE->flip->f = cutFace;
		newHE->setCutEdge(true);
		hes_new.push_back(newHE);
		preV = curV;
	}

	return newFace;
}

MeshRimFace::MeshRimFace()
{

}


MeshRimFace::~MeshRimFace()
{

}

