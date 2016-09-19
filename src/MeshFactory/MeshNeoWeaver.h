#pragma once
#include "MeshFactory/MeshFactory.h"
class MeshNeoWeaver : public MeshFactory
{
public:
	static HDS_Mesh* create(
		const mesh_t* ref, const BridgerConfig &config);
private:
	static HDS_Mesh* createWeaving(
		const mesh_t* ref_mesh, const BridgerConfig &config);
};

