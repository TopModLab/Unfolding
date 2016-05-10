#include "meshmanager.h"
#include "meshcutter.h"
#include "meshunfolder.h"
#include "meshsmoother.h"
#include "MeshExtender.h"
#include "meshhollower.h"
#include "meshrimface.h"
#include "MeshWeaver.h"
#include "MeshIterator.h"
#include "MeshConnector.h"

#include "utils.hpp"

#if USE_REEB_GRAPH
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
#endif

MeshManager* MeshManager::instance = nullptr;

doubles_t MeshManager::getInterpolatedGeodesics(int vidx, int lev0, int lev1, double alpha)
{
	auto dist0 = gcomp_smoothed[lev0]->distanceTo(vidx);
	auto dist1 = gcomp_smoothed[lev1]->distanceTo(vidx);
	return Utils::interpolate(dist0, dist1, alpha);
}


doubles_t MeshManager::getInterpolatedZValue(int lev0, int lev1, double alpha)
{
	auto m0 = hds_mesh_smoothed[lev0];
	auto m1 = hds_mesh_smoothed[lev1];

	HDS_Mesh* hds_mesh = operationStack->getOriMesh();

	auto dist0 = doubles_t(hds_mesh->verts().size());
	auto dist1 = doubles_t(hds_mesh->verts().size());
	for (auto v : m0->verts()) {
		dist0[v->index] = v->pos.z();
	}
	for (auto v : m1->verts()) {
		dist1[v->index] = v->pos.z();
	}
	return Utils::interpolate(dist0, dist1, alpha);
}

doubles_t MeshManager::getInterpolatedPointNormalValue(int lev0, int lev1, double alpha, const QVector3D &pnormal)
{
	auto m0 = hds_mesh_smoothed[lev0];
	auto m1 = hds_mesh_smoothed[lev1];

	HDS_Mesh* hds_mesh = operationStack->getOriMesh();

	auto dist0 = doubles_t(hds_mesh->verts().size());
	auto dist1 = doubles_t(hds_mesh->verts().size());
	for (auto v : m0->verts()) {
		dist0[v->index] = QVector3D::dotProduct(v->pos, pnormal);
	}
	for (auto v : m1->verts()) {
		dist1[v->index] = QVector3D::dotProduct(v->pos, pnormal);
	}
	return Utils::interpolate(dist0, dist1, alpha);
}

doubles_t MeshManager::getInterpolatedCurvature(int lev0, int lev1, double alpha)
{
	auto m0 = hds_mesh_smoothed[lev0];
	auto m1 = hds_mesh_smoothed[lev1];

	HDS_Mesh* hds_mesh = operationStack->getOriMesh();
	auto dist0 = doubles_t(hds_mesh->verts().size());
	auto dist1 = doubles_t(hds_mesh->verts().size());
	for (auto v : m0->verts()) {
		dist0[v->index] = v->curvature;
	}
	for (auto v : m1->verts()) {
		dist1[v->index] = v->curvature;
	}
	return Utils::interpolate(dist0, dist1, alpha);
}


bool MeshManager::loadOBJFile(const string &filename) {
#if USE_REEB_GRAPH
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
#endif

	//OBJLoader loader;
#ifdef _DEBUG
	QTime clock;
	clock.start();
#endif
	// Check if file is successfully loaded
	if(meshloader->load(filename))
	{
		cout << "file " << filename << " loaded." << endl;

		QProgressDialog loadingProgress("Loading the object...", "", 0, 100);
		loadingProgress.setWindowModality(Qt::WindowModal);
		loadingProgress.setValue(0);
		loadingProgress.setAutoClose(true);
		loadingProgress.setCancelButton(0);
		loadingProgress.setMinimumDuration(1000);

		/// build a half edge mesh here
		mesh_t* msh = buildHalfEdgeMesh(meshloader->getVerts(), meshloader->getFaces());
		if (msh == nullptr)
		{
			loadingProgress.close();
			return false;
		}
		operationStack->push(msh);
#ifdef _DEBUG
		qDebug("Clear Operation Takes %d ms In Total.", clock.elapsed());
		clock.restart();
#endif
		initSparseGraph();
//////////////////////////////////////////////////////////////////////////
		loadingProgress.setValue(100);

		return true;
	}
	else
	{
		return false;
	}
}
HDS_Mesh * MeshManager::buildHalfEdgeMesh(
	const doubles_t &inVerts, const vector<PolyIndex*> &inFaces)
{
#ifdef _DEBUG
	cout << "Building the half edge mesh ..." << endl;
#endif // _DEBUG

	mesh_t *thismesh = new mesh_t;
	int ss = 0;
	size_t vertsCount = inVerts.size() / 3;
	size_t facesCount = inFaces.size();
	size_t curFaceCount = facesCount;
	size_t heCount = 0;
	for (size_t i = 0; i < inFaces.size(); i++)
		heCount += inFaces[i]->size;

	// Half-Edge Data in Mesh
	vector<vert_t*> verts(vertsCount, nullptr);
	vector<face_t*> faces(facesCount, nullptr);
	vector<he_t*> hes(heCount, nullptr);
	// Temporary Half-Edge Pair Recorder
	using hepair_t = pair<hdsid_t, hdsid_t>;
	map<hepair_t, he_t*> heMap;

	// Malloc Vertices
	for (size_t i = 0; i < vertsCount; i++)
	{
		size_t vid = i * 3;
		verts[i] = new vert_t(QVector3D(
					static_cast<float>(inVerts[vid]),
					static_cast<float>(inVerts[vid + 1]),
					static_cast<float>(inVerts[vid + 2])));
	}
	// Malloc Faces
	for (size_t i = 0; i < facesCount; i++)
	{
		faces[i] = new face_t;
	}

	for (size_t i = 0, heIdx = 0; i < facesCount; i++)
	{

		auto& Fi = inFaces[i];
		face_t* curFace = faces[i];

		for (size_t j = 0; j < Fi->size; j++)
		{
			he_t* curHe = new he_t;
			vert_t* curVert = verts[Fi->v[j]];
			curHe->v = curVert;
			curHe->f = curFace;

			if (curVert->he == nullptr)
				curVert->he = curHe;

			hes[heIdx + j] = curHe;
		}

		// link the half edge of the face
		for (int j = 0; j < Fi->size; j++)
		{
			int jp = j - 1;
			if (jp < 0) jp += Fi->size;
			int jn = j + 1;
			if (jn >= Fi->size) jn -= Fi->size;

			hes[heIdx + j]->prev = hes[heIdx + jp];
			hes[heIdx + j]->next = hes[heIdx + jn];

			int vj = Fi->v[j];
			//     cout<<"j = "<<j<<"  Fi.v[j] = "<<Fi.v[j]<<endl;//later added;
			int vjn = Fi->v[jn];
			//      cout<<"jn = "<<jn<<"  Fi.v[jn] = "<<Fi.v[jn]<<endl;//later added;
			pair<int, int> vPair = make_pair(vj, vjn);
			//cout<<"vPair.first = "<<vPair.first<<endl;  //later added;
			//cout<<"vPair.sencond = "<<vPair.second<<endl;
			if (heMap.find(vPair) == heMap.end())
			//?? true every time;for heMap.end() points to he_t*, equal to heMap.find(vpair);
			{
				heMap[vPair] = hes[heIdx + j];
				//address transport; hes[heIdx + j] = curHe; *****critical******
				//      cout<<"heshes[heIdx+j]"<<hes[heIdx+j]<<endl; //later added;
				//      ss+=1;  //later added;
			}
			else
			{
				return thismesh;
			}
		}
		//    cout<<"ss = "<<ss<<endl;    //later added;
		curFace->he = hes[heIdx];
		curFace->computeNormal();

		heIdx += Fi->size;
	}
	set<hepair_t> visitedHESet;
	auto unvisitedHESet = heMap;
	// for each half edge, find its flip
	for (auto heit : heMap)
	{
		int from, to;

		hepair_t hePair = heit.first;

		if (visitedHESet.find(hePair) == visitedHESet.end())
		{

			from = hePair.first;
			to = hePair.second;
			hepair_t invPair = make_pair(to, from);

			auto invItem = heMap.find(invPair);

			if (invItem != heMap.end())
			{
				he_t* he = heit.second;
				he_t* hef = invItem->second;

				he->flip = hef;
				hef->flip = he;
				unvisitedHESet.erase(hePair);
				unvisitedHESet.erase(invPair);
			}

			visitedHESet.insert(hePair);
			visitedHESet.insert(invPair);
		}
	}
	// Check Holes and Fill with Null Faces
	if (unvisitedHESet.size() > 0)
	{
		QMessageBox msgBox(QMessageBox::Warning, QString("Warning"),
			QString("Current mesh has holes and can cause critical problems!\n"
				"Do you still want to load it?"),
			QMessageBox::Yes | QMessageBox::Cancel);
		msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
		if (msgBox.exec() == QMessageBox::Cancel)
		{
			for (auto v : verts)
			{
				delete v;
			}
			for (auto f : faces)
			{
				delete f;
			}
			for (auto he : hes)
			{
				delete he;
			}
			delete thismesh;
			return nullptr;
		}
		while (unvisitedHESet.size() > 0)
		{
			auto heit = unvisitedHESet.begin();
			he_t* he = heit->second;
			unvisitedHESet.erase(heit);
			// Skip checked edges, won't skip in first check
			if (he->flip != nullptr) continue;
			//he_t* hef = new he_t;
			face_t* nullface = new face_t;

			auto curHE = he;
			vector<he_t*> null_hes, null_hefs;
			do 
			{
				null_hes.push_back(curHE);
				null_hefs.push_back(new he_t);
				
				// Loop adjacent edges to find the exposed edge
				while (curHE->flip != nullptr)
				{
					curHE = curHE->flip->next;
				}
			} while (curHE != he);
			// Assign Null Edges
			auto size = null_hes.size();
			for (int i = 0; i < size; i++)
			{
				he = null_hes[i];
				he_t* hef = null_hefs[i];
				he->isCutEdge = hef->isCutEdge = true;
				he->flip = hef;
				hef->flip = he;
				hef->v = he->next->v;
				hef->f = nullface;

				he_t* hef_prev = null_hefs[(i + 1) % size];
				hef->prev = hef_prev;
				hef_prev->next = hef;
			}
			// Insert Null Edges
			hes.insert(hes.end(), null_hefs.begin(), null_hefs.end());
			// Assign Null Face
			nullface->isCutFace = true;
			nullface->he = he;
			nullface->he = null_hefs[0];
			faces.push_back(nullface);
		}
	}

	for (auto &v : verts) {
		v->computeCurvature();
		v->computeNormal();
	}
	int negCount = 0;
	for (auto &he : hes) {
		he->computeCurvature();
		if (he->isNegCurve) negCount++;
	}
	thismesh->setMesh(faces, verts, hes);

#ifdef _DEBUG
	cout << "\tNegative Edge Count ::" << negCount / 2.0 << endl;
	cout << "\tFinished Building Half-Edge Structure." << endl;
	cout << "\tHalf-Edge Count = " << thismesh->halfedges().size() << endl;
#endif

	return thismesh;
}

void MeshManager::cutMeshWithSelectedEdges()
{
	HDS_Mesh* inMesh = operationStack->getCurrentMesh();

	QScopedPointer<HDS_Mesh> ref_mesh;
		ref_mesh.reset(new HDS_Mesh(*inMesh));


	//cout << "validating reference mesh" << endl;
	//ref_mesh->validate();

	/// cut the mesh using the selected edges
	set<int> selectedEdges;
	for(auto he : ref_mesh->halfedges())
	{
		if( he->isPicked )
		{
			/// use picked edges as cut edges
			he->setPicked(false);
			he->setCutEdge(true);

			if( selectedEdges.find(he->index) == selectedEdges.end() &&
					selectedEdges.find(he->flip->index) == selectedEdges.end() )
			{
				selectedEdges.insert(he->index);
			}
		}
	}

	//cout << "Number of selected edges = " << selectedEdges.size() << endl;

	bool isUnfoldable = false;
	HDS_Mesh* outMesh = ref_mesh.take();

	while( !isUnfoldable )
	{
		if(MeshCutter::cutMeshUsingEdges(outMesh, selectedEdges))
		{
			/// cutting performed successfully
			//cutted_mesh->printInfo("cutted mesh:");
			//cutted_mesh->printMesh("cutted mesh:");
			cout<<"cut succeed!"<<endl;
		}
		else
		{
			/// can not cut it
		}

		isUnfoldable = true;// MeshUnfolder::unfoldable(cutted_mesh.data());
		/// replace the ref mesh with the cutted mesh
		/// commented out due to bug when cutting all edges
		//ref_mesh.reset(new HDS_Mesh(*cutted_mesh));
		//cout<<"ref_mesh reset"<<endl;

		/// discard the selected edges now
		selectedEdges.clear();
	}
	operationStack->push(outMesh);
	cout << ".........................." << endl;
}

bool MeshManager::initSparseGraph()
{
	QProgressDialog loadingProgress("Loading the object...", "", 0, 100);
	loadingProgress.setWindowModality(Qt::WindowModal);
	loadingProgress.setValue(0);
	loadingProgress.setAutoClose(true);
	loadingProgress.setCancelButton(0);
	loadingProgress.setMinimumDuration(1000);
#ifdef _DEBUG
	QTime clock;
	clock.start();
#endif
	// save the half edge mesh out to a temporary file
	loadingProgress.setValue(30);

	HDS_Mesh* hds_mesh = operationStack->getOriMesh();
	auto filename = meshloader->getFilename();

	// preprocess the mesh with smoothing
	const int nsmooth = 10;
	QScopedPointer<HDS_Mesh> tmp_mesh;
	vector<string> smoothed_mesh_filenames;
	tmp_mesh.reset(new HDS_Mesh(*hds_mesh));
	hds_mesh_smoothed.push_back(QSharedPointer<HDS_Mesh>(new HDS_Mesh(*tmp_mesh)));
	for (int i = 0; i < nsmooth; ++i)
	{
		loadingProgress.setValue(30 + (double)i / (double)nsmooth * 50);

		const int stepsize = 10;
		string smesh_filename = filename.substr(0, filename.length() - 4) + "_smoothed_" + std::to_string((i + 1)*stepsize) + ".obj";
		smoothed_mesh_filenames.push_back(smesh_filename);
		//cout<<" smesh_filename is "<< smesh_filename<<endl;

		if (Utils::exists(smesh_filename)) {
			// load the mesh directly
			OBJLoader tmploader;
			tmploader.load(smesh_filename);
			tmp_mesh.reset(buildHalfEdgeMesh(tmploader.getVerts(), tmploader.getFaces()));
			//  cout<<"load mesh directly"<<endl;
		}
		else {
			for (int j = 0; j < stepsize; ++j) {
				MeshSmoother::smoothMesh_Laplacian(tmp_mesh.data());
			}
			//tmp_mesh->save(smesh_filename); //commented out exporting smoothes objs
		}
		hds_mesh_smoothed.push_back(QSharedPointer<HDS_Mesh>(new HDS_Mesh(*tmp_mesh)));


	}
	cout << "Smoothed Mesh Filename:\t" << smoothed_mesh_filenames.size() << endl;
	loadingProgress.setValue(80);
	//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
	cout << "smoothed meshes computed finished." << endl;
	qDebug("Smoothing Mesh Takes %d ms In Total.", clock.elapsed());
	clock.restart();
#endif
	// initialize the sparse graph
	if (hds_mesh->verts().size()>10)
	{
		ui32s_t* triFids = meshloader->getTriangulatedIndices();
		gcomp.reset(new GeodesicComputer(filename, &meshloader->getVerts(), triFids));
		delete triFids;
		gcomp_smoothed.push_back(QSharedPointer<GeodesicComputer>(gcomp.data()));
		for (int i = 0; i < smoothed_mesh_filenames.size(); ++i) {
			// compute or load SVG for smoothed meshes
			//    gcomp_smoothed.push_back(QSharedPointer<GeodesicComputer>(new GeodesicComputer(smoothed_mesh_filenames[i])));//cancel this sentence, all became correct, what's it function?

			//cout<<"smoothed_mesh_filenames ["<<i<<"]  =  "<<smoothed_mesh_filenames[i]<<endl;
			loadingProgress.setValue(80 + (double)i / (double)smoothed_mesh_filenames.size() * 20);

		}
		cout << "SVGs computed." << endl;

		//set the graph for discrete geodesics computer
		dis_gcomp.reset(new DiscreteGeoComputer(hds_mesh));
		cout << "dis gcomp set." << endl;
	}
	else {
		loadingProgress.setValue(100);

		return true;
	}
	//*/
#ifdef _DEBUG
	qDebug("Sparsing Graph Takes %d ms In Total.", clock.elapsed());
	clock.restart();
#endif
	return false;
}

/*
void MeshManager::mapToExtendedMesh()
{
	QScopedPointer<HDS_Mesh> ref_mesh;
	QScopedPointer<HDS_Mesh> des_mesh;
	ref_mesh.reset(new HDS_Mesh(*cutted_mesh));
	des_mesh.reset(new HDS_Mesh(*extended_mesh));



	//mark out all cut edges

	//get cut edges in cutted mesh
	set<int> cutEdges;
	for(auto he : ref_mesh->halfedges()) {
		if (he->isCutEdge) {
			if( cutEdges.find(he->index) == cutEdges.end() )
			{
				cutEdges.insert(he->index);
			}
		}
	}

	//find original cut edges in extended mesh and mark out its face
	for(auto f : des_mesh->faces()) {
		if (f->isHole) {
			HDS_HalfEdge* edge = f->he;
			do {
				//edge->setPicked(true);
				edge->setCutEdge(true);
				edge = edge->next;
			}while(edge != f->he);
		}

		if (f->isBridger) {
			if (cutEdges.find(f->he->flip->index) != cutEdges.end()) {
				f->he->setPicked(true);

			}
		}
	}

	extended_mesh.reset(new HDS_Mesh(*des_mesh));

}*/

void MeshManager::unfoldMesh()
{
	HDS_Mesh* inMesh = operationStack->getCurrentMesh();

	QScopedPointer<HDS_Mesh> ref_mesh;


	ref_mesh.reset(new HDS_Mesh(*inMesh));
	ref_mesh->validate();
	cout << "unfolded mesh set" << endl;

	/// cut the mesh using the selected edges
	set<int> selectedFaces;
	for (auto f : ref_mesh->faces())
	{
		if (f->isPicked) {
			/// use picked edges as cut edges
			f->setPicked(false);
			if (selectedFaces.find(f->index) == selectedFaces.end()) {
				selectedFaces.insert(f->index);
			}
		}
	}
	HDS_Mesh* outMesh = new HDS_Mesh(*ref_mesh);

	if (MeshUnfolder::unfold(outMesh, ref_mesh.take(), selectedFaces)) {
		/// unfolded successfully
		outMesh->printInfo("unfolded mesh:");
		operationStack->push(outMesh);
		//unfolded_mesh->printMesh("unfolded mesh:");
	}
	else {
		/// failed to unfold
		cout << "Failed to unfold." << endl;
	}

}

/* legacy code, not sure what it's doing,
 * so didnt put the smoothed_mesh in operationStack*/
void MeshManager::smoothMesh()
{
	if( smoothed_mesh.isNull() )
	{
		HDS_Mesh* inMesh = operationStack->getCurrentMesh();
		cout<<"smoothmesh@@@"<<endl;
		smoothed_mesh.reset(new HDS_Mesh(*inMesh));
	}

	//MeshSmoother::smoothMesh(smoothed_mesh.data());
	//MeshSmoother::smoothMesh_perVertex(smoothed_mesh.data());
	MeshSmoother::smoothMesh_Laplacian(smoothed_mesh.data());
}


bool MeshManager::saveMeshes()
{
	/// save only the cutted and unfolded
	return true;
}


void MeshManager::extendMesh(map<QString, double> config)
{

	HDS_Bridger::setBridger(config);
	MeshExtender::setOriMesh(operationStack->getOriMesh());

	HDS_Mesh* inMesh = operationStack->getCurrentMesh();

	HDS_Mesh* outMesh = new HDS_Mesh(*inMesh);
	MeshExtender::extendMesh(outMesh);
	//update sorted faces
	outMesh->updateSortedFaces();
	operationStack->push(outMesh);

}

void MeshManager::rimMesh(double rimSize)
{
//	if (extended_mesh != nullptr)
//		extended_mesh->clearSortedFaces();

	HDS_Mesh* inMesh = operationStack->getCurrentMesh();

	QScopedPointer<HDS_Mesh> ref_mesh;
	ref_mesh.reset(new HDS_Mesh(*inMesh));

	// Select all edges to cut all faces
	set<int> selectedEdges;
	for (auto he : ref_mesh->halfedges())
	{
		if (selectedEdges.find(he->index) == selectedEdges.end() &&
			selectedEdges.find(he->flip->index) == selectedEdges.end())
		{
			selectedEdges.insert(he->index);
		}
	}

	bool rimSucceeded;
	/// make a copy of the mesh with selected edges
	HDS_Mesh* outMesh = new HDS_Mesh(*ref_mesh);
	rimSucceeded = MeshCutter::cutMeshUsingEdges(outMesh, selectedEdges);

	if (rimSucceeded)
	{
		//cutted_mesh->isHollowed
		outMesh->processType = HDS_Mesh::RIMMED_PROC;
		/// cutting performed successfully
		cout << "Rimming succeed!" << endl;
		operationStack->push(outMesh);
	}

	/// discard the selected edges now
	selectedEdges.clear();
}

void MeshManager::set3DRimMesh(std::map<QString,float> config)
{
    MeshRimFace::setOriMesh(operationStack->getOriMesh());
	HDS_Mesh* inMesh = operationStack->getCurrentMesh();

	HDS_Mesh* outMesh = new HDS_Mesh(*inMesh);
    MeshRimFace::configRimMesh(config);
	if (config["center"] == 0)
		MeshRimFace::rimMeshV(outMesh);
	else
		MeshRimFace::rimMeshF(outMesh);
	outMesh->updateSortedFaces();
	operationStack->push(outMesh);
}

void MeshManager::setBindMesh()
{
	setHollowMesh(1,2,0);
}

void MeshManager::setHollowMesh(double flapSize, int type, double shift)
{
	MeshHollower::setOriMesh(operationStack->getOriMesh());

	HDS_Mesh* inMesh = operationStack->getCurrentMesh();

	HDS_Mesh* outMesh = new HDS_Mesh(*inMesh);
	MeshHollower::hollowMesh(outMesh, flapSize, type, shift);
	outMesh->updateSortedFaces();
	operationStack->push(outMesh);

}

void MeshManager::setWeaveMesh(std::map<QString,float> config)
{
	MeshWeaver::setOriMesh(operationStack->getOriMesh());

	HDS_Mesh* inMesh = operationStack->getCurrentMesh();

	HDS_Mesh* outMesh = new HDS_Mesh(*inMesh);
	MeshWeaver::configWeaveMesh(config);
	MeshWeaver::weaveMesh(outMesh);
	outMesh->updateSortedFaces();
	operationStack->push(outMesh);
}

void MeshManager::exportXMLFile()
{
	if (true)// No connector generated
	{
		HDS_Mesh* unfolded_mesh = operationStack->getUnfoldedMesh();
		MeshConnector::generateConnector(unfolded_mesh);
	}
	
}

void MeshManager::colorMeshByGeoDistance(int vidx)
{
	auto laplacianSmoother = [&](const doubles_t &val, HDS_Mesh *mesh) {
		const double lambda = 0.25;
		const double sigma = 1.0;
		unordered_map<HDS_Vertex*, double> L(mesh->verts().size());
		doubles_t newval(mesh->verts().size());
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
		dists = laplacianSmoother(dists, operationStack->getOriMesh());
#else
	auto Q = MeshIterator::BFS(hds_mesh.data(), vidx);
	doubles_t dists(hds_mesh->verts().size());
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
	operationStack->getOriMesh()->colorVertices(dists);
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
	operationStack->getOriMesh()->colorVertices(dists);
}

#if USE_REEB_GRAPH
void MeshManager::updateReebGraph(const doubles_t &fvals)
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
#endif

