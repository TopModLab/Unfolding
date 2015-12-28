#include "meshrimface.h"
#include "MeshExtender.h"

HDS_Mesh* MeshRimFace::thismesh = nullptr;
vector<HDS_Vertex*> MeshRimFace::vertices_new;
vector<HDS_HalfEdge*> MeshRimFace::hes_new;
HDS_Mesh* MeshRimFace::ori_mesh = nullptr;

void MeshRimFace::setOriMesh(HDS_Mesh* mesh)
{
	ori_mesh = mesh;
}

void MeshRimFace::rimMesh3D(HDS_Mesh *mesh)
{
	thismesh = mesh;
	float planeWidthScale = 0.2;
	float planeHeight = 0.2;

	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_Face face_t;

	unordered_set<vert_t*> old_verts = thismesh->verts();
	unordered_map <int, vert_t*> ori_map = ori_mesh->vertMap;


	thismesh->releaseMesh();
	HDS_Vertex::resetIndex();
	HDS_HalfEdge::resetIndex();
	HDS_Face::resetIndex();

	for(auto v: old_verts) {
		face_t* cutFace = new face_t;
		cutFace->index = HDS_Face::assignIndex();
		cutFace->isCutFace = true;


		for (auto he: mesh->incidentEdges(v)) {
			vert_t* vp = he->flip->prev->v;
			vert_t* v1 = he->flip->v;
			vert_t* vn = he->next->next->v;
			//get normals for plane prev and plane next
			QVector3D np = QVector3D::normal(v->pos, v1->pos, vp->pos);
			QVector3D nn = QVector3D::normal(v->pos, v1->pos, vn->pos);
			QVector3D n = np + nn;
			n.normalize();

			//get middle point f the edge
			QVector3D v_mid = (v->pos + v1->pos)/2;

			//get perpendicular plane
			QVector3D v_v1 = v1->pos - v->pos;
			v_v1.normalize();

			QVector3D cross = QVector3D::crossProduct(n, v_v1);

			//get scaling
			float scale_fn = (vn->pos - v1->pos).length()*planeWidthScale/2;
			float scale_fp = (vp->pos - v1->pos).length()*planeWidthScale/2;

			//get mid points on plane
			QVector3D vn_mid = v_mid + scale_fn * cross;
			QVector3D vp_mid = v_mid - scale_fp * cross;


			float innerScale = 1 - planeHeight/2;
			float outerScale = 1 + planeHeight/2;

			vert_t* vn_i = new vert_t((1 - innerScale) * v->pos + innerScale * vn_mid);
			vert_t* vn_o = new vert_t((1 - outerScale) * v->pos + outerScale * vn_mid);
			vert_t* vp_i = new vert_t((1 - innerScale) * v->pos + innerScale * vp_mid);
			vert_t* vp_o = new vert_t((1 - outerScale) * v->pos + outerScale * vp_mid);


			vn_i->refid = v->refid;
			vn_o->refid = v1->refid;
			vp_i->refid = v->refid;
			vp_o->refid = v1->refid;

			vector<vert_t*> vertices;
			vertices.push_back(vp_i);
			vertices.push_back(vn_i);
			vertices.push_back(vn_o);
			vertices.push_back(vp_o);

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

