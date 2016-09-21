#pragma once
#ifndef __MESHCONNECTOR_H__
#define __MESHCONNECTOR_H__
#include "GeomUtils/GeodesicComputer.h"
#include "GeomUtils/DiscreteGeoComputer.h"

#include "Utils/common.h"
#include <QMutex>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QProgressDialog>

#include "GeomProc/meshloader.h"

#include "HDS/hds_mesh.h"
#include "HDS/hds_face.h"
#include "HDS/hds_halfedge.h"
#include "HDS/hds_vertex.h"
#include "GeomUtils/hds_bridger.h"
#include "Utils/mathutils.h"

#include "GeomUtils/Graph.h"
#include "UI/ConnectorPanel.h"

enum class UNIT_TYPE : int
{
	MILIMITER,
	INCH,
	POINT
};

typedef const char cstchar;
cstchar SVG_HEAD[] =
	"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" \
	"<svg width=\"%d\" height=\"%d\" "\
	"xmlns=\"http://www.w3.org/2000/svg\">\n";
cstchar SVG_CIRCLE[] =
	"\t<circle id=\"Circle%d\" cx=\"%f\" cy=\"%f\" r=\"%lf\" " \
	"style=\"stroke:magenta;stroke-width:%lf;fill:none\" />\n";
cstchar SVG_LINE[] =
	"\t<line id=\"Line%d\" x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" " \
	"style=\"fill:none;stroke:%s;stroke-width:%lf\" />\n";
cstchar SVG_DASHARRAY[] =
	"\t<path stroke-dasharray=\"%f, %f\" "\
	"id=\"Dash%d\" d=\"M%f %f L%f %f\" " \
	"style=\"fill:none;stroke:blue;stroke-width:%lf\" />";
cstchar SVG_TEXT[] =
	"\t<text x=\"%lf\" y=\"%lf\" transform=\"rotate(%lf %lf,%lf)\" " \
	"style=\"font-size:%lf;font-family:'%s';"\
	"stroke:blue;stroke-width:%lf;fill:none;" \
	"text-anchor:middle;alignment-baseline:middle;\" >%s</text>\n";
cstchar SVG_LABEL[] =
	"\t<text x=\"%lf\" y=\"%lf\" " \
	"style=\"font-size:10;stroke:blue;stroke-width:%lf;fill:none;" \
	"text-anchor:middle;alignment-baseline:middle;" \
	"font-family:'%s'\" >-</text>\n";
cstchar SVG_ARCH[] =
	"\t<path id=\"Rim%d\" d=\"M %lf %lf " \
	"A %lf %lf, 0, 1, 1, %lf %lf " \
	"L %lf %lf " \
	"A %lf %lf, 0, 1, 0, %lf %lf " \
	"Z\" style=\"fill:none;stroke:blue;stroke-width:%lf\" />\n";
	/*
	"<path d=\"M p1x p1y " \
	"A R R, 0, 1, 0, p2x p2y " \
	"L p3x p3y " \
	"A r r, 0, 1, 1, p4x p4y " \
	"Z\" />");
	*/
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
		rot = RadianToDegree(atan2(dir.y(), dir.x()));
	}
};

// Unit conversion
inline Float MM2Pt(Float mm)
{
	return mm * 72.0 / 25.4;
}
inline Float Inch2Pt(Float inch)
{
	return inch * 72.0;
}
inline Float ConvertToPt(int src_type, Float len)
{
	if (len == 0) return 0;
	switch (src_type)
	{
	case (int)UNIT_TYPE::MILIMITER:
		return MM2Pt(len);
	case (int)UNIT_TYPE::INCH:
		return Inch2Pt(len);
	case (int)UNIT_TYPE::POINT:
		return len;
	default:
		return len;
	}
}
inline Float Pt2MM(Float pt)
{
	return pt * 25.4f / 72.0f;
}
inline Float Pt2Inch(Float pt)
{
	return pt / 72.0f;
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
		// FBWalk type
		ARCH_CONNECTOR = 0,
		RING_CONNECTOR = 1,
		// Woven type
		WOVEN_HOLE_CONNECTOR = 0,
	};
	static bool genConnector(const mesh_t* unfolded_mesh,
		const QString &filename, const confMap &conf);
private:
	static void exportQuadEdgePiece(FILE* fp,
		const mesh_t* unfolded_mesh, const confMap &conf);
	static void exportWingedEdgePiece(FILE* fp,
		const mesh_t* unfolded_mesh, const confMap &conf);
	static void exportGESPiece(FILE* fp,
		const mesh_t* unfolded_mesh, const confMap &conf);
	static void exportGRSPiece(FILE* fp,
		const mesh_t* unfolded_mesh, const confMap &conf);
	static void exportRegularPiece(FILE* fp,
		const mesh_t* unfolded_mesh, const confMap &conf);
	static void exportFBWalkPiece(FILE* fp,
		const mesh_t* unfolded_mesh, const confMap &conf);
	static void exportWovenPiece(FILE* fp,
		const mesh_t* unfolded_mesh, const confMap &conf);

	
	static void writeToSVG(const char* filename);

	static void writeCutLayer(
		FILE* SVG_File, const vector<QVector2D> &cut,
		Float str_wd = 0.01f, int cuttype = 0, int id = 0);
	static void wrtieEtchLayer(
		FILE* SVG_File, const vector<QVector2D> &etch,
		Float str_wd = 0.01f, int seg = 0);

private:
	/*static Float pinholesize;
	static Float he_scale;*/
};

#endif
