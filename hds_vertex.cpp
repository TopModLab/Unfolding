#include "hds_vertex.h"
#include "hds_halfedge.h"

HDS_Vertex::HDS_Vertex() {
  isPicked = false;
}

HDS_Vertex::HDS_Vertex(const QVector3D &pos):pos(pos) {
  isPicked = false;
}

HDS_Vertex::~HDS_Vertex(){}
