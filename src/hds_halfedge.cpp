#include "hds_vertex.h"
#include "hds_halfedge.h"
#include "hds_face.h"

#include <QDebug>

hdsid_t HDS_HalfEdge::uid = 0;

HDS_HalfEdge::HDS_HalfEdge()
	//: flag(DEFAULT)
{
	isPicked = false;
	isCutEdge = false;
	isExtended = false;
	index = -1;
	refid = 0;
	f = nullptr;
	v = nullptr;
	flip = nullptr;
	prev = nullptr;
	next = nullptr;
	cutTwin = nullptr;
}

uint16_t HDS_HalfEdge::getFlag() const
{
	return (uint16_t)(-(int16_t)isPicked) & PICKED
		| (uint16_t)(-(int16_t)isCutEdge) & CUTEDGE
		| (uint16_t)(-(int16_t)isExtended) & EXTENDED;
}

HDS_HalfEdge::~HDS_HalfEdge()
{

}

HDS_HalfEdge::HDS_HalfEdge(const HDS_HalfEdge &other)
	: index(other.index), refid(other.refid)
{
	isPicked = other.isPicked;
	isCutEdge = other.isCutEdge;
	//index = other.index;
	flag = other.flag;
	f = nullptr;
	v = nullptr;
	flip = nullptr;
	prev = nullptr;
	next = nullptr;
	cutTwin = nullptr;
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
}
