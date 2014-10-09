#include "meshsmoother.h"
#include "hds_mesh.h"

#include "numerical.h"

MeshSmoother::MeshSmoother()
{
}

void MeshSmoother::smoothMesh_perVertex(HDS_Mesh *mesh) {
  auto vertex_comp = [](const HDS_Vertex *a, const HDS_Vertex *b) {
    return fabs(a->curvature) > fabs(b->curvature);
  };

  /// put all non-zero curvature vertices into a heap
  const double CTHRES = 1e-6;
  vector<HDS_Vertex*> H;
  for (auto v : mesh->vertSet) {
    if ( fabs(v->curvature) > CTHRES )
      H.push_back(v);
  }
  std::make_heap(H.begin(), H.end(), vertex_comp);
  
  /// modify the curvature of the vertices one by one, making them 0
  while (!H.empty()) {
    auto v = H.front();
    std::pop_heap(H.begin(), H.end(), vertex_comp);
    H.pop_back();

    /// make the curvature at this vertex 0
    auto neighbors = v->neighbors();

    /// update its neighbors
    map<HDS_Vertex*, double> entries;
    double sum_inv_dist = 0.0;
    for (auto neighbor : neighbors) {
      if (fabs(neighbor->curvature) < CTHRES) {
        entries.insert(make_pair(neighbor, 0.0));
      }
      else {
        double inv_dist = 1.0 / v->pos.distanceToPoint(neighbor->pos);
        entries.insert(make_pair(neighbor, inv_dist));
        sum_inv_dist += inv_dist;
      }
    }

    if (sum_inv_dist > 0) {
      for (auto entry : entries) {
        double w = entry.second / sum_inv_dist;
        entry.first->curvature += w * v->curvature;
      }

      v->curvature = 0;
    }

    /// remove
    auto newEnd = std::remove_if(H.begin(), H.end(), [=](const HDS_Vertex *a) {
      return fabs(a->curvature) <= CTHRES;
    });

    H.erase(newEnd, H.end());
    std::make_heap(H.begin(), H.end(), vertex_comp);
    cout << H.size() << endl;
  }

  double sum_curvature = std::accumulate(mesh->vertSet.begin(), mesh->vertSet.end(), 0.0, [](double val, HDS_Vertex* v) {
    return val + v->curvature;
  });

  cout << "sum = " << sum_curvature << endl;
}

void MeshSmoother::smoothMesh(HDS_Mesh *mesh)
{
  /// compute the smoothed curvature at each vertex
  int nVerts = mesh->vertSet.size();

  /// construct the smoothing matrix
  mat A(nVerts, nVerts, arma::fill::zeros);
  vec x(nVerts);
  unordered_map<int, int> vidxMap;
  int ridx = 0;
  for(auto v : mesh->vertSet) {
    vidxMap[v->index] = ridx;
    /// set the entry in vector x
    x(ridx) = v->curvature;
    ++ridx;
  }

  /// assemble the matrix A
  const double CTHRES = 1e-10;
  for(auto v : mesh->vertSet) {
    int ridx = vidxMap[v->index];

    if( fabs(v->curvature) < CTHRES ) {
      A(ridx, ridx) = 1.0;
    }
    else {
      /// set the entry in vector x
      /// get all its neighbors
      auto neighbors = v->neighbors();
      vector<pair<int, double>> entries;
      double sum_inv_dist = 0.0;
      for(auto neighbor : neighbors) {
        if( fabs(neighbor->curvature) < CTHRES ) {
          entries.push_back(make_pair(vidxMap[neighbor->index], 0.0));
        }
        else {
          double inv_dist = 1.0 / v->pos.distanceToPoint(neighbor->pos);
          entries.push_back(make_pair(vidxMap[neighbor->index], inv_dist));
          sum_inv_dist += inv_dist;
        }
      }

      for(auto entry : entries) {
        double w = entry.second / (sum_inv_dist + 1.0);
        A(entry.first, ridx) = w;
      }

      A(ridx, ridx) = 1.0 / (sum_inv_dist + 1.0);
    }
  }

  //cout << A << endl;

  /// compute new curvatures
  vec x_hat = A * x;
  double sum_curvature = 0.0;
  for(auto v : mesh->vertSet) {
    int ridx = vidxMap[v->index];

    /// set the entry in vector x
    v->curvature = x_hat(ridx);
    sum_curvature += x_hat(ridx);
  }

  cout << "sum = " << sum_curvature << endl;

  /// compute the new vertex positions using the new curvatures
}
