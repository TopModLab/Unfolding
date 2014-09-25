#include "hds_vertex.h"
#include "hds_halfedge.h"
#include "mathutils.hpp"

size_t HDS_Vertex::uid = 0;

HDS_Vertex::HDS_Vertex() {
  isPicked = false;
  index = -1;
  he = nullptr;
}

HDS_Vertex::HDS_Vertex(const QVector3D &pos):pos(pos) {
  isPicked = false;
  index = -1;
  he = nullptr;
}

HDS_Vertex::HDS_Vertex(const HDS_Vertex& v)
{
  isPicked = v.isPicked;
  index = v.index;
  pos = v.pos;
  curvature = v.curvature;
  he = nullptr;
}

HDS_Vertex HDS_Vertex::operator=(const HDS_Vertex &other)
{
  throw "Not implemented.";
}

void HDS_Vertex::computeCurvature()
{
  curvature = 0;
  auto prevHE = he;
  auto curHE = he->flip->next;
  do {
    if( !prevHE->f->isCutFace ) {
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

HDS_Vertex::~HDS_Vertex(){}
