#include "hds_face.h"
#include "hds_halfedge.h"

HDS_Face::HDS_Face()
{
  isPicked = false;
  index = -1;
  he = nullptr;
}

HDS_Face::~HDS_Face(){}

HDS_Face::HDS_Face(const HDS_Face &other)
{
  isPicked = other.isPicked;
  index = other.index;
  normal = other.normal;
  he = nullptr;
}
