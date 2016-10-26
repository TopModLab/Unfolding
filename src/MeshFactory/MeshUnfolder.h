#pragma once
#include "Utils/common.h"
#include "HDS/hds_common.h"
#include "MeshFactory/MeshFactory.h"

class MeshUnfolder : public MeshFactory
{
public:
	static HDS_Mesh* unfold(const HDS_Mesh* ref_mesh);
	static HDS_Mesh* unfold(const HDS_Mesh* ref_mesh, vector<hdsid_t> &dirtyEdges);


	static void unfoldSeparateFace(
		const QVector3D &pos, const QVector3D &orient,
		hdsid_t curFid, HDS_Mesh* unfolded_mesh
	);
private:
	static bool hasBadVertex(const HDS_Mesh* ref_mesh);
	static void unfoldFace(
		hdsid_t sharedHEid, hdsid_t curFid,
		HDS_Mesh* unfolded_mesh, const HDS_Mesh* ref_mesh,
		vector<bool> &visitedVerts, vector<hdsid_t> &dirtyEdges);
	static void reset_layout(HDS_Mesh* unfolded_mesh);
};
