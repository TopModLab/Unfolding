#pragma once

#include "common.h"
#include "hds_bridger.h"
#include "hds_mesh.h"

class MeshExtender;

class MeshHollower
{
public:
	MeshHollower(){}
    static void hollowMesh(HDS_Mesh * mesh, double newFlapSize, int type, double shift);

private:
	static HDS_Face* addFlapFace(int type, HDS_HalfEdge* orginalHE, HDS_HalfEdge* startHE, HDS_Face* cutFace);
	static HDS_Face* createFace(HDS_Vertex* startV, vector<QVector3D> vertPos, HDS_Vertex* endV, HDS_Face* cutFace);
public:
	static double flapSize;

private:
	static vector<HDS_Vertex*> vertices_new;
	static vector<HDS_HalfEdge*> hes_new;
	static HDS_Mesh* thismesh;

};

