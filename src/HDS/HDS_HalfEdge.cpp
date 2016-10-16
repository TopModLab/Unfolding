#include "HDS/HDS_Vertex.h"
#include "HDS/HDS_HalfEdge.h"
#include "HDS/HDS_Face.h"

#include <QDebug>

hdsid_t HDS_HalfEdge::uid = 0;

HDS_HalfEdge::HDS_HalfEdge()
	: index(uid++), refid(sInvalidHDS)
	, fid(sInvalidHDS), vid(sInvalidHDS)
	, prev_offset(0), next_offset(0), flip_offset(0)
	, cutTwin_offset(0), brt_offset(0)
	, flag(0), angle(0.0f)
{
}

HDS_HalfEdge::~HDS_HalfEdge()
{
}

void HDS_HalfEdge::computeCurvature()
{
#ifdef USE_LEGACY_FACTORY
	QVector3D normalCross = QVector3D::crossProduct(f->n, flip()->f->n);
	if ( QVector3D::dotProduct(normalCross,(v->pos - flip()->v->pos)) > 0 ) {
		isNegCurve = true;
	} else {
		isNegCurve = false;
	}

	//calculate curvature angle
	double dot = QVector3D::dotProduct(f->n, flip()->f->n)/(f->n.length()*flip()->f->n.length());
	angle = M_PI - acos (dot);
	if (isNegCurve) angle = 2* M_PI - angle;
	//cout<<"edge curvature angle"<<angle/3.14*180<<endl;
#endif // USE_LEGACY_FACTORY
}

QVector3D HDS_HalfEdge::computeNormal()
{
#ifdef USE_LEGACY_FACTORY
	///get planes on the edge
	HDS_Vertex* vp = flip()->prev()->v;
	HDS_Vertex* v1 = flip()->v;
	HDS_Vertex* vn = prev()->v;
	//get normals for plane prev and plane next
	QVector3D np = QVector3D::normal(v->pos, vp->pos, v1->pos);
	QVector3D nn = QVector3D::normal(v->pos, v1->pos, vn->pos);
	QVector3D n = np + nn;
	n.normalize();
	return n;
#endif // USE_LEGACY_FACTORY
	return QVector3D(0, 1, 0);
}
