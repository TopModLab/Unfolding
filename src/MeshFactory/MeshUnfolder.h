#pragma once
#include "Utils/common.h"
#include "HDS/hds_common.h"
#include "MeshFactory/MeshFactory.h"

class MeshUnfolder : public MeshFactory
{
public:
	static HDS_Mesh* unfold(const HDS_Mesh* ref_mesh);

private:
	static bool unfoldable(const HDS_Mesh* ref_mesh);
	static void unfoldFace(hdsid_t prevFid, hdsid_t curFid,
		HDS_Mesh* unfolded_mesh, const HDS_Mesh* ref_mesh);
	static void reset_layout(HDS_Mesh* unfolded_mesh);
};
