#pragma once

#include "common.h"

class HDS_Mesh;
class HDS_Vertex;
class HDS_Face;
class HDS_HalfEdge;

class MeshIterator
{
public:
  static queue<pair<HDS_Vertex*,int>> BFS(HDS_Mesh* mesh, int vidx);
};

