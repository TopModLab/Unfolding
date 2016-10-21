#include "HDS/HDS_Face.h"
#include "HDS/HDS_Vertex.h"
#include "HDS/HDS_HalfEdge.h"
#include <qdebug.h>

hdsid_t HDS_Face::uid = 0;

HDS_Face::HDS_Face()
	: index(uid++), refid(sInvalidHDS)
	, heid(sInvalidHDS), flag(0)
	, scalingFactor(1)
{
}

HDS_Face::~HDS_Face()
{
}

QVector3D HDS_Face::scaleCorner(HDS_Vertex* v)
{
	// v1_p ---------- v0
	//      |        |
	//      |    f   |
	//      |        |
	//      |        |
	// v2_  ---------- v1_n
	//    n/p

	//scale down based on center point

	// TODO: replace by hds_mesh function
	QVector3D c;// = center();
    QVector3D vec_cv = v->pos - c;
    return c + scalingFactor * vec_cv;

/*
	//scale down corner proportionally along edges
   QVector3D v0 = v->pos;


   //find half edge from v0 to v1_n
   HDS_HalfEdge* curHe = he;
   do {
       if (curHe->v == v) {
           break;
       }
       curHe = curHe->next;
   } while( curHe != he );

   QVector3D v1_n = curHe->next()->v->pos;
   QVector3D v2_n = curHe->next()->next()->v->pos;
   QVector3D v1_p = curHe->prev()->v->pos;
   QVector3D v2_p = curHe->prev()->prev()->v->pos;

   QVector3D v01_n = (1 - scalingFactor/2)* v0 + scalingFactor/2 *v1_n;
   QVector3D v12_p = (1 - scalingFactor/2)* v1_p + scalingFactor/2 *v2_p;

   QVector3D v01_p = (1 - scalingFactor/2)* v0 + scalingFactor/2 *v1_p;
   QVector3D v12_n = (1 - scalingFactor/2)* v1_n + scalingFactor/2 *v2_n;

   //get intersection point
   QVector3D v0_scaled;
   Utils::LineLineIntersect(v01_n, v12_p, v01_p, v12_n, &v0_scaled);
   return v0_scaled;*/

}

vector<QVector3D> HDS_Face::getScaledCorners()
{
#ifdef USE_LEGACY_FACTORY
	checkPlanar();

	if(!isNonPlanar) {
		// update the vertex position
		auto vertices = corners();
		for (auto v : vertices) {
			scaledCorners.insert(scaledCorners.end(), scaleCorner(v));
		}
	} else {
		//scale down non-planar face
	}
#endif
	return scaledCorners;
}

void HDS_Face::scaleDown()
{
#ifdef USE_LEGACY_FACTORY
	int n = 0;
	for (auto v : corners()) {
		v->pos = scaledCorners.at(n);
		n++;
	}
#endif
}
