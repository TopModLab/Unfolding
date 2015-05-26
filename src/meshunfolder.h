#ifndef MESHUNFOLDER_H
#define MESHUNFOLDER_H

#include "common.h"

#include <QtGui/QVector3D>
//#include <QVector3D>

class HDS_Mesh;



class MeshUnfolder
{
public:
	MeshUnfolder();

	static bool unfold(HDS_Mesh *mesh, HDS_Mesh *ref, set<int> fixedFaces = set<int>());
	static bool unfoldable(HDS_Mesh *ref_mesh);

private:
	static void unfoldFace(int fprev, int fcur, HDS_Mesh *unfolded_mesh, HDS_Mesh *ref_mesh,
					const QVector3D &uvec, const QVector3D &vvec);
};

#endif // MESHUNFOLDER_H
