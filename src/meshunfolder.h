#pragma once
#ifndef MESHUNFOLDER_H
#define MESHUNFOLDER_H

#include "common.h"
#include "BBox.h"
//#include <QtGui/QVector3D>
#include <QVector3D>
#include <QProgressDialog>

class HDS_Mesh;
class HDS_Face;
//static set<set<HDS_Face*>*> pieces;

class MeshUnfolder
{
public:
	static MeshUnfolder* getInstance();

	static bool unfold(
		HDS_Mesh* mesh, const HDS_Mesh* ref,
		set<int> fixedFaces = set<int>());
	static bool unfoldable(const HDS_Mesh* ref_mesh);
	static void reset_layout(HDS_Mesh* unfolded_mesh);

private:
	static void unfoldFace(int fprev, int fcur,
		HDS_Mesh* unfolded_mesh, const HDS_Mesh* ref_mesh,
		const QVector3D &uvec, const QVector3D &vvec);
	
public:
private:
	MeshUnfolder();
	static MeshUnfolder* instance;
};

#endif // MESHUNFOLDER_H
