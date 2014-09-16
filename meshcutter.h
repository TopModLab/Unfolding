#ifndef MESHCUTTER_H
#define MESHCUTTER_H

#include "common.h"

class HDS_Mesh;
class HDS_Face;
class HDS_HalfEdge;
class HDS_Vertex;

class MeshCutter
{
public:

  static bool cutMeshUsingEdges(HDS_Mesh *mesh, set<HDS_HalfEdge*> &edges);

private:
  static set<HDS_HalfEdge*> findCutEdges(HDS_Mesh *mesh);
};

#endif // MESHCUTTER_H
