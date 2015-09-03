#pragma once

#include "common.h"
#include "hds_connector.h"
#include "hds_mesh.h"

class MeshExtender;

class MeshHollower
{
public:
	MeshHollower(){}
	static void hollowMesh(HDS_Mesh * mesh, double flapSize);



};

