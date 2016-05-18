#pragma once

#include "utils.hpp"

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

	static void initiate();
	static void updateNewMesh();
	static void scaleFaces();

	//quadratic bezier curve based bridger
	static void addBridger(HDS_HalfEdge* he1, HDS_HalfEdge* he2, vector<QVector3D> vpos);
	//cubic bezier curve based bridger


    static vector <QVector3D> scaleBridgerEdge(he_t* he);


	static HDS_Face* createFace(vector<HDS_Vertex*> vertices, face_t* cutFace = nullptr);
	static HDS_Face* duplicateFace(face_t* face, face_t* cutFace);
	static HDS_HalfEdge* duplicateEdge(he_t* edge);

	static void assignCutFace(face_t* face, face_t* cutFace);

	static HDS_Mesh* cur_mesh;
	static HDS_Mesh* ori_mesh;

	static vector<he_t*> hes_new;
	static vector<vert_t*> verts_new;
	static vector<face_t*> faces_new;

private:
	static void createBridger(HDS_Bridger* Bridger);

};

