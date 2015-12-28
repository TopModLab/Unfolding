#pragma once

#include "common.h"
#include "MeshExtender.h"

class MeshHollower :public MeshExtender
{
public:
	MeshHollower(){}
    static void hollowMesh(HDS_Mesh * mesh, double newFlapSize, int type, double shift);
private:
	static HDS_Face* addFlapFace(int type, HDS_HalfEdge* orginalHE, HDS_HalfEdge* startHE, HDS_Face* cutFace);
public:
	static double flapSize;
	static double shiftAmount;

};

