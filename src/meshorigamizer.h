#pragma once

#include "common.h"
#include "MeshExtender.h"
#include <unordered_map>


class MeshOrigamizer :public MeshExtender
{
public:
	MeshOrigamizer() {}
	~MeshOrigamizer() {}
	static bool origamiMesh(HDS_Mesh * mesh);
protected:
	//similar function to MeshExtender::scaleFaces(), create new faces without scaling
	static void seperateFaces();
};



