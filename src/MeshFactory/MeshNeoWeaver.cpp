#include "MeshNeoWeaver.h"

HDS_Mesh* MeshNeoWeaver::create(
	const mesh_t* ref, const BridgerConfig &config)
{
	return createWeaving(ref, config);
}

HDS_Mesh* MeshNeoWeaver::createWeaving(
	const mesh_t* ref_mesh, const BridgerConfig &config)
{
	if (!ref_mesh) return nullptr;

	auto &ref_verts = ref_mesh->verts();
	auto &ref_hes = ref_mesh->halfedges();
	auto &ref_faces = ref_mesh->faces();
	size_t refHeCount = ref_hes.size();
	size_t refFaceCount = ref_faces.size();

	vector<QVector3D> fNorms(refFaceCount);
	vector<QVector3D> heNorms(refHeCount);

	for (int i = 0; i < refFaceCount; i++)
	{
		fNorms[i] = ref_mesh->faceNormal(i);
	}

	for (int i = 0; i < heNorms.size(); i++)
	{
		auto curHE = &ref_hes[i];
		// TODO: to be finished
	}

	return nullptr;
}
