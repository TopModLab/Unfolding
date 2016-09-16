#pragma once
#include "hds_mesh.h"

struct BridgerConfig
{
	uint32_t sample;
};

class MeshFactory
{
	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_Face face_t;
	using mesh_t = HDS_Mesh;
public:
	static void init();
	static void setRefMesh(const mesh_t* ref);
	static void constructEdge(vert_t* v1, vert_t* v2, he_t* he1, he_t* he2);
	static void constructFace(he_t* unlinkedHE, size_t edgeCount, face_t* f);
	static void fillNullFace(vector<he_t> &hes, vector<face_t> &fs,
		const unordered_set<hdsid_t> &nullHEs);
	static void generateBridger(he_t* he1, he_t *he2, const BridgerConfig& config,
		vector<vert_t> &verts, vector<he_t> &hes, vector<face_t> &fs);
	
protected:
	static const mesh_t* refMesh;

};

