#include "hds_vertex.h"
#include "hds_halfedge.h"

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
  he = nullptr;
}

HDS_Vertex::~HDS_Vertex(){}
