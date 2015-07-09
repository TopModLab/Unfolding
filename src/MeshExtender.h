#pragma once

#include "common.h"
#include <QString>

#include "hds_connector.h"

class HDS_Mesh;
class HDS_Face;
class HDS_HalfEdge;
class HDS_Vertex;

class MeshExtender
{
public:
	static bool extendMesh(HDS_Mesh *mesh);


};

