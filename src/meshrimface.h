#ifndef MESHRIMFACE_H
#define MESHRIMFACE_H

#include "common.h"
#include "MeshExtender.h"

class MeshExtender;

class MeshRimFace : public MeshExtender
{
public:
	MeshRimFace();
	static void rimMesh3D(HDS_Mesh *mesh, float planeWidth, float planeHeight);
	~MeshRimFace();

};

#endif // MESHRIMFACE_H
