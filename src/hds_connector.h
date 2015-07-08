#ifndef HDS_CONNECTOR_H
#define HDS_CONNECTOR_H

#include "hds_halfedge.h"
#include "hds_face.h"

class HDS_Connector
{
private:
	static size_t uid;

public:
	static void resetIndex() { uid = 0; }
	static size_t assignIndex() { return uid++; }

	HDS_Connector();
	~HDS_Connector();

	HDS_Connector(HDS_HalfEdge* he);

	HDS_Connector(const HDS_Connector &other);
	HDS_Connector operator=(const HDS_Connector &other);


	vector<HDS_Face*> faces; //corresponding faces pointer
	HDS_HalfEdge* he; //original corresponding he before adding connector

	HDS_Connector* twin; //points to its twin connector created in a cut event

	int index;
	bool isFlap; //if original he is cutted, connector becomes flap

	int bezierSamples;
	double bezierCurvature;
	double convergingPoint;


};

#endif // HDS_CONNECTOR_H
