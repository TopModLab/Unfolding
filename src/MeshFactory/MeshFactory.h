#pragma once
#include "HDS/hds_mesh.h"

struct BridgerConfig
{
	uint32_t sample;
	BridgerConfig() {}
};

class MeshFactory
{
public:
	using vert_t = HDS_Vertex;
	using he_t = HDS_HalfEdge;
	using face_t = HDS_Face;
	using mesh_t = HDS_Mesh;

	static HDS_Mesh* create(const mesh_t* ref,
		const BridgerConfig &config = BridgerConfig()) {
		return nullptr;
	}

	static void init();
	static void setRefMesh(const mesh_t* ref);
	static void constructEdge(vert_t* v1, vert_t* v2, he_t* he1, he_t* he2);
	static void constructFace(he_t* unlinkedHE, size_t edgeCount, face_t* f);
	static void fillNullFaces(
		vector<he_t> &hes, vector<face_t> &faces,
		unordered_set<hdsid_t> &nullHEs);
	static void generateBridger(he_t* he1, he_t *he2, const BridgerConfig& config,
		vector<vert_t> &verts, vector<he_t> &hes, vector<face_t> &fs);
	
protected:
	static const mesh_t* refMesh;

};

