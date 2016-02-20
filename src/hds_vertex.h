#ifndef HDS_VERTEX_H
#define HDS_VERTEX_H

#include "common.h"
#include "hds_common.h"

#include <QtGui/QVector3D>

class HDS_HalfEdge;

class HDS_Vertex
{
private:
	static hdsid_t uid;

public:
	static void resetIndex() { uid = 0; }
	static hdsid_t assignIndex() { return uid++; }
	void setRefId(hdsid_t id) { refid = (id << 2) + HDS_Common::FROM_VERTEX; }

	HDS_Vertex();
	HDS_Vertex(const QVector3D &pos, int idx = -1, int refid = 0);
	~HDS_Vertex();

	HDS_Vertex(const HDS_Vertex &other);
	HDS_Vertex operator=(const HDS_Vertex &other);

	void setPicked(bool v) { isPicked = v; }
	void computeCurvature();
	void computeNormal();
	vector<HDS_Vertex *> neighbors() const;

	qreal x() { return pos.x(); }
	qreal y() { return pos.y(); }
	qreal z() { return pos.z(); }

	QVector3D pos;
	QVector3D normal;
	HDS_HalfEdge *he;

	int index;
	int refid;
	double curvature;
	bool isPicked;
	double colorVal;

	enum ReebsPointType{
		Minimum,
		Maximum,
		Saddle,
		Regular
	} rtype;
	int sdegree;  // the degree of saddle point
	double morseFunctionVal;
};

#endif // HDS_VERTEX_H
