#ifndef MESHRIMFACE_H
#define MESHRIMFACE_H

#include "common.h"
#include "MeshExtender.h"


class MeshRimFace : public MeshExtender
{
public:
	MeshRimFace();
    static void rimMesh3D(HDS_Mesh *mesh, int rimType, float planeWidth, float planeHeight);
	~MeshRimFace();

    static void computePlaneCornerOnEdge(vert_t* v, he_t* he, vector<QVector3D> &vpos);
    static void computePlaneCornerOnFace(vert_t* v, he_t* he, vector<he_t*> &control_edges, vector<QVector3D> &control_points_p, vector<QVector3D> &control_points_n);

	enum RimType {
		EdgeWithBezier = 0,
		EdgeWithCubic = 1,
		FaceWithCubic = 2
	};

private:
    static float planeWidth;
    static float planeHeight;
};

#endif // MESHRIMFACE_H
