#pragma once

template <class NodeType, class EdgeType>
struct Graph
{
  Graph(){}
  ~Graph(){}

  void clear() {
    V.clear();
    E.clear();
    Es.clear();
  }

  vector<NodeType> V;
  vector<EdgeType> E;
  vector<EdgeType> Es;  // edges on the surface
};

struct SimpleNode {
  SimpleNode(){}
  SimpleNode(int idx, int idxref) :idx(idx), idxref(idxref){}

  int idx;      // graph index
  int idxref;   // reference to global index
};

struct SimpleEdge {
  SimpleEdge(){}
  SimpleEdge(int s, int t) :s(s), t(t){}
  int s, t;
};

typedef Graph<SimpleNode, SimpleEdge> SimpleGraph;

