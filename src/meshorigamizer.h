#pragma once

#include "common.h"
#include "MeshExtender.h"
#include <unordered_map>


class MeshOrigamizer :public MeshExtender
{

public:
	MeshOrigamizer() {}
	~MeshOrigamizer() {}
	static bool origamiMesh(HDS_Mesh * mesh);
protected:
	//similar function to MeshExtender::scaleFaces(), create new faces without scaling
	static void seperateFaces();
	static void addBridger(HDS_HalfEdge* he1, HDS_HalfEdge* he2, double theta, double length);
	static pair<vector<he_t*>, HDS_Face*> bridging(HDS_HalfEdge* he1, HDS_HalfEdge* he2, face_t* cutFace1, face_t* cutFace2);
};



