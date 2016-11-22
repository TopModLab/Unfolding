#pragma once
#include "HDS/HDS_Mesh.h"
#include "Utils/utils.h"

class MeshFactory
{
public:
	using vert_t = HDS_Vertex;
	using he_t = HDS_HalfEdge;
	using face_t = HDS_Face;
	using mesh_t = HDS_Mesh;

	static HDS_Mesh* create(const mesh_t* ref, const confMap &conf) {
		return nullptr;
	}

	static void init();
	static void constructHE(vert_t* v, he_t* he);
	static void constructHE(vert_t* v, he_t* he, size_t edgeCount);

	static void constructFace(
		he_t* unlinkedHE, size_t edgeCount,
		face_t* fid);
	static void constructFace(
		vector<he_t> hes, const vector<int> indices, 
		face_t* fid);
	static void fillNullFaces(
		vector<he_t> &hes, vector<face_t> &faces,
		unordered_set<hdsid_t> &nullHEs);
	static void generateBridge(
		hdsid_t he1, hdsid_t he2, 
		mesh_t* mesh,
		vector<QVector3D> &vpos1 = vector<QVector3D>(),
		vector<QVector3D> &vpos2 = vector<QVector3D>()
	);
};

