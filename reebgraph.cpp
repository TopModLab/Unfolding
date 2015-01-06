#include "reebgraph.h"


ReebGraph::ReebGraph()
{
}


ReebGraph::~ReebGraph()
{
}

void ReebGraph::build(HDS_Mesh *mesh, const vector<double> &fval)
{
  // process the vertices
  // simply create a list of nodes
  for (auto v : mesh->vertSet) {
    createNode(v, fval[v->index]);
  }

  unordered_set<pair<int, int>> edgeSet;
  // process the triangles
  for (auto f : mesh->faceSet) {
    // for each edge in the triangle, create an arc
    auto corners = f->corners();
    vector<Edge> edges;
    for (int i = 0; i < corners.size(); ++i) {
      auto cur = corners[i];
      auto nxt = corners[( + 1) % corners.size()];

      if (cur->index > nxt->index) {
        auto tmp = nxt;
        nxt = cur; cur = tmp;
      }

      Edge e = make_pair(cur->index, nxt->index);
      if (edgeSet.find(e) == edgeSet.end())
      {
        edgeSet.insert(e);
        createArc(e);
      }
      edges.push_back(e);
    }

    // accept only triangle meshes
    assert(edges.size() == 3);
    mergePaths(edges[0], edges[1], edges[2]);

    for (auto e : edges) {
      // remove it if all its vertices are finalized
    }
  }
}

void ReebGraph::createNode(HDS_Vertex *v, double f)
{
  nodes.insert(new Node(v->index, f));
}

void ReebGraph::createArc(const Edge &e)
{
  auto a = new Arc(e);
  arcs.insert(a);
  edgeArcMap.insert(make_pair(e, a));
}

void ReebGraph::mergePaths(const Edge &e0, const Edge &e1, const Edge &e2)
{
  auto a0 = edgeArcMap[e0], a1 = edgeArcMap[e1], a2 = edgeArcMap[e2];
  glueByMergeSorting(a0, a1, e0, e1);
  glueByMergeSorting(a0, a2, e0, e2);
}

void ReebGraph::glueByMergeSorting(Arc *a0, Arc *a1, const Edge& e0, const Edge& e1)
{
  while (true) {
    auto n0 = bottomNode(a0);
    auto n1 = bottomNode(a1);
    if (n0->f > n1->f) {
      mergeArcs(a0, a1);
    }
    else {
      mergeArcs(a1, a0);
    }

    a0 = nextArcMappedToEdge(a0, e0);
    a1 = nextArcMappedToEdge(a1, e1);

    if (!isValidArc(a0) || !isValidArc(a1)) break;
  }
}

Node* ReebGraph::bottomNode(Arc *a)
{

}

void ReebGraph::mergeArcs(Arc *a0, Arc *a1)
{
  // merge the arcs based on the embedding information
}

Arc* ReebGraph::nextArcMappedToEdge(Arc *a, const Edge &e)
{

}

bool ReebGraph::isValidArc(Arc *a)
{

}
