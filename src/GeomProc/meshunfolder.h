#pragma once
#ifndef MESHUNFOLDER_H
#define MESHUNFOLDER_H

#include "Utils/common.h"
#include "GeomUtils/BBox.h"
#include "HDS/hds_common.h"
//#include <QtGui/QVector3D>
#include <QVector3D>
#include <QProgressDialog>

class MeshUnfolder
{
public:
	static MeshUnfolder* getInstance();

	static bool unfold(
		HDS_Mesh* mesh, const HDS_Mesh* ref,
		set<hdsid_t> fixedFaces = set<hdsid_t>());
	static bool unfoldable(const HDS_Mesh* ref_mesh);
	static void reset_layout(HDS_Mesh* unfolded_mesh);

private:
	static void unfoldFace(hdsid_t fprev, hdsid_t fcur,
		HDS_Mesh* unfolded_mesh, const HDS_Mesh* ref_mesh,
		const QVector3D &uvec, const QVector3D &vvec);
	
public:
private:
	MeshUnfolder();
	static MeshUnfolder* instance;
};

#endif // MESHUNFOLDER_H
