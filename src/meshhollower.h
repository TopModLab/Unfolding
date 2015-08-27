#pragma once

#include "common.h"
#include "hds_connector.h"

class HDS_Mesh;
class HDS_Face;
class HDS_HalfEdge;
class HDS_Vertex;

class MeshHollower
{
public:
	MeshHollower(){}
	static void hollowMesh(HDS_Mesh * mesh);


};

