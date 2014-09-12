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

set<HDS_Face *> HDS_Face::connectedFaces() const
{
  throw "Not implemented.";
}

QVector3D HDS_Face::center() const
{
  auto cs = corners();
  QVector3D c;
  for(auto p : cs) {
    c += p->pos;
  }
  c /= (qreal) cs.size();
  return c;
}

vector<HDS_Vertex*> HDS_Face::corners() const
{
  HDS_HalfEdge *curHE = he;
  vector<HDS_Vertex*> corners;
  do {
    corners.push_back(curHE->v);
    curHE = curHE->next;
  } while( curHE != he );
  return corners;
}
