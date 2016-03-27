#ifndef MESHRIMFACE_H
#define MESHRIMFACE_H

#include "common.h"
#include "MeshExtender.h"


class MeshRimFace : public MeshExtender
{
public:
	MeshRimFace();
	static void configRimMesh(std::map<QString, bool> config);
	static void rimMesh3D(HDS_Mesh *mesh, float planeWidth, float planeHeight);
	~MeshRimFace();

    static void computePlaneCornerOnEdge(vert_t* v, he_t* he, vector<QVector3D> &vpos);
    static void computePlaneCornerOnFace(vert_t* v, he_t* he, vector<he_t*> &control_edges, vector<QVector3D> &control_points_p, vector<QVector3D> &control_points_n);

private:
    static float planeWidth;
    static float planeHeight;

	static bool onEdge;
	static bool isHalf;
	static bool isQuadratic;
	static bool smoothEdge;
	static bool addConnector;
};

inline void
MeshRimFace::configRimMesh(std::map<QString, bool> config) {
	onEdge = config["onEdge"];
	isHalf = config["isHalf"];
	isQuadratic = config["isQuadratic"];
	smoothEdge = config["smoothEdge"];
	addConnector = config["addConnector"];
}

#endif // MESHRIMFACE_H
