#ifndef HDS_FACE_H
#define HDS_FACE_H

#include "common.h"
#include "BBox.h"
#include <QVector3D>

class HDS_HalfEdge;
class HDS_Vertex;

class HDS_Face
{
private:
	static size_t uid;

public:
	static void resetIndex() { uid = 0; }
	static size_t assignIndex() { return uid++; }

	HDS_Face();
	~HDS_Face();

	HDS_Face(const HDS_Face &other);
	HDS_Face operator=(const HDS_Face &other);

	void setPicked(bool v) { isPicked = v; }

	/// Find all directly connected faces
	set<HDS_Face *> connectedFaces();
	/// Find all linked faces in current partiction
	set<HDS_Face *> linkededFaces();

	QVector3D center() const;
	vector<HDS_Vertex*> corners() const;
	QVector3D computeNormal();
	void checkPlanar();
	
	// Check if 
	bool isConnected(const HDS_Face *other);

	void setScaledCorners(double factor);
	vector<QVector3D> getScaledCorners();
	void scaleDown();
	double getScalingFactor(){ return scalingFactor; }
	
	//bounding box related, should only work on cut face
	void update_bbox();

	QVector3D n;
	HDS_HalfEdge *he;

	int index;
	bool isPicked;
	bool isCutFace; //invisible face between cut edges
	bool isHole;
	bool isConnector;

	bool isPlanar;
	vector<HDS_HalfEdge*> internalHEs; //for non-planar faces
	double scalingFactor;
	vector<QVector3D> scaledCorners;

	// Bounding box for the objcet
	BBox3 *bound;
};

#endif // HDS_FACE_H
