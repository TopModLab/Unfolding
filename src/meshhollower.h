#pragma once

#include "common.h"
#include "MeshExtender.h"
#include <unordered_map>

typedef std::unordered_map<int,double> holePosRefMap;
struct Flap {HDS_HalfEdge* flap_he; QVector3D vp_flap; QVector3D vn_flap;};

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
    static holePosRefMap* refMapPointer;

 private:
    static unordered_map<he_t*,Flap> flapMap;
 };



