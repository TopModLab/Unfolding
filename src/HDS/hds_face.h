#ifndef HDS_FACE_H
#define HDS_FACE_H

#include "HDS/hds_common.h"
#include "GeomUtils/BBox.h"



class HDS_Face : public HDS_Common
{
private:
	static hdsid_t uid;

public:
	static void resetIndex() { uid = 0; }
	static hdsid_t assignIndex() { return uid++; }
	void setRefId(hdsid_t id) { refid = (id << 2) + HDS_Common::FROM_FACE; }

	HDS_Face();
	~HDS_Face() {}

	//HDS_Face(const HDS_Face &other);
	//HDS_Face operator=(const HDS_Face &other);

	// Get the connected half-edge id
	// Explicit pointer access is handled by HDS_Mesh
	hdsid_t heID() const { return heid; }

	void setPicked(bool v) { isPicked = v; }

	// Find all directly connected faces
	set<HDS_Face *> connectedFaces() const;
	// Find all linked faces in current partition
	set<HDS_Face *> linkedFaces();

	QVector3D center() const;
	vector<HDS_Vertex*> corners() const;
	QVector3D computeNormal();
	QVector3D computeNormal() const;
	QVector3D updateNormal();
	void checkPlanar();

	// Check if 
	bool isConnected(const HDS_Face *other);

	void setScaleFactor(Float factor);
	QVector3D scaleCorner(HDS_Vertex* v);
	vector<QVector3D> getScaledCorners();
	void scaleDown();
	Float getScalingFactor() { return scalingFactor; }

	uint16_t getFlag() const { return flag; }
	//bounding box related, should only work on cut face
	//void update_bbox();
public:
	hdsid_t index;
	hdsid_t refid;

	//HDS_HalfEdge *he;
	hdsid_t heid;

	// Flags
	union
	{
		uint16_t flag;
		struct
		{
			bool : 1;
			bool isPicked : 1;
			bool isCutFace : 1; //invisible face between cut edges
			//bool isHole : 1;
			bool isBridger : 1;
			bool isNonPlanar : 1;
			bool isJoint : 1; //woven joint face
		};
	};

	QVector3D n;
	
private:
	// TODO: remove from class member
	Float scalingFactor;
	vector<QVector3D> scaledCorners;

	// Bounding box for the objcet
	//BBox3 *bound;
};

#endif // HDS_FACE_H
