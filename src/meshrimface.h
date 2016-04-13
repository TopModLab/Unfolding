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

	static void projectFaceCenter(vert_t* v, he_t* he, QVector3D &vn, QVector3D &vp);
	static void projectEdgeVertices(vert_t* v, he_t* he, QVector3D &up_pos, QVector3D &down_pos);

	static void computePlaneCornerOnEdge(vert_t* v, he_t* he, vector<QVector3D> &vpos);
	static void computePlaneCornerOnFace(vert_t* v, he_t* he, vector<QVector3D> &vmid, vector<QVector3D> &vpos);
	static void computeDiamondCornerOnFace(he_t* he, vector<QVector3D> &vpos);
	static void computeDiamondCornerOnEdge(he_t* he, vector<QVector3D> &vpos, QVector3D &vn_max, QVector3D &vp_max);

protected:
    static float planeWidth;
    static float planeHeight;

	static bool onEdge;
	static bool isRect;
	static bool isHalf;
	static bool isQuadratic;
	static bool smoothEdge;
	static bool addConnector;
	static bool avoidIntersect;
};

inline void
MeshRimFace::configRimMesh(std::map<QString, bool> config) {
	onEdge = config["onEdge"];
	isRect = config["isRect"];
	isHalf = config["isHalf"];
	isQuadratic = config["isQuadratic"];
	smoothEdge = config["smoothEdge"];
	addConnector = config["addConnector"];
	avoidIntersect = config["avoidIntersect"];
}

#endif // MESHRIMFACE_H
