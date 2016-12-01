#pragma once
#include "MeshFactory/MeshFactory.h"

class MeshNeoWeaver : public MeshFactory
{
public:
	static HDS_Mesh* create(const mesh_t* ref, const confMap &conf);
private:
	static HDS_Mesh* createOctWeaving(
		const mesh_t* ref_mesh, const confMap &conf);
	static HDS_Mesh* createWeaving(
		const mesh_t* ref_mesh, const confMap &conf);
    static HDS_Mesh* createCrossWeaving(
        const mesh_t* ref_mesh, const confMap &conf);
	static HDS_Mesh* createClassicalWeaving(
		const mesh_t* ref_mesh, const confMap &conf
	);
	static HDS_Mesh* createConicalWeaving(
		const mesh_t* ref_mesh, const confMap &conf
	);
};

