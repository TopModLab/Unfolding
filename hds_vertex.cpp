#include "hds_vertex.h"
#include "hds_halfedge.h"

HDS_Vertex::HDS_Vertex() {
  isPicked = false;
  index = -1;
}

HDS_Vertex::HDS_Vertex(const QVector3D &pos):pos(pos) {
  isPicked = false;
  index = -1;
}

HDS_Vertex::~HDS_Vertex(){}
