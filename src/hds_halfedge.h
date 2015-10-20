#ifndef HDS_EDGE_H
#define HDS_EDGE_H

#include "hds_face.h"
#include "hds_vertex.h"

class HDS_Face;
class HDS_Vertex;

class HDS_HalfEdge
{
private:
	static size_t uid;

public:
	enum EDGE_STATUS
	{
		DEFAULT = 0,
		PICKED = 1,
		CUTEDGE = 1 << 1,
		EXTENDED = 1 << 2
	};

	static void resetIndex() { uid = 0; }
	static size_t assignIndex() { return uid++; }

	HDS_HalfEdge();
	~HDS_HalfEdge();

	HDS_HalfEdge(const HDS_HalfEdge& other);
	HDS_HalfEdge operator=(const HDS_HalfEdge& other);

	void setPicked(bool v) { isPicked = v; flip->isPicked = v; }
	void setCutEdge(bool v) { isCutEdge = v; flip->isCutEdge = v; }
	void setFlip(HDS_HalfEdge* thef) {flip = thef; thef->flip = this;}

	HDS_Face *f;
	HDS_Vertex *v;
	HDS_HalfEdge *prev, *next, *flip;

	HDS_HalfEdge *cutTwin;  // pointer to its twin halfedge created in a cut event

	int index;
	bool isPicked;
	bool isCutEdge;
	bool isExtended;//From hollower
	int status;
};

#endif // HDS_EDGE_H
