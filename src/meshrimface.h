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

	void computePlaneCornerOnEdge(vert_t* v, he_t* he, vector<QVector3D> &vpos);
	void computePlaneCornerOnFace(vert_t* v, he_t* he, vector<QVector3D> &control_points_p, vector<QVector3D> &control_points_n);

	enum RimType {
		EdgeWithBezier = 0,
		EdgeWithCubic = 1,
		FaceWithCubic = 2
	};

private:
	float planeWidth;
	float planeHeight;
};

#endif // MESHRIMFACE_H
