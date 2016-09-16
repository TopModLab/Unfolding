#include "MeshFactory.h"

const HDS_Mesh* MeshFactory::refMesh = nullptr;

void MeshFactory::init()
{
	// TODO
}

void MeshFactory::setRefMesh(const mesh_t* ref)
{
	refMesh = ref;
}

void MeshFactory::constructEdge(vert_t* v1, vert_t* v2, he_t* he1, he_t* he2)
{
	// Link edge between vertices
}

void MeshFactory::constructFace(he_t* unlinkedHE, size_t edgeCount, face_t* f)
{
	// Link edge loops of a face 
	// Restriction: unlinkedHE has to be stored in order
	// TODO
}

void MeshFactory::fillNullFace(vector<he_t> &hes, vector<face_t> &fs,
	const unordered_set<hdsid_t> &nullHEs)
{
	// TODO
}

void MeshFactory::generateBridger(
	he_t* he1, he_t *he2, const BridgerConfig& config,
	vector<vert_t> &verts, vector<he_t> &hes, vector<face_t> &fs)
{
	// TODO
}
