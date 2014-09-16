#include "meshcutter.h"
#include "hds_mesh.h"
#include "utils.hpp"

bool MeshCutter::cutMeshUsingEdges(HDS_Mesh *mesh, set<HDS_HalfEdge *> &edges)
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
  for(auto he : edges) {
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

    he_new_flip->index = mesh->heSet.size();
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

    hef_new_flip->index = mesh->heSet.size();
    mesh->heSet.insert(hef_new_flip);
    mesh->heMap.insert(make_pair(hef_new_flip->index, hef_new_flip));

    /// fix the new face
    nf->he = hef;
    nf->index = mesh->faceSet.size();
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

        /// reuse the index of the original cut vertex, but use new indices for other vertices
        if( i > 0 ) {
          cv_new[i]->index = mesh->vertSet.size() + i - 1;
        }
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
      mesh->vertSet.erase(cv.first);
      mesh->vertMap.erase(cv.first->index);

      for( auto v : cv_new ) {
        mesh->vertSet.insert(v);
        mesh->vertMap.insert(make_pair(v->index, v));
      }

      /// delete the old vertex
      delete cv.first;

      /// remove the old cut faces and add the new unified cut face
      for(auto f : cutFaces) {
        if( mesh->faceSet.find(f) != mesh->faceSet.end() ) {
          mesh->faceSet.erase(f);
          mesh->faceMap.erase(f->index);
          delete f;
        }
      }

      face_t *nf = new face_t;
      nf->index = mesh->faceSet.size();
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

  return true;
}

set<HDS_HalfEdge *> MeshCutter::findCutEdges(HDS_Mesh *mesh)
{
  auto isBadVertex = [](HDS_Vertex* v) -> bool {
    const double THRES = 1e-6;
    const double PI2 = 3.1415926535897 * 2.0;

    cout << "vert #" << v->index << "\t";
    double sum = 0;
    auto he = v->he;
    auto curHE = he->flip->next;
    bool isPlanar = true;
    QVector3D normal = he->f->n;
    cout << normal << "\t";
    do {
      QVector3D v1 = he->flip->v->pos - he->v->pos;
      QVector3D v2 = curHE->flip->v->pos - curHE->v->pos;
      double nv1pnv2 = v1.length() * v2.length();
      double inv_nv1pnv2 = 1.0 / nv1pnv2;
      double cosVal = QVector3D::dotProduct(v1, v2) * inv_nv1pnv2;
      double angle = acos(cosVal);
      sum += angle;

      isPlanar &= fabs(QVector3D::dotProduct(normal, v1)) < THRES;

      he = curHE;
      curHE = he->flip->next;
    }while( he != v->he ) ;

    cout << (isPlanar?"planar":"non-planar") << endl;
    /// either sums up to rought 2 * Pi, or has a cut face connected.
    return fabs(sum - PI2) > THRES || (!isPlanar);
  };

  set<HDS_HalfEdge*> cutEdges;
  set<HDS_Vertex*> reachedVertex;

  for(auto &v : mesh->vertSet) {
    /// if this edge is connected to a non-planar vertex, cut this edge,
    /// and form a cut edge tree starting from this edge
    if( isBadVertex(v) && reachedVertex.find(v) == reachedVertex.end() ) {
      /// add cut edges all the way until no more bad vertex could be reached
      queue<HDS_Vertex*> Q;
      Q.push(v);
      reachedVertex.insert(v);
      while(!Q.empty()) {
        auto cur = Q.front();
        Q.pop();

        cout << "reached vertex " << cur->index << endl;
        /// for all its bad vertex neighbor, add cut edges
        auto he = cur->he;
        do {
          auto &vi = he->flip->v;
          cout << "got " << vi->index << endl;
          if( isBadVertex(vi) && reachedVertex.find(vi) == reachedVertex.end() ) {
            cout << "pushing in " << vi->index << endl;
            Q.push(vi);
            reachedVertex.insert(vi);
            cutEdges.insert(he);
            he->setCutEdge(true);
          }
          he = he->flip->next;
        } while( he != cur->he );
      }
    }
  }
  cout << "#cut edges = " << cutEdges.size() << endl;
  return cutEdges;
}
