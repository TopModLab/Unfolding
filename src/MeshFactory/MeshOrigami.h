#pragma once
#include "MeshFactory/MeshFactory.h"

class MeshOrigami : public MeshFactory
{
public:
	static HDS_Mesh* create(const mesh_t* ref, const confMap &conf);
private:
	static HDS_Mesh* createOrigami(
		const mesh_t* ref_mesh, const confMap &conf);
};