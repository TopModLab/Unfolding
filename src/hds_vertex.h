#ifndef HDS_VERTEX_H
#define HDS_VERTEX_H

#include "hds_common.h"

class HDS_Vertex : public HDS_Common
{
private:
	static hdsid_t uid;

public:
	enum EDGE_FLAG : uint16_t
	{
		FLAG_DEFAULT = 0,
		PICKED = 1 << 1
	};

	static void resetIndex() { uid = 0; }
	static hdsid_t assignIndex() { return uid++; }
	void setRefId(hdsid_t id) { refid = (id << 2) + HDS_Common::FROM_VERTEX; }

	HDS_Vertex(const QVector3D &pos = QVector3D(),
		int idx = -1, int refid = 0);
	HDS_Vertex(const HDS_Vertex &other);
	~HDS_Vertex();

	HDS_Vertex operator=(const HDS_Vertex &other);

	void setPicked(bool v) { isPicked = v; }
	uint16_t getFlag() const;

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
	double colorVal;
	bool isPicked;
	uint16_t flag;

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
