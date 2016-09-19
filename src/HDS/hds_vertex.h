#ifndef HDS_VERTEX_H
#define HDS_VERTEX_H

#include "HDS/hds_common.h"

class HDS_Vertex : public HDS_Common
{
private:
	static hdsid_t uid;

public:
	enum ReebsPointType : uint16_t
	{
		Minimum,
		Maximum,
		Saddle,
		Regular
	} rtype;

	static void resetIndex() { uid = 0; }
	static hdsid_t assignIndex() { return uid++; }
	void setRefId(hdsid_t id) { refid = (id << 2) + HDS_Common::FROM_VERTEX; }

	HDS_Vertex(const QVector3D &p = QVector3D());
	//HDS_Vertex(const HDS_Vertex &other);
	~HDS_Vertex() {}

	//HDS_Vertex operator=(const HDS_Vertex &other);

	// Get the connected half-edge id
	// Explicit pointer access is handled by HDS_Mesh
	hdsid_t heID() const { return heid; }

	void setPicked(bool v) { isPicked = v; }
	uint16_t getFlag() const { return flag; }

	void computeCurvature();
	void computeNormal();
	vector<HDS_Vertex *> neighbors() const;

	qreal x() { return pos.x(); }
	qreal y() { return pos.y(); }
	qreal z() { return pos.z(); }

public:
	hdsid_t index;
	hdsid_t refid;

	//HDS_HalfEdge *he;
	hdsid_t heid;

	union
	{
		uint16_t flag;
		struct
		{
			bool : 1;
			bool isPicked : 1;
		};
	};

	QVector3D pos;
	QVector3D normal;
	double curvature;
	double colorVal;

	int sdegree;  // the degree of saddle point
	double morseFunctionVal;
};

#endif // HDS_VERTEX_H
