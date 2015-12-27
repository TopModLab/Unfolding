#pragma once

#include "common.h"

#include "hds_bridger.h"

class HDS_Mesh;
class HDS_Face;
class HDS_HalfEdge;
class HDS_Vertex;


class MeshExtender
{
public:
	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_Face face_t;

	MeshExtender(){}
	static bool extendMesh(HDS_Mesh *mesh);

	static void setOriMesh(HDS_Mesh* hds_mesh);
protected:

	friend class MeshHollower;

	static void deleteOldMesh();
	static void updateNewMesh();

	static void addBridger(HDS_HalfEdge* he1, HDS_HalfEdge* he2, HDS_Vertex* v1, HDS_Vertex* v2);
	static void scaleFaces();
	static HDS_Face* createFace(vector<HDS_Vertex*> vertices);


	static HDS_Mesh* cur_mesh;
	static HDS_Mesh* ori_mesh;

	static vector<he_t*> hes_new;
	static vector<vert_t*> verts_new;
	static vector<face_t*> faces_new;

private:
	static bool isHollow;
};

