#ifndef HDS_FACE_H
#define HDS_FACE_H

#include "common.h"

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

	set<HDS_Face *> connectedFaces();
	QVector3D center() const;
	vector<HDS_Vertex*> corners() const;
	QVector3D computeNormal();

	QVector3D n;
	HDS_HalfEdge *he;

	int index;
	bool isPicked;
	bool isCutFace;
	bool isConnector;
	bool isFlap;

};

#endif // HDS_FACE_H
