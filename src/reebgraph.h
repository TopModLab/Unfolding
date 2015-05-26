#pragma once

#include "common.h"
#include "hds_face.h"
#include "hds_vertex.h"
#include "hds_mesh.h"

class ReebGraph
{
public:
  struct Node {
    Node(int vidx, double f) :idx(vidx), f(f){}
    int idx;
    double f;
  };
  
  typedef pair<int, int> Edge;

  struct EdgeHasher {
    std::size_t operator()(Edge const& e) const
    {
      std::size_t h1 = std::hash<int>()(e.first);
      std::size_t h2 = std::hash<int>()(e.second);
      return h1 ^ (h2 << 1);
    }
  };

  struct Arc {
    Arc(const Edge &edge) {
      e.push_back(edge);
    }

    // edge and embedding information
    vector<Edge> e;
  };

public:
  ReebGraph();
  ~ReebGraph();

  void build(HDS_Mesh *mesh, const vector<double> &fval);

protected:
  void createNode(HDS_Vertex *v, double f);
  void createArc(const Edge &e);
  void mergePaths(const Edge &e0, const Edge &e1, const Edge &e2);
  void glueByMergeSorting(Arc *a0, Arc *a1, const Edge& e0, const Edge& e1);
  Node* bottomNode(Arc *a);
  void mergeArcs(Arc *a0, Arc *a1);
  Arc* nextArcMappedToEdge(Arc *a, const Edge &e);
  bool isValidArc(Arc *a);

private:
  unordered_set<Node*> nodes;
  unordered_set<Arc*> arcs;
  unordered_map<Edge, Arc*, EdgeHasher> edgeArcMap;
};

