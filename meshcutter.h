#ifndef MESHCUTTER_H
#define MESHCUTTER_H

#include "common.h"
#include "unionfind.h"

class HDS_Mesh;
class HDS_Face;
class HDS_HalfEdge;
class HDS_Vertex;

class MeshCutter
{
public:

  static bool cutMeshUsingEdges(HDS_Mesh *mesh, set<HDS_HalfEdge*> &edges);

private:
  struct Edge {
    Edge(){}
    Edge(int i, int j, int u, int v, double w):i(i), j(j), u(u), v(v), weight(w){}
    int i, j;
    int u, v;
    double weight;
    bool operator>(const Edge& e) const {
      return weight > e.weight;
    }
  };
  typedef vector<vector<pair<double, int>>> PathInfo;

  typedef std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge>> PQ;

  static PathInfo allPairShortestPath(HDS_Mesh *mesh);
  static vector<int> retrivePath(PathInfo m, int u, int v);

  static set<HDS_HalfEdge*> findCutEdges(HDS_Mesh *mesh);
  static vector<Edge> minimumSpanningTree(PQ &edges, int);
};

#endif // MESHCUTTER_H
