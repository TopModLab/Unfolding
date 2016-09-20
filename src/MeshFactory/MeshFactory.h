#pragma once
#include "HDS/hds_mesh.h"


class MeshFactory
{
public:
	using vert_t = HDS_Vertex;
	using he_t = HDS_HalfEdge;
	using face_t = HDS_Face;
	using mesh_t = HDS_Mesh;

	static HDS_Mesh* create(const mesh_t* ref) {
		return nullptr;
	}

	static void init();
	static void setRefMesh(const mesh_t* ref);
	static void constructHE(vert_t* v, he_t* he);
	static void constructFace(he_t* unlinkedHE, size_t edgeCount, const hdsid_t fid);
	static void constructFace(vector<he_t> hes, const vector<int> indices, const hdsid_t fid);
	static void fillNullFaces(
		vector<he_t> &hes, vector<face_t> &faces,
		unordered_set<hdsid_t> &nullHEs);
	static void generateBridger(he_t* he1, he_t *he2,
		vector<vert_t> &verts, vector<he_t> &hes, vector<face_t> &fs);
	
protected:
	static const mesh_t* refMesh;
	static confMap config;
};

