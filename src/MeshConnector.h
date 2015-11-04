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
#include "mathutils.hpp"

#include "Graph.hpp"

enum class ConnectorConf
{
	// General
	SCALE = 0,
	WIDTH,
	LENGTH,
	// Regular
		// Gear
		GEAR_COUNT,
	// Hollow
	PINHOLESIZE
};

struct EnumClassHash
{
	template <typename T>
	std::size_t operator()(T t) const
	{
		return static_cast<std::size_t>(t);
	}
};

typedef std::unordered_map<ConnectorConf, double, EnumClassHash > confMap;

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
		// Regular type
		SIMPLE_CONNECTOR = 0,
		INSERT_CONNECTOR = 1,
		GEAR_CONNECTOR = 2,
		SAW_CONNECTOR = 3,
		ADVSAW_CONNECTOR = 4,
		// Hollow type
		HOLLOW_CONNECTOR = 0,
		// Rim type
		ARCH_CONNECTOR = 0,
		RING_CONNECTOR = 1
	};
	static void generateConnector(mesh_t *unfolded_mesh);
private:
	static void exportHollowPiece(mesh_t* unfolded_mesh, const char* filename,
		const confMap& conf,
		int cn_t = HOLLOW_CONNECTOR);
	static void exportBindPiece(mesh_t* unfolded_mesh, const char* filename,
		const confMap& conf,
		int cn_t = HOLLOW_CONNECTOR);
	static void exportRegularPiece(mesh_t* unfolded_mesh, const char* filename,
		const confMap& conf,
		int cn_t = SIMPLE_CONNECTOR);
	static void exportRimmedPiece(mesh_t* unfolded_mesh, const char* filename,
		const confMap& conf,
		int cn_t = ARCH_CONNECTOR);

private:
	/*static double pinholesize;
	static double he_scale;*/
};

#endif
