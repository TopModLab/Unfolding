#include "meshsmoother.h"
#include "hds_mesh.h"

#include "numerical.h"

MeshSmoother::MeshSmoother()
{
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
