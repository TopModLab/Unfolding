#pragma once
#include "MeshFactory/MeshFactory.h"

class MeshOrigami : public MeshFactory
{
public:
	static HDS_Mesh* create(const mesh_t* ref, const confMap &conf);
//private:
	static HDS_Mesh* createOrigami(
		const mesh_t* ref_mesh, const confMap &conf);
	static HDS_Mesh* processOrigami(
		const mesh_t* ref_mesh, const mesh_t* ori_mesh,
		vector<QVector3D> pos, vector<QVector3D> rot);
	static void evaluateOrigami(
		const mesh_t* ref_mesh, mesh_t* eval_mesh,
		float &dist, vector<QVector3D> &pos);

};