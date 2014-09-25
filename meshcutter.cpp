#include "meshcutter.h"
#include "hds_mesh.h"
#include "utils.hpp"
#include "mathutils.hpp"

bool MeshCutter::cutMeshUsingEdges(HDS_Mesh *mesh, set<int> &edges)
{
  typedef HDS_HalfEdge he_t;
  typedef HDS_Vertex vert_t;
  typedef HDS_Face face_t;

  if( edges.empty() ) {
    edges = findCutEdges(mesh);
  }


  /// vertices connected to cut edges
  map<vert_t*, int> cutVerts;

  /// split each cut edge into 2 edges
  for(auto heIdx : edges) {
    auto he = mesh->heMap[heIdx];
    auto hef = he->flip;
    auto vs = he->v;
    auto ve = hef->v;

    /// record the cut vertices
    auto cit = cutVerts.find(vs);
    if( cit == cutVerts.end() ) {
      cutVerts.insert(make_pair(vs, 1));
    }
    else {
      ++cutVerts[vs];
    }
    cit = cutVerts.find(ve);
    if( cit == cutVerts.end() ) {
      cutVerts.insert(make_pair(ve, 1));
    }
    else {
      ++cutVerts[ve];
    }

    /// create a new face
    face_t *nf = new face_t;

    /// duplicate half edges
    he_t *he_new_flip = new he_t;
    he_t *hef_new_flip = new he_t;

    /// the flip of he
    he_new_flip->flip = he;
    he_new_flip->f = nf;
    he_new_flip->v = ve;
    he_new_flip->next = hef_new_flip;
    he_new_flip->prev = hef_new_flip;
    he_new_flip->isCutEdge = true;

    he->flip = he_new_flip;

    he_new_flip->index = HDS_HalfEdge::assignIndex();
    mesh->heSet.insert(he_new_flip);
    mesh->heMap.insert(make_pair(he_new_flip->index, he_new_flip));

    /// the flip of hef
    hef_new_flip->flip = hef;
    hef_new_flip->f = nf;
    hef_new_flip->v = vs;
    hef_new_flip->next = he_new_flip;
    hef_new_flip->prev = he_new_flip;
    hef_new_flip->isCutEdge = true;

    hef->flip = hef_new_flip;

    hef_new_flip->index = HDS_HalfEdge::assignIndex();
    mesh->heSet.insert(hef_new_flip);
    mesh->heMap.insert(make_pair(hef_new_flip->index, hef_new_flip));

    /// fix the new face
    nf->he = hef;
    nf->index = HDS_Face::assignIndex();
    nf->isCutFace = true;
    mesh->faceSet.insert(nf);
    mesh->faceMap.insert(make_pair(nf->index, nf));
  }

  mesh->printInfo();
  mesh->validate();

  /// check each cut vertices, merge faces if necessary
  for(auto cv : cutVerts) {
    if( cv.second > 1 ) {
      /// merge all incident faces
      cout << "merging cut faces incident to vertex #" << cv.first->index << endl;

      /// get all incident faces
      vector<face_t*> incidentFaces = mesh->incidentFaces(cv.first);
      vector<face_t*> cutFaces = Utils::filter(incidentFaces, [](face_t* f) {
        return f->isCutFace;
      });

#if 1
      /// test if we are merging the same face
      set<face_t*> cutFacesSet(cutFaces.begin(), cutFaces.end());
      if( cutFacesSet.size() == 1 ) {
        cout << "merging the same face." << endl;
      }
      else {
        cout << "merging different faces." << endl;
      }
#endif

      vector<he_t*> incidentHEs = mesh->incidentEdges(cv.first);
      vector<he_t*> cutEdges = Utils::filter(incidentHEs, [](he_t* e) {
        return e->f->isCutFace;
      });

      /// merge them
      /// since they are discovered in a given order (CW or CCW), just glue them together
      /// each cut vertex is splitted into n vertices, depending on the number of cut faces
      /// it is connected to. non-cut vertices are not splitted.
      /// as shown below
      ///
      ///            ncv
      ///            / \
      ///           /cf \
      ///           cv----(cv)           <--- cut vertex is splitted into 2 vertices
      ///            \ cf /
      ///             \  /
      ///              \/
      ///             ncv

      /// the degree of the cut vertex
      int k = cutFaces.size();

      cout << "cut vertex degree = " << k << endl;
      cout << "cut faces number = " << cutFaces.size() << endl;
      cout << "incident edges = " << incidentHEs.size() << endl;
      for(auto x : incidentHEs)
        cout << x->index << " @ " << x->f->index << "[" << x->v->index << ", " << x->flip->v->index << "]" << endl;
      cout << "cut edges = " << cutEdges.size() << endl;
      for(auto x : cutEdges)
        cout << x->index << " @ " << x->f->index << "[" << x->v->index << ", " << x->flip->v->index << "]" << endl;
#if 0
      /// verify incident edges
      for(int i=0;i<incidentHEs.size();++i) {
        int j = (i+1) % incidentHEs.size();
        if( incidentHEs[i]->flip->next != incidentHEs[j] ) cout << "failed @" << i << endl;
      }
#endif

      vector<vert_t*> cv_new(k);
      for(int i=0;i<k;++i){
        cv_new[i] = new vert_t(*cv.first);
        cv_new[i]->he = cutEdges[i];

        /// assign a new id to the vertex
        cv_new[i]->index = HDS_Vertex::assignIndex();
      }

      /// divide all incident half edges into k group
      vector<vector<he_t*>> heGroup(k);
      int groupIdx = 0, nextGroup = 1;
      he_t *he = cutEdges[groupIdx];
      he_t *curHE = he;
      do {
        cout << "gid = " << groupIdx << endl;
        /// put curHE into current group
        heGroup[groupIdx].push_back(curHE);

        curHE = curHE->flip->next;
        if( curHE == cutEdges[nextGroup] ) {
          /// switch to the next group
          ++groupIdx;
          nextGroup = (groupIdx + 1) % k;
        }

      } while( curHE != he );

      for(int i=0;i<k;++i) {
        cout << "Group #" << i << " has " << heGroup[i].size() << " half edges." << endl;
        for(auto x : heGroup[i])
          cout << x->index << endl;
      }

      /// k-way split the vertex, assign new vertex to the k groups
      for(int i=0;i<k;++i) {
        for(auto x : heGroup[i]) {
          x->v = cv_new[i];
          cout << x->flip->v->pos.x() << ", "
               << x->flip->v->pos.y() << ", "
               << x->flip->v->pos.z() << endl;
        }

        /// for this group, connect its tail with head
        heGroup[i].front()->prev = heGroup[i].back()->flip;
        heGroup[i].back()->flip->next = heGroup[i].front();
      }

      /// remove the old vertex and add new vertices
      cout << "removing vertex " << cv.first->index << endl;
      mesh->vertSet.erase(mesh->vertSet.find(cv.first));
      mesh->vertMap.erase(cv.first->index);
      delete cv.first;

      for( auto v : cv_new ) {
        cout << "inserting vertex " << v->index << endl;
        mesh->vertSet.insert(v);
        mesh->vertMap.insert(make_pair(v->index, v));
      }

      /// remove the old cut faces and add the new unified cut face
      for(auto f : cutFaces) {
        if( mesh->faceSet.find(f) != mesh->faceSet.end() ) {
          cout << "removing face " << f->index << endl;
          mesh->faceSet.erase(f);
          mesh->faceMap.erase(f->index);
          delete f;
        }
      }

      face_t *nf = new face_t;
      nf->index = HDS_Face::assignIndex();
      cout << "new cut face index = " << nf->index << endl;
      nf->isCutFace = true;
      nf->he = cutEdges.front();
      mesh->faceSet.insert(nf);
      mesh->faceMap.insert(make_pair(nf->index, nf));

      /// update the incident face of all edges of this face
      auto fhe = cutEdges.front();
      do {
        fhe->f = nf;
        fhe = fhe->next;
      } while( fhe != cutEdges.front() );

      mesh->printInfo("merged");
      mesh->validate();
    }
  }

  /// update the curvature of each vertex
  for( auto &v : mesh->vertSet ) {
    v->computeCurvature();
    cout << v->index << ": " << (*v) << endl;
  }

  return true;
}

MeshCutter::PathInfo MeshCutter::allPairShortestPath(HDS_Mesh *mesh) {
  PathInfo m;
  m.resize(mesh->vertSet.size());
  const double realmax = 1e10;
  for(auto &x:m) {
    x.resize(mesh->vertSet.size(), pair<double, int>(realmax, -1));
  }

  /// floyd-warshall
  for(auto e : mesh->heSet) {
    auto u = e->v;
    auto v = e->flip->v;
    m[u->index][v->index] = make_pair(u->pos.distanceToPoint(v->pos), v->index);
  }

  for(int k=0;k<mesh->vertSet.size();++k) {
    m[k][k] = make_pair(0, -1);
    for(int i=0;i<mesh->vertSet.size();++i) {
      for(int j=0;j<mesh->vertSet.size();++j) {
        if( m[i][k].first + m[k][j].first < m[i][j].first ) {
          m[i][j] = make_pair(m[i][k].first + m[k][j].first, m[i][k].second);
        }
      }
    }
  }

  return m;
}

vector<int> MeshCutter::retrivePath(PathInfo m, int u, int v) {
  if( m[u][v].second == -1 ) return vector<int>();
  vector<int> path;
  path.push_back(u);

  while( u != v ) {
    u = m[u][v].second;
    path.push_back(u);
  }

  return path;
}

vector<MeshCutter::Edge> MeshCutter::minimumSpanningTree(PQ &edges, int nVerts) {
  cout << "finding MST ..." << endl;
  cout << edges.size() << endl;
  vector<MeshCutter::Edge> mst;
  UnionFind uf(nVerts);   // connected part
  while( !edges.empty() && mst.size() < nVerts - 1 ) {
    auto e = edges.top();
    edges.pop();
    cout << "edge (" << e.i << ", " << e.j << ") @ [" << e.u << ", " << e.v << ", " << e.weight << "]" << endl;
    if( !uf.Connected(e.i, e.j) ) {
      cout << "accepted." << endl;
      mst.push_back(e);
      uf.Union(e.i, e.j);
    }
  }
  return mst;
}

set<int> MeshCutter::findCutEdges(HDS_Mesh *mesh)
{
  cout << "Finding cut edges..." << endl;
  auto isBadVertex = [](HDS_Vertex* v) -> bool {
    double sum = 0;
    auto he = v->he;
    auto curHE = he->flip->next;
    bool hasCutFace = false;
    do {
      if( !curHE->f->isCutFace ) {
        QVector3D v1 = he->flip->v->pos - he->v->pos;
        QVector3D v2 = curHE->flip->v->pos - curHE->v->pos;
        double nv1pnv2 = v1.length() * v2.length();
        double inv_nv1pnv2 = 1.0 / nv1pnv2;
        double cosVal = QVector3D::dotProduct(v1, v2) * inv_nv1pnv2;
        double angle = acos(clamp<double>(cosVal, -1.0, 1.0));
        sum += angle;
      }
      else hasCutFace = true;
      he = curHE;
      curHE = he->flip->next;
    }while( he != v->he ) ;

    /// either sums up roughly 2 * Pi, or has a cut face connected.
    return (sum > PI2) || (sum < PI2 && !hasCutFace);
  };

  set<int> cutEdges;

  /// find out all bad vertices
  unordered_set<HDS_Vertex*> cutVertices = Utils::filter_set(mesh->vertSet, isBadVertex);
  unordered_map<HDS_Vertex*, int> cutVerticesIndex;
  /// label the bad vertices
  int vidx = 0;
  for(auto x : cutVertices) {
    cutVerticesIndex[x] = vidx;
    ++vidx;
  }
  cout << "#cut vertices: " << cutVertices.size() << endl;
  PathInfo pathinfo = allPairShortestPath(mesh);
  PQ edges;
  for(auto x : cutVertices) {
    for(auto y : cutVertices) {
      if( x->index < y->index ) {
        edges.push(Edge(cutVerticesIndex[x], cutVerticesIndex[y], x->index, y->index, pathinfo[x->index][y->index].first));
      }
    }
  }

  vector<Edge> mst = minimumSpanningTree(edges, cutVertices.size());
  cout << "MST edges:" << endl;
  for(auto &e : mst) {
    cout << e.u << ", " << e.v << ", " << e.weight << endl;
  }

  vector<vector<int>> mstEdges;
  for(auto &e : mst) {
    vector<int> path = retrivePath(pathinfo, e.u, e.v);
    mstEdges.push_back(path);
  }

  /// generate a tree connecting all bad vertices using shortest path
  /// this basically bacomes a minimum spanning tree.
  ///
  for(auto &path : mstEdges) {
    /// if this edge is connected to a non-planar vertex, cut this edge,
    /// and form a cut edge tree starting from this edge
    for(int i=0;i<path.size()-1;++i) {
      int u = path[i], v = path[i+1];
      auto vu = mesh->vertMap[u], vv = mesh->vertMap[v];
      auto he = vu->he;
      auto curHE = he;
      do {
        if( curHE->flip->v == vv ) {
          cutEdges.insert(curHE->index);
          curHE->setCutEdge(true);
          break;
        }
        curHE = curHE->flip->next;
      } while( curHE != he );
    }
  }
  cout << "#cut edges = " << cutEdges.size() << endl;
  return cutEdges;
}
