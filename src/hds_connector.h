#ifndef HDS_CONNECTOR_H
#define HDS_CONNECTOR_H

#include "hds_halfedge.h"
#include "hds_face.h"
#include "hds_vertex.h"

#include <QString>

class HDS_Connector
{
private:
	static size_t uid;

public:
	static void resetIndex() { uid = 0; }
	static size_t assignIndex() { return uid++; }

	~HDS_Connector();

	HDS_Connector(HDS_HalfEdge* he, HDS_HalfEdge* hef);

	HDS_Connector(const HDS_Connector &other);
	HDS_Connector operator=(const HDS_Connector &other);


	vector<HDS_Face*> faces; //corresponding faces pointer
	vector<HDS_HalfEdge*> hes; //corresponding half edges pointer
	vector<HDS_Vertex*> verts; //corresponding vertices pointer

	//HDS_Connector* flapTwin; //points to its twin connector created in a cut event
	void setOriginalPositions();

	int index;
	bool isFlap; //if original he is cutted, connector becomes flap

	//config connector
	static void setConnector(std::map<QString, double> config);
	static void setScale(double scale);

	static double getScale(){return scale;}

private:
	HDS_HalfEdge* he; //original corresponding he before adding connector
	HDS_HalfEdge* hef;
	QVector3D p00;
	QVector3D p01;
	QVector3D p10, p11, p20, p21;
	vector<QVector3D> bezierPos_front, bezierPos_back;

private:
	vector<QVector3D> getBezierCurvePos(int index);
	vector<QVector3D> calculateBezierCurve(QVector3D p0, QVector3D p1, QVector3D p2);

private:
	static int shape;
	static double scale;//size
	static double curv;
	static int nSamples;
	static double cp;
	static int opening;


};

#endif // HDS_CONNECTOR_H
