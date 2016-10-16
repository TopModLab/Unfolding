#pragma once
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
	};

	static void resetIndex() { uid = 0; }
	static void matchIndexToSize(size_t	size) { uid = size; }

	static hdsid_t assignIndex() { return uid++; }
	void setRefId(hdsid_t id) { refid = (id << 2) + HDS_Common::FROM_VERTEX; }

	HDS_Vertex(const QVector3D &p = QVector3D());
	~HDS_Vertex();

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
	ReebsPointType rtype;
	Float curvature;
	Float colorVal;

	Float morseFunctionVal;
	int32_t sdegree;  // the degree of saddle point
};
