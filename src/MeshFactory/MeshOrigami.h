#pragma once
#include "MeshFactory/MeshFactory.h"

class MeshOrigami : public MeshFactory
{
	typedef unordered_map<hdsid_t, pair<hdsid_t, int>> bridgeMap;
public:
	static HDS_Mesh* create(const mesh_t* ref, const confMap &conf);
private:
	static vector<QVector3D> pos;
	static vector<QVector3D> orient;
	static vector<hdsid_t> heid;

	static HDS_Mesh* createOrigami(
		const mesh_t* ref_mesh);
	static bridgeMap bridgeOrigami(
		const mesh_t* ref_mesh, mesh_t* ori_mesh, float scale, float depth);
	static void processOrigami(
		const mesh_t* ref_mesh, mesh_t* ori_mesh, bridgeMap bridges);
	static void evaluateOrigami(
		const mesh_t* ref_mesh, mesh_t* eval_mesh,
		float &dist, vector<QVector3D>& movingDir);

};