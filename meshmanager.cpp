#include "meshmanager.h"
#include "meshcutter.h"
#include "meshunfolder.h"
#include "meshsmoother.h"
#include "MeshExtender.h"
#include "MeshIterator.h"

#include "utils.hpp"

#include <vtkPolyDataToReebGraphFilter.h>
#include <vtkDirectedGraph.h>
#include <vtkReebGraph.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>
#include <vtkVertexListIterator.h>
#include <vtkEdgeListIterator.h>
#include <vtkGraphEdge.h>
#include <vtkDataSetAttributes.h>
#include <vtkVariantArray.h>

MeshManager* MeshManager::instance = NULL;

bool MeshManager::loadOBJFile(const string& filename) {
  try {
    cout << "[VTK] Reading mesh file ..." << endl;
    vtkSmartPointer<vtkOBJReader> vtkReader = vtkSmartPointer<vtkOBJReader>::New();
    vtkReader->SetFileName(filename.c_str());
    vtkReader->Update();
    vtkMesh = vtkReader->GetOutput();
    cout << "[VTK] Creating reeb graph ..." << endl;

    updateReebGraph();
  }
  catch (exception e) {
    cerr << e.what() << endl;
  }

  OBJLoader loader;
  if( loader.load(filename) ) {
    cout << "file " << filename << " loaded." << endl;

    /// build a half edge mesh here
    //hds_mesh->printMesh("original");
    hds_mesh.reset(buildHalfEdgeMesh(loader.getFaces(), loader.getVerts()));

    cutted_mesh.reset();
    unfolded_mesh.reset();
    smoothed_mesh.reset();

    /// save the half edge mesh out to a temporary file
    hds_mesh->save("temp.obj");
    
    /// preprocess the mesh with smoothing
    const int nsmooth = 10;
    QScopedPointer<HDS_Mesh> tmp_mesh;
    vector<string> smoothed_mesh_filenames;
    tmp_mesh.reset(new HDS_Mesh(*hds_mesh));
    hds_mesh_smoothed.push_back(QSharedPointer<HDS_Mesh>(new HDS_Mesh(*tmp_mesh)));
    for (int i = 0; i < nsmooth; ++i) {
      const int stepsize = 10;
      string smesh_filename = filename.substr(0, filename.length() - 4) + "_smoothed_" + std::to_string((i + 1)*stepsize) + ".obj";
      smoothed_mesh_filenames.push_back(smesh_filename);
      
      if (Utils::exists(smesh_filename)) {
        // load the mesh directly
        OBJLoader tmploader;
        tmploader.load(smesh_filename);
        tmp_mesh.reset(buildHalfEdgeMesh(tmploader.getFaces(), tmploader.getVerts()));
      }
      else {
        for (int j = 0; j < stepsize; ++j) {
          MeshSmoother::smoothMesh_Laplacian(tmp_mesh.data());
        }
        tmp_mesh->save(smesh_filename);
      }
      hds_mesh_smoothed.push_back(QSharedPointer<HDS_Mesh>(new HDS_Mesh(*tmp_mesh)));
    }
    cout << "smoothed meshes computed finished." << endl;

    /// initialize the sparse graph
    gcomp.reset(new GeodesicComputer(filename));
    gcomp_smoothed.push_back(QSharedPointer<GeodesicComputer>(gcomp.data()));
    for (int i = 0; i < smoothed_mesh_filenames.size(); ++i) {
      // compute or load SVG for smoothed meshes
      gcomp_smoothed.push_back(QSharedPointer<GeodesicComputer>(new GeodesicComputer(smoothed_mesh_filenames[i])));
    }
    cout << "SVGs computed." << endl;
    return true;
  }
  else return false;
}

HDS_Mesh* MeshManager::buildHalfEdgeMesh(const vector<MeshLoader::face_t> &inFaces,
                                    const vector<MeshLoader::vert_t> &inVerts) {
  mesh_t *thismesh = new mesh_t;

  cout << "building the half edge mesh ..." << endl;

  size_t vertsCount = inVerts.size();
  size_t facesCount = inFaces.size();
  size_t curFaceCount = facesCount;
  size_t heCount = 0;

  vector<vert_t*> verts;
  vector<face_t*> faces;
  vector<he_t*> hes;

  verts.resize(vertsCount);
  faces.resize(facesCount);

  for(size_t i=0;i<inFaces.size();i++)
    heCount += inFaces[i].v.size();

  hes.resize(heCount);

  for(size_t i=0;i<vertsCount;i++)
  {
    verts[i] = new vert_t(inVerts[i]);
  }

  for(size_t i=0;i<facesCount;i++)
  {
    faces[i] = new face_t;
  }

  map<pair<int, int>, he_t*> heMap;
  heMap.clear();

  for(size_t i=0, heIdx = 0;i<facesCount;i++)
  {
    auto& Fi = inFaces[i];
    face_t* curFace = faces[i];

    for(size_t j=0;j<Fi.v.size();j++)
    {
      he_t* curHe = new he_t;;
      vert_t* curVert = verts[Fi.v[j]];
      curHe->v = curVert;
      curHe->f = curFace;

      if( curVert->he == nullptr )
        curVert->he = curHe;

      hes[heIdx + j] = curHe;
    }

    // link the half edge of the face
    for(int j=0;j<Fi.v.size();j++)
    {
      int jp = j-1;
      if( jp < 0 ) jp += Fi.v.size();
      int jn = j+1;
      if( jn >= Fi.v.size() ) jn -= Fi.v.size();

      hes[heIdx+j]->prev = hes[heIdx+jp];
      hes[heIdx+j]->next = hes[heIdx+jn];

      int vj = Fi.v[j];
      int vjn = Fi.v[jn];

      pair<int, int> vPair = make_pair(vj, vjn);

      if( heMap.find(vPair) == heMap.end() )
      {
        heMap[vPair] = hes[heIdx+j];
      }
    }

    curFace->he = hes[heIdx];
    curFace->computeNormal();

    heIdx += Fi.v.size();
  }

  set<pair<int, int> > pairedHESet;

  // for each half edge, find its flip
  for(auto heit = heMap.begin();heit!=heMap.end();heit++)
  {
    int from, to;

    pair<int, int> hePair = (*heit).first;

    if( pairedHESet.find(hePair) == pairedHESet.end() )
    {

      from = hePair.first;
      to = hePair.second;
      pair<int, int> invPair = make_pair(to, from);

      auto invItem = heMap.find(invPair);

      if( invItem != heMap.end() )
      {
        he_t* he = (*heit).second;
        he_t* hef = (*invItem).second;

        he->flip = hef;
        hef->flip = he;
      }

      pairedHESet.insert( hePair );
      pairedHESet.insert( invPair );
    }
  }

  for( auto &v : verts ) {
    v->computeCurvature();
	v->computeNormal();
  }

  thismesh->setMesh(faces, verts, hes);
  cout << "finished building halfedge structure." << endl;
  cout << "halfedge count = " << thismesh->halfedges().size() << endl;

  return thismesh;
}

void MeshManager::cutMeshWithSelectedEdges()
{
  QScopedPointer<HDS_Mesh> ref_mesh;
  ref_mesh.reset(new HDS_Mesh(*hds_mesh));

  cout << "validating reference mesh" << endl;
  ref_mesh->validate();

  /// cut the mesh using the selected edges
  set<int> selectedEdges;
  for(auto he : ref_mesh->halfedges()) {
    if( he->isPicked ) {
      /// use picked edges as cut edges
      he->setPicked(false);
      he->setCutEdge(true);

      if( selectedEdges.find(he->index) == selectedEdges.end() &&
          selectedEdges.find(he->flip->index) == selectedEdges.end() ) {
        selectedEdges.insert(he->index);
      }
    }
  }
  cout << "Number of selected edges = " << selectedEdges.size() << endl;

  bool isUnfoldable = false;
  while( !isUnfoldable ) {
    /// make a copy of the mesh with selected edges
    cutted_mesh.reset(new HDS_Mesh(*ref_mesh));

    bool cutSucceeded = MeshCutter::cutMeshUsingEdges(cutted_mesh.data(), selectedEdges);
    if( cutSucceeded ) {
      /// cutting performed successfully
      cutted_mesh->printInfo("cutted mesh:");
      //cutted_mesh->printMesh("cutted mesh:");
    }
    else {
      /// can not cut it
    }

    isUnfoldable = true;// MeshUnfolder::unfoldable(cutted_mesh.data());
    /// replace the ref mesh with the cutted mesh
    ref_mesh.reset(new HDS_Mesh(*cutted_mesh));
    /// discard the selected edges now
    selectedEdges.clear();
  }
}

void MeshManager::unfoldMesh() {
  HDS_Mesh* ref_mesh;

  if (extended_mesh.isNull())
    ref_mesh = cutted_mesh.data();    
  else
    ref_mesh = extended_mesh.data();

  unfolded_mesh.reset(new HDS_Mesh(*ref_mesh));

  /// cut the mesh using the selected edges
  set<int> selectedFaces;
  for (auto f : ref_mesh->faces()) {
    if( f->isPicked ) {
      /// use picked edges as cut edges
      f->setPicked(false);

      if( selectedFaces.find(f->index) == selectedFaces.end() ) {
        selectedFaces.insert(f->index);
      }
    }
  }

  if (MeshUnfolder::unfold(unfolded_mesh.data(), ref_mesh, selectedFaces)) {
    /// unfolded successfully
    unfolded_mesh->printInfo("unfolded mesh:");
    //unfolded_mesh->printMesh("unfolded mesh:");
  }
  else {
    /// failed to unfold
    cout << "Failed to unfold." << endl;
  }

}

void MeshManager::smoothMesh() {
  if( smoothed_mesh.isNull() )
    smoothed_mesh.reset(new HDS_Mesh(*hds_mesh));
  
  //MeshSmoother::smoothMesh(smoothed_mesh.data());
  //MeshSmoother::smoothMesh_perVertex(smoothed_mesh.data());
  MeshSmoother::smoothMesh_Laplacian(smoothed_mesh.data());
}

bool MeshManager::saveMeshes() {
  /// save only the cutted and unfolded
  return true;
}

void MeshManager::extendMesh()
{
  if ( cutted_mesh.isNull() )
    extended_mesh.reset(new HDS_Mesh(*hds_mesh));
  else
    extended_mesh.reset(new HDS_Mesh(*cutted_mesh));
  MeshExtender::extendMesh(extended_mesh.data(), 0.75);
}

void MeshManager::colorMeshByGeoDistance(int vidx)
{
  auto laplacianSmoother = [&](const vector<double> &val, HDS_Mesh *mesh) {
    const double lambda = 0.25;
    const double sigma = 1.0;
    unordered_map<HDS_Vertex*, double> L(mesh->verts().size());
    vector<double> newval(mesh->verts().size());
    for (auto vi : mesh->verts()) {
      auto neighbors = vi->neighbors();

      double denom = 0.0;
      double numer = 0.0;

      for (auto vj : neighbors) {
        //double wij = 1.0 / (vi->pos.distanceToPoint(vj->pos) + sigma);
        double wij = 1.0 / neighbors.size();
        denom += wij;
        numer += wij * val[vj->index];
      }

      L.insert(make_pair(vi, numer / denom - val[vi->index]));
  }
    for (auto p : L) {
      newval[p.first->index] = val[p.first->index] + lambda * p.second;
    }

    return newval;
  };
#if 1
  auto dists = gcomp->distanceTo(vidx);
  int niters = 100;
  for (int i = 0; i < niters; ++i)
    dists = laplacianSmoother(dists, hds_mesh.data());
#else
  auto Q = MeshIterator::BFS(hds_mesh.data(), vidx);
  vector<double> dists(hds_mesh->verts().size());
  while (!Q.empty()){
    auto cur = Q.front();
    Q.pop();
    dists[cur.first->index] = cur.second;
  }
#endif

  // save it to a file
  ofstream fout("geodist.txt");
  for (auto x : dists) {
    fout << x << endl;
  }
  fout.close();

  double maxDist = *(std::max_element(dists.begin(), dists.end()));
  std::for_each(dists.begin(), dists.end(), [=](double &x){
    x /= maxDist;
    x -= 0.5;
    //cout << x << endl;
  });
  hds_mesh->colorVertices(dists);
}

void MeshManager::colorMeshByGeoDistance(int vidx, int lev0, int lev1, double ratio)
{
  auto dists = getInterpolatedGeodesics(vidx, lev0, lev1, ratio);
  double maxDist = *(std::max_element(dists.begin(), dists.end()));
  std::for_each(dists.begin(), dists.end(), [=](double &x){
    x /= maxDist;
    x -= 0.5;
    //cout << x << endl;
  });
  hds_mesh->colorVertices(dists);
}

void MeshManager::updateReebGraph(const vector<double> &fvals)
{
  vtkSmartPointer<vtkReebGraph> surfaceReebGraph = vtkSmartPointer<vtkReebGraph>::New();
  int nverts = vtkMesh->GetNumberOfPoints();
  cout << "number of points = " << vtkMesh->GetNumberOfPoints() << endl;
  cout << "[VTK] Preparing scalar data ..." << endl;
  vtkSmartPointer<vtkDoubleArray> scalarField = vtkSmartPointer<vtkDoubleArray>::New();
  scalarField->Resize(nverts);
  if (fvals.empty() || fvals.size() != nverts) {
    for (int i = 0; i < nverts; ++i) {
      auto pt = vtkMesh->GetPoint(i);
      scalarField->SetValue(i, pt[2]);
    }
  }
  else {
    for (int i = 0; i < nverts; ++i) {
      scalarField->SetValue(i, fvals[i]);
    }
  }

  cout << "[VTK] Building reeb graph ..." << endl;
  int ret = surfaceReebGraph->Build(vtkMesh, scalarField);
  switch (ret) {
  case vtkReebGraph::ERR_INCORRECT_FIELD:
    cout << "[VTK] ERR_INCORRECT_FIELD" << endl;
    break;
  case vtkReebGraph::ERR_NOT_A_SIMPLICIAL_MESH:
    cout << "[VTK] ERR NOT A SIMPLICIAL MESH" << endl;
    break;
  default:
    cout << "[VTK] Succeeded." << endl;
    break;
  }
  surfaceReebGraph->Print(std::cout);
  
  rbGraph.clear();

  // print the information of this graph
  vtkSmartPointer<vtkVertexListIterator> it = vtkVertexListIterator::New();
  surfaceReebGraph->GetVertices(it);
  while (it->HasNext()) {
    auto vidx = it->Next();
    auto vdata = surfaceReebGraph->GetVertexData();
    auto refidx = vdata->GetArray("Vertex Ids")->GetComponent(vidx, 0);
    cout << refidx << endl;
    rbGraph.V.push_back(SimpleNode(vidx, refidx));
    auto ptdata = vtkMesh->GetPoint(refidx);
    cout << ptdata[0] << ", " << ptdata[1] << ", " << ptdata[2] << endl;
  }

  vtkSmartPointer<vtkEdgeListIterator> eit = vtkEdgeListIterator::New();
  surfaceReebGraph->GetEdges(eit);
  while (eit->HasNext()) {
    auto edge = eit->NextGraphEdge();
    rbGraph.E.push_back(SimpleEdge(edge->GetSource(), edge->GetTarget()));
    cout << edge->GetSource() << " -> " << edge->GetTarget() << endl;
  }

  vtkDataArray *vertexInfo = vtkDataArray::SafeDownCast(surfaceReebGraph->GetVertexData()->GetAbstractArray("Vertex Ids"));
  vtkVariantArray *edgeInfo = vtkVariantArray::SafeDownCast(surfaceReebGraph->GetEdgeData()->GetAbstractArray("Vertex Ids"));
  surfaceReebGraph->GetEdges(eit);
  while (eit->HasNext()) {
    vtkEdgeType e = eit->Next();
    vtkAbstractArray *deg2NodeList = edgeInfo->GetPointer(e.Id)->ToArray();
    cout << "     Arc #" << e.Id << ": "
      //<< *(vertexInfo->GetTuple(e.Source)) 
      << vertexInfo->GetComponent(e.Source, 0)
      << " -> "
      //<< *(vertexInfo->GetTuple(e.Target)) 
      << vertexInfo->GetComponent(e.Target, 0)
      << " (" << deg2NodeList->GetNumberOfTuples() << " degree-2 nodes)" << endl;

    int ntuples = deg2NodeList->GetNumberOfTuples();
    if (ntuples > 0) {
      rbGraph.Es.push_back(SimpleEdge(vertexInfo->GetComponent(e.Source, 0), deg2NodeList->GetVariantValue(0).ToInt()));
      for (int j = 0; j < ntuples - 1; ++j) {
        rbGraph.Es.push_back(SimpleEdge(deg2NodeList->GetVariantValue(j).ToInt(), deg2NodeList->GetVariantValue(j + 1).ToInt()));
      }
      rbGraph.Es.push_back(SimpleEdge(deg2NodeList->GetVariantValue(ntuples - 1).ToInt(), vertexInfo->GetComponent(e.Target, 0)));
    }
    else {
      rbGraph.Es.push_back(SimpleEdge(vertexInfo->GetComponent(e.Source, 0), vertexInfo->GetComponent(e.Target, 0)));
    }
  }
}

vector<double> MeshManager::getInterpolatedGeodesics(int vidx, int lev0, int lev1, double alpha)
{
  auto dist0 = gcomp_smoothed[lev0]->distanceTo(vidx);
  auto dist1 = gcomp_smoothed[lev1]->distanceTo(vidx);
  return Utils::interpolate(dist0, dist1, alpha);
}


vector<double> MeshManager::getInterpolatedZValue(int lev0, int lev1, double alpha)
{
  auto m0 = hds_mesh_smoothed[lev0];
  auto m1 = hds_mesh_smoothed[lev1];

  auto dist0 = vector<double>(hds_mesh->verts().size());
  auto dist1 = vector<double>(hds_mesh->verts().size());
  for (auto v : m0->verts()) {
    dist0[v->index] = v->pos.z();
  }
  for (auto v : m1->verts()) {
    dist1[v->index] = v->pos.z();
  }
  return Utils::interpolate(dist0, dist1, alpha);
}

vector<double> MeshManager::getInterpolatedPointNormalValue(int lev0, int lev1, double alpha, const QVector3D &pnormal)
{
  auto m0 = hds_mesh_smoothed[lev0];
  auto m1 = hds_mesh_smoothed[lev1];

  auto dist0 = vector<double>(hds_mesh->verts().size());
  auto dist1 = vector<double>(hds_mesh->verts().size());
  for (auto v : m0->verts()) {
    dist0[v->index] = QVector3D::dotProduct(v->pos, pnormal);
  }
  for (auto v : m1->verts()) {
    dist1[v->index] = QVector3D::dotProduct(v->pos, pnormal);
  }
  return Utils::interpolate(dist0, dist1, alpha);
}

vector<double> MeshManager::getInterpolatedCurvature(int lev0, int lev1, double alpha)
{
  auto m0 = hds_mesh_smoothed[lev0];
  auto m1 = hds_mesh_smoothed[lev1];

  auto dist0 = vector<double>(hds_mesh->verts().size());
  auto dist1 = vector<double>(hds_mesh->verts().size());
  for (auto v : m0->verts()) {
    dist0[v->index] = v->curvature;
  }
  for (auto v : m1->verts()) {
    dist1[v->index] = v->curvature;
  }
  return Utils::interpolate(dist0, dist1, alpha);
}

