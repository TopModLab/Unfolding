#include "hds_face.h"
#include "hds_halfedge.h"

HDS_Face::HDS_Face()
{
  isPicked = false;
  isCutFace = false;
  index = -1;
  he = nullptr;
}

HDS_Face::~HDS_Face(){}

HDS_Face::HDS_Face(const HDS_Face &other)
{
  isPicked = other.isPicked;
  isCutFace = other.isCutFace;
  index = other.index;
  normal = other.normal;
  he = nullptr;
}

HDS_Face HDS_Face::operator=(const HDS_Face &other)
{
  throw "Not implemented.";
}

QVector3D HDS_Face::center() const
{
  HDS_HalfEdge *curHE = he;
  QVector3D c;
  int count = 0;
  do {
    c += curHE->v->pos;
    ++count;
    curHE = curHE->next;
  } while( curHE != he );

  c = c / (qreal) count;
  return c;
}
