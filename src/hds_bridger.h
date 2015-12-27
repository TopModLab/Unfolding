#ifndef HDS_BRIDGER_H
#define HDS_BRIDGER_H

#include "hds_mesh.h"

#include <QString>

class HDS_Bridger
{

	typedef HDS_Face face_t;
	typedef HDS_HalfEdge he_t;
	typedef HDS_Vertex vert_t;

private:
	static size_t uid;

public:
	static void resetIndex() { uid = 0; }
	static size_t assignIndex() { return uid++; }

	~HDS_Bridger();

	HDS_Bridger(HDS_HalfEdge* he, HDS_HalfEdge* hef, HDS_Vertex* v1, HDS_Vertex* v2);

	HDS_Bridger(const HDS_Bridger &other);
	HDS_Bridger operator=(const HDS_Bridger &other);


	vector<face_t*> faces; //corresponding faces pointer
	vector<he_t*> hes; //corresponding half edges pointer
	vector<vert_t*> verts; //corresponding vertices pointer

	//HDS_Bridger* flapTwin; //points to its twin Bridger created in a cut event
	void createBridge();
	void setOriginalPositions(HDS_Vertex* v1, HDS_Vertex* v2);
	face_t* bridging(HDS_HalfEdge* he1, HDS_HalfEdge* he2);

	int index;
	bool isFlap; //if original he is cutted, Bridger becomes flap

	//config Bridger
	void setCutFace(face_t* cutFace1, face_t* cutFace2);
	static void setBridger(std::map<QString, double> config);
	static void setScale(double scale);

	static double getScale(){return scale;}

private:
	HDS_Face* cutFace1, *cutFace2;
	HDS_HalfEdge* he; //original corresponding he before adding Bridger
	HDS_HalfEdge* hef;
	QVector3D p00;
	QVector3D p01;
	QVector3D p10, p11, p20, p21;
	vector<QVector3D> bezierPos_front, bezierPos_back;

private:

	vector<QVector3D> calculateBezierCurve(QVector3D p0, QVector3D p1, QVector3D p2);

private:
	static int shape;
	static double scale;//size
	static double curv;
	static int nSamples;
	static double cp;
	static int opening;


};

#endif // HDS_BRIDGER_H
