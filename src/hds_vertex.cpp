#include "hds_vertex.h"
#include "hds_halfedge.h"
#include "hds_face.h"
#include "mathutils.hpp"

hdsid_t HDS_Vertex::uid = 0;

HDS_Vertex::HDS_Vertex(const QVector3D &pos, int idx, int rid)
	: pos(pos)
	, index(idx), refid(rid)
	, colorVal(0)
	, isPicked(false), he(nullptr)
	, flag(FLAG_DEFAULT)
{
}

HDS_Vertex::HDS_Vertex(const HDS_Vertex& v)
{
	isPicked = v.isPicked;
	index = v.index;
	refid = v.refid;
	pos = v.pos;
	normal = v.normal;
	curvature = v.curvature;
	colorVal = v.colorVal;
	he = nullptr;
	rtype = v.rtype;
	morseFunctionVal = v.morseFunctionVal;
}

HDS_Vertex HDS_Vertex::operator=(const HDS_Vertex &other)
{
	throw "Not implemented.";
}

HDS_Vertex::~HDS_Vertex(){}

vector<HDS_Vertex*> HDS_Vertex::neighbors() const {
	HDS_HalfEdge *curHE = he;
	vector<HDS_Vertex*> neighbors;
	do {
		neighbors.push_back(curHE->flip->v);
		curHE = curHE->flip->next;
	} while( curHE != he );
	return neighbors;
}

uint16_t HDS_Vertex::getFlag() const
{
	return (uint16_t)(-(int16_t)isPicked) & PICKED;

}

void HDS_Vertex::computeCurvature()
{
	curvature = 0;
	auto prevHE = he;
	auto curHE = he->flip->next;
	do {
		if( !prevHE->f->isCutFace )
		{
			QVector3D v1 = prevHE->flip->v->pos - prevHE->v->pos;
			QVector3D v2 = curHE->flip->v->pos - curHE->v->pos;
			double nv1pnv2 = v1.length() * v2.length();
			double inv_nv1pnv2 = 1.0 / nv1pnv2;
			double cosVal = QVector3D::dotProduct(v1, v2) * inv_nv1pnv2;
			double angle = acos(clamp<double>(cosVal, -1.0, 1.0));
			curvature += angle;
		}
		prevHE = curHE;
		curHE = prevHE->flip->next;
	}while( prevHE != he );
	curvature = PI2 - curvature;

}

void HDS_Vertex::computeNormal()
{
	auto prevHE = he;
	auto curHE = he->flip->next;
	normal = QVector3D(0, 0, 0);
	do {
		if (!prevHE->f->isCutFace) {
			QVector3D v1 = prevHE->flip->v->pos - prevHE->v->pos;
			QVector3D v2 = curHE->flip->v->pos - curHE->v->pos;


			normal += QVector3D::crossProduct(v2, v1) * acos(QVector3D::dotProduct(v2.normalized(), v1.normalized()));
		}
		prevHE = curHE;
		curHE = prevHE->flip->next;
	} while (prevHE != he);

	normal.normalize();
}
