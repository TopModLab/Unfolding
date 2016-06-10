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
	static void initBridger(const confMap &conf);
protected:
	//origami_panel parameters
	static double tucked_length;
	static double tucked_smooth;
	static double tucked_angle;
	static double origami_scale;
	static int bridger_type;

	static vector<QVector3D> ctrlPoints_front;
	static vector<QVector3D> ctrlPoints_back;
	//temporary bridger elements array
	static vector<face_t*> bridger_faces;
	static vector<he_t*> bridger_hes;
	static vector<vert_t*> bridger_verts;
		
	//similar function to MeshExtender::scaleFaces(), create new faces without scaling
	static void seperateFaces();
	static void setControlPoints(HDS_HalfEdge* he1, HDS_HalfEdge* he2, double theta, double length, double smooth_length);
	static void setControlPoints2(HDS_HalfEdge* he1, HDS_HalfEdge* he2, double theta, double length, double smooth_length);
	static void addBridger(HDS_HalfEdge* he1, HDS_HalfEdge* he2, double theta, double folding_length, double smooth_length);
	static HDS_Face* bridging(HDS_HalfEdge* he1, HDS_HalfEdge* he2, face_t* cutFace1, face_t* cutFace2);
};



