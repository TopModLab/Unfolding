#pragma once

#include "HDS/hds_mesh.h"
class MeshDFormer
{
	using vert_t = HDS_Vertex;
	using he_t = HDS_HalfEdge;
	using face_t = HDS_Face;
	using mesh_t = HDS_Mesh;
public:
	MeshDFormer();
	~MeshDFormer();

	struct defect
	{
		defect(double ang, const he_t* _he, const he_t* _phe)
			: angle(ang), he(_he), phe(_phe) {}

		double angle, dist_defect;
		const he_t* he;	// current
		const he_t* phe;// previous 
	};

	using dfect_key = pair<hdsid_t, hdsid_t>;
	using defect_map = unordered_map<dfect_key, QSharedPointer<defect>>;

	static mesh_t* generateDForm(const mesh_t* inMesh);
public:
	static defect_map* valuel;
};

