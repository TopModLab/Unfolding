#include "hds_halfedge.h"

HDS_HalfEdge::HDS_HalfEdge()
{
  isPicked = false;
  isCutEdge = false;
  index = -1;
  f = nullptr;
  v = nullptr;
  flip = nullptr;
  prev = nullptr;
  next = nullptr;
}

HDS_HalfEdge::~HDS_HalfEdge()
{

}

HDS_HalfEdge::HDS_HalfEdge(const HDS_HalfEdge &other)
{
  isPicked = other.isPicked;
  isCutEdge = other.isCutEdge;
  index = other.index;
  f = nullptr;
  v = nullptr;
  flip = nullptr;
  prev = nullptr;
  next = nullptr;
}

HDS_HalfEdge HDS_HalfEdge::operator=(const HDS_HalfEdge &other)
{
    throw "Not implemented.";
}
