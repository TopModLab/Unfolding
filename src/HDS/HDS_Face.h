#pragma once
#include "HDS/hds_common.h"
#include "GeomUtils/BBox.h"



class HDS_Face : public HDS_Common
{
private:
	static hdsid_t uid;

public:
	static void resetIndex() { uid = 0; }
	static void matchIndexToSize(size_t	size) { uid = size; }

	static hdsid_t assignIndex() { return uid++; }
	void setRefId(hdsid_t id) { refid = (id << 2) + HDS_Common::FROM_FACE; }

	HDS_Face();
	~HDS_Face();

	// Get the connected half-edge id
	// Explicit pointer access is handled by HDS_Mesh
	hdsid_t heID() const { return heid; }

	void setPicked(bool v) { isPicked = v; }

	void setScaleFactor(Float factor) { scalingFactor = factor; }
	Float getScalingFactor() const { return scalingFactor; }
	QVector3D scaleCorner(HDS_Vertex* v);
	vector<QVector3D> getScaledCorners();
	void scaleDown();

	uint16_t getFlag() const { return flag; }
public:
	hdsid_t index;
	hdsid_t refid;

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
};