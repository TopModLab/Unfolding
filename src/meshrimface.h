#ifndef MESHRIMFACE_H
#define MESHRIMFACE_H

#include "MeshExtender.h"


class MeshRimFace : public MeshExtender
{
public:
	MeshRimFace();
	static void configRimMesh(std::map<QString, float> config);
	static void rimMeshV(HDS_Mesh *mesh);//around vertex
	static void rimMeshF(HDS_Mesh *mesh);//around face
	~MeshRimFace();

	static void projectFaceCenter(vert_t* v, he_t* he, QVector3D &vn, QVector3D &vp);
	static void projectEdgeVertices(vert_t* v, he_t* he, QVector3D &up_pos, QVector3D &down_pos);

	static void computePlaneCornerOnEdge(vert_t* v, he_t* he, vector<QVector3D> &vpos);
	static void computePlaneCornerOnFace(vert_t* v, he_t* he, vector<QVector3D> &vmid, vector<QVector3D> &vpos);
	static void computeDiamondCornerOnFace(he_t* he, vector<QVector3D> &vpos);
	static void computeDiamondCornerOnEdge(he_t* he, vector<QVector3D> &vpos, QVector3D &vn_max, QVector3D &vp_max);

protected:
	static int shapeType;
	static float flapWidth;
    static float planeWidth;
    static float planeHeight;
	static float pivotPosition;

private:
	static bool onEdge;
	static bool isHalf;
	static bool isQuadratic;
	static bool smoothEdge;
	static bool addConnector;
	static bool avoidIntersect;
};

inline void
MeshRimFace::configRimMesh(std::map<QString, float> config) {
	planeWidth = config["roundness"];
	planeHeight = config["thicknessOfBridger"];

	if ((int)config["center"] == 0)  {
	shapeType = (int)config["type"];

	onEdge = (int)config["onEdge"];
	isHalf = (int)config["isHalf"];
	isQuadratic = (int)config["isQuadratic"];
	smoothEdge = (int)config["smoothEdge"];
	addConnector = (int)config["addConnector"];
	avoidIntersect = (int)config["avoidIntersect"];
	if (isQuadratic)
		planeWidth = 1- planeWidth;
	}else {
		flapWidth = config["thicknessOfCon"];
		planeWidth = 1- planeWidth;
	}


}

#endif // MESHRIMFACE_H
