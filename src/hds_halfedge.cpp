#include "hds_vertex.h"
#include "hds_halfedge.h"
#include "hds_face.h"

#include <QDebug>

hdsid_t HDS_HalfEdge::uid = 0;

HDS_HalfEdge::HDS_HalfEdge()
	: index(-1), refid(0)
	, flag(0)
	, f(nullptr), v(nullptr)
	, flip(nullptr), prev(nullptr), next(nullptr), cutTwin(nullptr)
{
}

HDS_HalfEdge::~HDS_HalfEdge()
{
}

HDS_HalfEdge::HDS_HalfEdge(const HDS_HalfEdge &other)
	: index(other.index), refid(other.refid)
	, flag(other.flag)
	, f(nullptr), v(nullptr)
	, flip(nullptr), prev(nullptr), next(nullptr), cutTwin(nullptr)
{
}

HDS_HalfEdge HDS_HalfEdge::operator=(const HDS_HalfEdge &other)
{
	throw "Not implemented.";
}

void HDS_HalfEdge::computeCurvature()
{
	QVector3D normalCross = QVector3D::crossProduct(f->n, flip->f->n);
	if ( QVector3D::dotProduct(normalCross,(v->pos - flip->v->pos)) > 0 ) {
		isNegCurve = true;
	} else {
		isNegCurve = false;
	}

	//calculate curvature angle
	double dot = QVector3D::dotProduct(f->n, flip->f->n)/(f->n.length()*flip->f->n.length());
	angle = M_PI - acos (dot);
	if (isNegCurve) angle = 2* M_PI - angle;
	//cout<<"edge curvature angle"<<angle/3.14*180<<endl;
}

QVector3D HDS_HalfEdge::computeNormal()
{
	///get planes on the edge
	HDS_Vertex* vp = flip->prev->v;
	HDS_Vertex* v1 = flip->v;
	HDS_Vertex* vn = prev->v;
	//get normals for plane prev and plane next
	QVector3D np = QVector3D::normal(v->pos, vp->pos, v1->pos);
	QVector3D nn = QVector3D::normal(v->pos, v1->pos, vn->pos);
	QVector3D n = np + nn;
	n.normalize();
	return n;
}
