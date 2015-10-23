#pragma once
#ifndef __MESHCONNECTOR_H__
#define __MESHCONNECTOR_H__
#include "GeodesicComputer.h"
#include "DiscreteGeoComputer.h"

#include "common.h"
#include <QMutex>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QProgressDialog>

#include "meshloader.h"

#include "hds_mesh.h"
#include "hds_face.h"
#include "hds_halfedge.h"
#include "hds_vertex.h"
#include "hds_bridger.h"

#include "Graph.hpp"

class MeshConnector
{
private:
	typedef HDS_Mesh mesh_t;
	typedef HDS_HalfEdge he_t;
	typedef HDS_Face face_t;
	typedef HDS_Vertex vert_t;
public:
	MeshConnector();
	~MeshConnector();
	enum ConnectorType
	{
		SIMPLE_CONNECTOR,
		INSERT_CONNECTOR,
		GEAR_CONNECTOR,
		SAW_CONNECTOR,
		ADVSAW_CONNECTOR,
		HOLLOW_CONNECTOR
	};
	static void exportXML(mesh_t *unfolded_mesh, const char *filename);
private:
	static void exportHollowPiece(mesh_t* unfolded_mesh, const char* filename, int mode = 0);
	static void exportRegularPiece(mesh_t* unfolded_mesh, const char* filename, int mode = 0);
	static void exportRimmedPiece(mesh_t* unfolded_mesh, const char* filename, int mode = 0);
};

#endif