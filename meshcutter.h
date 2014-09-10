#ifndef MESHCUTTER_H
#define MESHCUTTER_H

#include "common.h"

class HDS_Mesh;
class HDS_HalfEdge;

class MeshCutter
{
public:
  MeshCutter();

  static bool cutMeshUsingEdges(HDS_Mesh *mesh, set<HDS_HalfEdge*> &edges);
};

#endif // MESHCUTTER_H
