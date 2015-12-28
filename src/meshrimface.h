#ifndef MESHRIMFACE_H
#define MESHRIMFACE_H

#include "common.h"
#include "hds_bridger.h"
#include "hds_mesh.h"

class MeshExtender;

class MeshRimFace
{
public:
	MeshRimFace();
	static void rimMesh3D(HDS_Mesh *mesh);
	static HDS_Face* createFace(vector<HDS_Vertex*> vertices, HDS_Face* cutFace);
	static void setOriMesh(HDS_Mesh* mesh);
	~MeshRimFace();

private:
	static HDS_Mesh* ori_mesh;
	static HDS_Mesh* thismesh;
	static vector<HDS_Vertex*> vertices_new;
	static vector<HDS_HalfEdge*> hes_new;
};

#endif // MESHRIMFACE_H
