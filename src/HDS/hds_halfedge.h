#ifndef HDS_EDGE_H
#define HDS_EDGE_H
#include "HDS/hds_common.h"

class HDS_HalfEdge : public HDS_Common
{
private:
	static hdsid_t uid;
public:
	static void resetIndex() { uid = 0; }
	static hdsid_t assignIndex() { return uid++; }
	void setRefId(hdsid_t id) { refid = (id << 2) + HDS_Common::FROM_EDGE; }

	HDS_HalfEdge();
	~HDS_HalfEdge();

	HDS_HalfEdge(const HDS_HalfEdge& other);
	HDS_HalfEdge operator=(const HDS_HalfEdge& other);

	void setPicked(bool v) { isPicked = v; flip->isPicked = v; }
	void setCutEdge(bool v) { isCutEdge = v; flip->isCutEdge = v; }
	void setFlip(HDS_HalfEdge* thef) {flip = thef; thef->flip = this;}
	void setBridgeTwin(HDS_HalfEdge* he) {bridgeTwin = he; he->bridgeTwin = this;}

	uint16_t getFlag() const { return flag; }

	void computeCurvature();
	QVector3D computeNormal();
public:
	HDS_Face *f;
	HDS_Vertex *v;
	HDS_HalfEdge *prev, *next, *flip;

	HDS_HalfEdge *cutTwin;  // pointer to its twin halfedge created in a cut event
	HDS_HalfEdge *bridgeTwin;

	hdsid_t index;
	hdsid_t refid;

	union
	{
		uint16_t flag;
		struct
		{
			bool : 1;
			bool isPicked : 1;
			bool isCutEdge : 1;
			bool isExtended : 1;//From hollower
			bool isNegCurve : 1;
			bool isJoint : 1;//From woven
		};
	};
	
	float angle;
};

#endif // HDS_EDGE_H
