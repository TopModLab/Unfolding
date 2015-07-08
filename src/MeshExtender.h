#pragma once

#include "common.h"
#include <QString>

class HDS_Mesh;
class HDS_Face;
class HDS_HalfEdge;
class HDS_Vertex;

class MeshExtender
{
public:
	static bool extendMesh(HDS_Mesh *mesh);

	//config connector

	static void setConnector(std::map<QString, double> config);
private:
	static int shape;
	static double scale;
};

