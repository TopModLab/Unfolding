#ifndef HDS_FACE_H
#define HDS_FACE_H

#include "common.h"
#include "hds_common.h"
#include "BBox.h"
#include <QVector3D>

class HDS_HalfEdge;
class HDS_Vertex;

class HDS_Face
{
private:
	static hdsid_t uid;

public:
	static void resetIndex() { uid = 0; }
	static hdsid_t assignIndex() { return uid++; }
	void setRefId(hdsid_t id) { refid = (id << 2) + HDS_Common::FROM_FACE; }

	HDS_Face();
	~HDS_Face();

	HDS_Face(const HDS_Face &other);
	HDS_Face operator=(const HDS_Face &other);

	void setPicked(bool v) { isPicked = v; }

	// Find all directly connected faces
	set<HDS_Face *> connectedFaces();
	// Find all linked faces in current partiction
	set<HDS_Face *> linkedFaces();

	QVector3D center() const;
	vector<HDS_Vertex*> corners() const;
	QVector3D computeNormal();
	void checkPlanar();
	static void LineLineIntersect(QVector3D p1,QVector3D p2,QVector3D p3,QVector3D p4,QVector3D *pa);

	// Check if 
	bool isConnected(const HDS_Face *other);

	void setScaleFactor(double factor);
	QVector3D scaleCorner(HDS_Vertex* v);
	vector<QVector3D> getScaledCorners();
	void scaleDown();
	double getScalingFactor(){ return scalingFactor; }

	uint16_t getFlag() const;
	//bounding box related, should only work on cut face
	//void update_bbox();
public:
	enum FACE_FLAG : uint16_t
	{
		DEFAULT		= 0,
		PICKED		= 1 << 1,
		CUTFACE		= 1 << 2,
		HOLE		= 1 << 3,
		BRIDGER		= 1 << 4,
		PLANAR		= 1 << 5
	};
	QVector3D n;
	HDS_HalfEdge *he;

	int index;
	int refid;

	// Flags
	bool isPicked;
	bool isCutFace; //invisible face between cut edges
	bool isHole;
	bool isBridger;
	bool isPlanar;

private:
	vector<HDS_HalfEdge*> internalHEs; //for non-planar faces
	double scalingFactor;
	vector<QVector3D> scaledCorners;

	// Bounding box for the objcet
	//BBox3 *bound;
};

#endif // HDS_FACE_H
