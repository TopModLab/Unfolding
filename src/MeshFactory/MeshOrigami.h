#pragma once
#include "MeshFactory/MeshFactory.h"

class MeshOrigami : public MeshFactory
{
public:
	static HDS_Mesh* create(const mesh_t* ref, const confMap &conf);
private:
	static vector<QVector3D> pos;
	static vector<QVector3D> orient;
	static vector<hdsid_t> heid;

	static HDS_Mesh* createOrigami(
		const mesh_t* ref_mesh, const confMap &conf);
	static void bridgeOrigami(
		const mesh_t* ref_mesh, mesh_t* ori_mesh, float scale, float depth);
	static void processOrigami(
		const mesh_t* ref_mesh, mesh_t* ori_mesh);
	static void evaluateOrigami(
		const mesh_t* ref_mesh, mesh_t* eval_mesh,
		float &dist, vector<QVector3D>& movingDir);

};