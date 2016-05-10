#include "MeshDFormer.h"

MeshDFormer::defect_map* MeshDFormer::valuel = nullptr;

MeshDFormer::MeshDFormer()
{
}


MeshDFormer::~MeshDFormer()
{
}

MeshDFormer::mesh_t* MeshDFormer::generateDForm(const mesh_t* inMesh)
{
	auto isBadVertex = [](face_t* f) -> bool {
		vector<double> sums;
		double sum = 0;
		auto he = f->he;
		auto curHE = he->flip->next;
		bool hasCutFace = false;
		do {
			if (!curHE->f->isCutFace) {
				QVector3D v1 = he->flip->v->pos - he->v->pos;
				QVector3D v2 = curHE->flip->v->pos - curHE->v->pos;
				double nv1pnv2 = v1.length() * v2.length();
				double inv_nv1pnv2 = 1.0 / nv1pnv2;
				double cosVal = QVector3D::dotProduct(v1, v2) * inv_nv1pnv2;
				double angle = acos(cosVal);
				sum += angle;
			}
			else hasCutFace = true;

			he = curHE;
			curHE = he->flip->next;
		} while (he != v->he);
	};
}
