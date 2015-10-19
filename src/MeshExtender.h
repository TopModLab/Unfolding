#pragma once

#include "common.h"

#include "hds_bridger.h"

class HDS_Mesh;
class HDS_Face;
class HDS_HalfEdge;
class HDS_Vertex;


class MeshExtender
{
public:
	MeshExtender(){}
	static bool extendMesh(HDS_Mesh *mesh);
	static vector<HDS_Vertex*> addBridger(HDS_Mesh*, HDS_HalfEdge* he1, HDS_HalfEdge* he2, HDS_Face* cutFace);
	static void scaleFaces(HDS_Mesh* mesh);

protected:

	friend class MeshHollower;
private:
	static bool hasCutEdge;
	static bool hasBridgeEdge;
	static bool isHollow;
};

