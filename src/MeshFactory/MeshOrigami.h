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
	static vector<hdsid_t> nCons;

	static HDS_Mesh* createOrigami(
		const mesh_t* ref_mesh, float scale, float depth);
	static bridgeMap bridgeOrigami(
		const mesh_t* ref_mesh, mesh_t* ori_mesh);
	static void processOrigami(
		const mesh_t* ref_mesh, mesh_t* ori_mesh, bridgeMap bridges);
	static void evalOrigamiPos(
		const mesh_t* ref_mesh, mesh_t* eval_mesh,
		vector<float> &dist, vector<QVector3D>& movingDir);
	static void evalOrigamiRot(
		const mesh_t* ref_mesh, mesh_t* eval_mesh);
};