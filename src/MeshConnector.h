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

enum ConnectorConf: int
{
	// General
	SCALE = 0,
	WIDTH,
	LENGTH,
	LABELED,
	LABEL_TEXT,
	// Regular
		// Gear
		GEAR_COUNT,
	// Hollow
	PINHOLE_UNIT,
	PINHOLESIZE,
	PINHOLECOUNT_TYPE,
	SCORE_TYPE,
		DASH_LEN,
		DASH_GAP,
		DASH_UNIT
};
enum class UNIT_TYPE : int
{
	MILIMITER,
	INCH,
	POINT
};
struct EnumClassHash
{
	template <typename T>
	std::size_t operator()(T t) const
	{
		return static_cast<std::size_t>(t);
	}
};

typedef std::unordered_map<ConnectorConf, double, EnumClassHash> confMap;

struct TextLabel
{
	int id;
	QVector2D pos;
	double rot;
	//QString ifo;
	bool operator == (const TextLabel &rhs) { return id == rhs.id; }
	TextLabel(int refid, const QVector2D &p, const QVector2D &dir/*, const QString &ifo*/)
		: id(refid), pos(p)
	{
		rot = Radian2Degree(atan2(dir.y(), dir.x()));
	}
};

// Unit conversion
inline double MM2Pt(double mm)
{
	return mm * 72.0 / 25.4;
}
inline double Inch2Pt(double inch)
{
	return inch * 72.0;
}
inline double ConvertToPt(int src_type, double len)
{
	switch (src_type)
	{
	case UNIT_TYPE::MILIMITER:
		return MM2Pt(len);
	case UNIT_TYPE::INCH:
		return Inch2Pt(len);
	case UNIT_TYPE::POINT:
		return len;
	default:
		return len;
	}
}
inline double Pt2MM(double pt)
{
	return pt * 25.4 / 72.0;
}
inline double Pt2Inch(double pt)
{
	return pt / 72.0;
}
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
	static void exportHollowMFPiece(mesh_t* unfolded_mesh, const char* filename,
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

	static void writeToSVG(const char* filename);

	static void writeCutLayer(FILE* SVG_File, const vector<QVector2D> &cut, int cuttype = 0, int id = 0);
	static void wrtieEtchLayer(FILE* SVG_File, const vector<QVector2D> &etch);

private:
	/*static double pinholesize;
	static double he_scale;*/
};

#endif
