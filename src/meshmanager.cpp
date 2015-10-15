#include "meshmanager.h"
#include "meshcutter.h"
#include "meshunfolder.h"
#include "meshsmoother.h"
#include "MeshExtender.h"
#include "meshhollower.h"
#include "MeshIterator.h"

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

MeshManager* MeshManager::instance = NULL;

bool MeshManager::loadOBJFile(const string& filename) {
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

	OBJLoader loader;

	if( loader.load(filename) ) {
		cout << "file " << filename << " loaded." << endl;

		QProgressDialog* loadingProgress;
		loadingProgress = new QProgressDialog("Loading the object...", "", 0, 100);
		loadingProgress->setWindowModality(Qt::WindowModal);
		loadingProgress->setValue(0);
		loadingProgress->setAutoClose(true);
		loadingProgress->setCancelButton(0);
		loadingProgress->setMinimumDuration(1000);

		/// build a half edge mesh here
		//hds_mesh->printMesh("original");
		hds_mesh.reset(buildHalfEdgeMesh(loader.getFaces(), loader.getVerts()));

		cutted_mesh.reset();
		extended_mesh.reset();
		unfolded_mesh.reset();
		smoothed_mesh.reset();

		/// save the half edge mesh out to a temporary file
		hds_mesh->save("temp.obj");
		loadingProgress->setValue(30);

		/// preprocess the mesh with smoothing
		const int nsmooth = 10;
		QScopedPointer<HDS_Mesh> tmp_mesh;
		vector<string> smoothed_mesh_filenames;
		tmp_mesh.reset(new HDS_Mesh(*hds_mesh));
		hds_mesh_smoothed.push_back(QSharedPointer<HDS_Mesh>(new HDS_Mesh(*tmp_mesh)));
		for (int i = 0; i < nsmooth; ++i) {
			loadingProgress->setValue(30+(double)i/(double)nsmooth*50);

			const int stepsize = 10;
			string smesh_filename = filename.substr(0, filename.length() - 4) + "_smoothed_" + std::to_string((i + 1)*stepsize) + ".obj";
			smoothed_mesh_filenames.push_back(smesh_filename);
			//cout<<" smesh_filename is "<< smesh_filename<<endl;

			if (Utils::exists(smesh_filename)) {
				// load the mesh directly
				OBJLoader tmploader;
				tmploader.load(smesh_filename);
				tmp_mesh.reset(buildHalfEdgeMesh(tmploader.getFaces(), tmploader.getVerts()));
				//  cout<<"load mesh directly"<<endl;
			}
			else {
				for (int j = 0; j < stepsize; ++j) {
					MeshSmoother::smoothMesh_Laplacian(tmp_mesh.data());
					//      cout<<"smoothMesh_Laplacian"<<endl;
				}
				//tmp_mesh->save(smesh_filename); //commented out exporting smoothes objs
			}
			hds_mesh_smoothed.push_back(QSharedPointer<HDS_Mesh>(new HDS_Mesh(*tmp_mesh)));


		}
		cout << "smoothed meshes computed finished." << endl;
		loadingProgress->setValue(80);

		/// initialize the sparse graph
		if(hds_mesh->verts().size()>10){                         //later added;
			gcomp.reset(new GeodesicComputer(filename));
			gcomp_smoothed.push_back(QSharedPointer<GeodesicComputer>(gcomp.data()));
			for (int i = 0; i < smoothed_mesh_filenames.size(); ++i) {
				// compute or load SVG for smoothed meshes
				//    gcomp_smoothed.push_back(QSharedPointer<GeodesicComputer>(new GeodesicComputer(smoothed_mesh_filenames[i])));//cancel this sentence, all became correct, what's it function?

				//cout<<"smoothed_mesh_filenames ["<<i<<"]  =  "<<smoothed_mesh_filenames[i]<<endl;
				loadingProgress->setValue(80+(double)i/(double)smoothed_mesh_filenames.size()*20);

			}
			cout << "SVGs computed." << endl;

			//set the graph for discrete geodesics computer
			dis_gcomp.reset(new DiscreteGeoComputer(getHalfEdgeMesh()));
			cout<<"dis gcomp set."<<endl;
		}
		else {
			loadingProgress->setValue(100);

			return true;
		}
		loadingProgress->setValue(100);


		return true;
	}
	else
	{
		return false;
	}
}

HDS_Mesh* MeshManager::buildHalfEdgeMesh(const vector<MeshLoader::face_t> &inFaces,
										 const vector<MeshLoader::vert_t> &inVerts) {

	mesh_t *thismesh = new mesh_t;


	cout << "building the half edge mesh ..." << endl;
	int ss=0;
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
		faces[i] = new face_t;              //no value;
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
			//     cout<<"j = "<<j<<"  Fi.v[j] = "<<Fi.v[j]<<endl;//later added;
			int vjn = Fi.v[jn];
			//      cout<<"jn = "<<jn<<"  Fi.v[jn] = "<<Fi.v[jn]<<endl;//later added;
			pair<int, int> vPair = make_pair(vj, vjn);
			//cout<<"vPair.first = "<<vPair.first<<endl;  //later added;
			//cout<<"vPair.sencond = "<<vPair.second<<endl;
			if( heMap.find(vPair) == heMap.end() )//?? true every time;for heMap.end() points to he_t*, equal to heMap.find(vpair);
			{

				heMap[vPair] = hes[heIdx+j];  //address transport; hes[heIdx + j] = curHe; *****critical******
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

void MeshManager::cutMeshWithSelectedEdges(bool isExtended)
{
	//restrain unwanted redrawing in the middle of cutting operation
	if (extended_mesh != nullptr)
		extended_mesh->clearSortedFaces();

	QScopedPointer<HDS_Mesh> ref_mesh;
	if (!isExtended)
		ref_mesh.reset(new HDS_Mesh(*hds_mesh));
	else {
		ref_mesh.reset(new HDS_Mesh(*extended_mesh));
	}

	cout << "validating reference mesh" << endl;
	ref_mesh->validate();

	/// cut the mesh using the selected edges
	set<int> selectedEdges;
	for(auto he : ref_mesh->halfedges())
	{
		if( he->isPicked ) {
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
	while( !isUnfoldable )
	{
		bool cutSucceeded;
		if (!isExtended) {
			/// make a copy of the mesh with selected edges
			cutted_mesh.reset(new HDS_Mesh(*ref_mesh));
			cutSucceeded = MeshCutter::cutMeshUsingEdges(cutted_mesh.data(), selectedEdges);
		}else {
			/// make a copy of the mesh with selected edges
			extended_cutted_mesh.reset(new HDS_Mesh(*ref_mesh));
			cutSucceeded = MeshCutter::cutMeshUsingEdges(extended_cutted_mesh.data(), selectedEdges);
		}
		if( cutSucceeded )
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

//	for (auto f : cutted_mesh->faces())
//	{
//		if (f->isCutFace)
//		{
//			cout << "Face " << f->index << " is cutface" << endl;
//		}
//	}
	cout << ".........................." << endl;
}

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

		if (f->isConnector) {
			if (cutEdges.find(f->he->flip->index) != cutEdges.end()) {
				f->he->setPicked(true);

			}
		}
	}

	extended_mesh.reset(new HDS_Mesh(*des_mesh));

}



void MeshManager::unfoldMesh(bool isExtended)
{
	QScopedPointer<HDS_Mesh> ref_mesh;

	if (!isExtended) {
		ref_mesh.reset(new HDS_Mesh(*cutted_mesh));
	}
	else {
		ref_mesh.reset(new HDS_Mesh(*extended_cutted_mesh));
	}
	ref_mesh->validate();
	cout << "unfolded mesh set" << endl;

	/// cut the mesh using the selected edges
	set<int> selectedFaces;
	for (auto f : ref_mesh->faces()) {
		if (f->isPicked) {
			/// use picked edges as cut edges
			f->setPicked(false);
			if (selectedFaces.find(f->index) == selectedFaces.end()) {
				selectedFaces.insert(f->index);
			}
		}
	}
	unfolded_mesh.reset(new HDS_Mesh(*ref_mesh));

	if (MeshUnfolder::unfold(unfolded_mesh.data(), ref_mesh.take(), selectedFaces)) {
		/// unfolded successfully
		unfolded_mesh->printInfo("unfolded mesh:");
		//unfolded_mesh->printMesh("unfolded mesh:");
	}
	else {
		/// failed to unfold
		cout << "Failed to unfold." << endl;
	}

}


void MeshManager::smoothMesh()
{
	if( smoothed_mesh.isNull() )
	{
		cout<<"smoothmesh@@@"<<endl;
		smoothed_mesh.reset(new HDS_Mesh(*hds_mesh));
	}

	//MeshSmoother::smoothMesh(smoothed_mesh.data());
	//MeshSmoother::smoothMesh_perVertex(smoothed_mesh.data());
	MeshSmoother::smoothMesh_Laplacian(smoothed_mesh.data());
}

void MeshManager::resetMesh()
{
	extended_mesh.reset();
	cutted_mesh.reset();
	unfolded_mesh.reset();
	extended_cutted_mesh.reset();
	extended_unfolded_mesh.reset();
}

bool MeshManager::saveMeshes()
{
	/// save only the cutted and unfolded
	return true;
}


void MeshManager::extendMesh(int meshType, map<QString, double> config)
{
	HDS_Connector::setConnector(config);

	switch(meshType){
	case 0://original
		extended_mesh.reset(new HDS_Mesh(*hds_mesh));
		MeshExtender::extendMesh(extended_mesh.data());
		//update sorted faces
		extended_mesh->updateSortedFaces();
		break;

	case 2://cutted
		extended_cutted_mesh.reset(new HDS_Mesh(*cutted_mesh));
		MeshExtender::extendMesh(extended_cutted_mesh.data());
		//update sorted faces
		extended_cutted_mesh->updateSortedFaces();
		break;
	case 3://unfolded
		extended_unfolded_mesh.reset(new HDS_Mesh(*unfolded_mesh));
		MeshExtender::extendMesh(extended_unfolded_mesh.data());
		//update sorted faces
		extended_unfolded_mesh->updateSortedFaces();

		break;
	}

}

void MeshManager::setHollowMesh(double flapSize, int type, double shift)
{
	extended_cutted_mesh.reset(new HDS_Mesh(*hds_mesh));
    MeshHollower::hollowMesh(extended_cutted_mesh.data(), flapSize, type, shift);
	extended_cutted_mesh->updateSortedFaces();

}

void MeshManager::exportXMLFile(const char* filename)
{
	enum ConnectorType
	{
		SIMPLE_CONNECTOR,
		INSERT_CONNECTOR,
		GEAR_CONNECTOR,
		SAW_CONNECTOR,
		ADVSAW_CONNECTOR,
		HOLLOW_CONNECTOR
	};
	ConnectorType cn_t = SIMPLE_CONNECTOR;
	FILE *SVG_File;
	errno_t err = fopen_s(&SVG_File, filename, "w");
	if (err)
	{
		printf("Can't write to file %s!\n", filename);
		return;
	}
	int size_x(360), size_y(360);
	//printf("Type in SVG file size: ");
	//err = scanf_s("%d%d", &size_x, &size_y);
	
	/*unordered_set<face_t*> faces = unfolded_mesh->faces();
	unordered_set<face_t*> cutfaces;// , infaces;
	for (auto face : faces)
	{
		if (face->isCutFace)
		{
			cutfaces.insert(face);
		}
	}*/
	//SVG file head
	fprintf(SVG_File,
			"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" \
			"<svg width=\"%d\" height=\"%d\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n",
			size_x, size_y);// define the size of export graph

	/************************************************************************/
	/* for cut layer                                                        */
	/************************************************************************/
	
	double he_offset(10), he_scale(20), wid_conn(10), len_conn(10);
	double circle_offset = 3;
	//err = scanf_s("%lf", &circle_offset);

	int printFaceID(0), printCircleID(0);
	for (auto piece : unfolded_mesh->pieceSet)
	{
		vector<face_t *> cutfaces;

		vector<QVector2D> printBorderEdgePts;//Edges on the boundary
		vector<QVector2D> printEdgePtsCarves;
		vector<QVector2D> printCirclePos;

		// Group current piece
		fprintf(SVG_File, "<g opacity=\"0.8\">\n");
		for (auto fid : piece)
		{
			face_t *curFace = unfolded_mesh->faceMap[fid];
			auto he = curFace->he;
			auto curHE = he;
			if (curFace->isCutFace)
			{
				do
				{
					if (curHE->isExtended)
					{
						QVector2D Pc = curHE->flip->f->center().toVector2D() * he_scale;
						
						QVector2D v0 = curHE->prev->v->pos.toVector2D() * he_scale;
						QVector2D v1 = curHE->v->pos.toVector2D() * he_scale;
						QVector2D v2 = curHE->next->v->pos.toVector2D() * he_scale;
						QVector2D v3 = curHE->next->next->v->pos.toVector2D() * he_scale;

						QVector2D d1 = ((v0 - v1).normalized() + (v2 - v1).normalized()).normalized();
						QVector2D d2 = ((v1 - v2).normalized() + (v3 - v2).normalized()).normalized();
						if (d1.lengthSquared() != 0 && d2.lengthSquared() != 0)
						{
							printCirclePos.push_back(v1 + d1 * circle_offset);
							printCirclePos.push_back(v1 + d1 * circle_offset * 2);
							printCirclePos.push_back(v2 + d2 * circle_offset);
							printCirclePos.push_back(v2 + d2 * circle_offset * 2);
						}
					}
					printBorderEdgePts.push_back(curHE->v->pos.toVector2D() * he_scale);
					curHE = curHE->next;
				} while (curHE != he);
			}
			else
			{
				fprintf(SVG_File, "\t<polygon id=\"%d\" points=\"", printFaceID++);
				// Write points of each edge
				do
				{
					fprintf(SVG_File, "%f,%f ",
						curHE->v->pos.x() * he_scale,
						curHE->v->pos.y() * he_scale);
					curHE = curHE->next;
				} while (curHE != he);
				// Close face loop
				fprintf(SVG_File,
					"%f,%f\" style=\"fill:none;stroke:yellow;stroke-width:0.01\" />\n",
					curHE->v->pos.x() * he_scale,
					curHE->v->pos.y() * he_scale);
				curHE = curHE->next;
			}
			
		}
		/************************************************************************/
		/* Write out circles                                                    */
		/************************************************************************/
		for (auto circlepos : printCirclePos)
		{
			fprintf(SVG_File, "\t<circle id=\"Circle%d\" cx=\"%f\" cy=\"%f\" r=\"0.5\" " \
				"style=\"stroke:black;stroke-width:0.01;fill:white\" />\n",
				printCircleID++, circlepos.x(), circlepos.y());
		}
			
		/************************************************************************/
		/* Write out edge for cut                                               */
		/************************************************************************/
		fprintf(SVG_File, "\t<polygon id=\"%d\" points=\"", printFaceID++);
		for (int isec = 0; isec < printBorderEdgePts.size(); isec++)
		{
			fprintf(SVG_File, "%f,%f ", printBorderEdgePts[isec].x(), printBorderEdgePts[isec].y());
		}
		fprintf(SVG_File, "\" style=\"fill:none;stroke:blue;stroke-width:0.8\" />\n");
		/*fprintf(SVG_File, "%f,%f\" style=\"fill:none;stroke:blue;stroke-width:0.8\" />\n",
			printBorderEdgePts[0].x(), printBorderEdgePts[0].y());*/
		fprintf(SVG_File, "</g>\n");//set a new group for inner lines
	}
	/*
	for (auto face : cutfaces)
	{
		HDS_HalfEdge *he = face->he;
		HDS_HalfEdge *curHE = he;

		
		//////////////////////////////////////////////////////////////////////////
		unordered_set<he_t*> cutedges;
		//unordered_set<HDS_HalfEdge*> cutTwinEdges;
		vector<QVector2D*> printEdgePts;
		vector<QVector2D*> printEdgePtsCarves;
		// Get all edges on cut face
		do 
		{
			cutedges.insert(curHE);
			curHE = curHE->next;
		} while (curHE != he);

		if (unfolded_mesh->isHollowed)
		{
			cn_t = HOLLOW_CONNECTOR;
			do
			{
				QVector2D *Pthis = new QVector2D(curHE->v->pos.toVector2D() * he_scale);
				printEdgePts.push_back(Pthis);
				curHE = curHE->next;
			} while (curHE != he);
			printEdgePts.push_back(printEdgePts[0]);
			//continue;
		}
		else
		{
			QVector2D *Pthis = new QVector2D(curHE->v->pos.toVector2D() * he_scale);
			do
			{
				//QVector3D faceCenter = curHE->flip->f->center();
				QVector2D Pc = curHE->flip->f->center().toVector2D() * he_scale;

				QVector2D *Pnext = new QVector2D(curHE->next->v->pos.toVector2D() * he_scale);

				printEdgePts.push_back(Pthis);

				// T: normalized vector representing current edge direction
				// d: vector from current point to face center
				// a: vector projected from d onto T
				// n: normalized normal of the current edge
				switch (cn_t)
				{
				case HOLLOW_CONNECTOR:
				{
					
					break;
				}
				case SIMPLE_CONNECTOR:
				{
					if (cutedges.find(curHE) != cutedges.end())
					{
						//calculate 
						QVector2D T = (*Pnext - *Pthis).normalized();
						QVector2D d = (Pc - *Pthis);
						QVector2D a = QVector2D::dotProduct(d, T) * T;
						QVector2D n = (a - d).normalized();

						QVector2D Pn = *Pthis + a;
						QVector2D Psc = Pn + n * wid_conn;

						//draw connector

						QVector2D *Pnst = new QVector2D(Psc - len_conn * T);
						QVector2D *Pnsn = new QVector2D(Psc + len_conn * T);

						printEdgePts.push_back(Pnst);
						printEdgePts.push_back(Pnsn);

						cutedges.erase(curHE);
						//cutedges.erase(curHE->flip->cutTwin->flip);
					}
					else
					{
						//draw receiver
					}
					break;
				}/*
				 case INSERT_CONNECTOR:
				 {
				 //calculate
				 QVector2D T = (*Pnext - *Pthis).normalized();
				 QVector2D d = (Pc - *Pthis);
				 QVector2D a = QVector2D::dotProduct(d, T) * T;
				 QVector2D n = (a - d).normalized();

				 QVector2D Pn = *Pthis + a;
				 QVector2D Psc = Pn + n * wid_conn;

				 QVector2D *Pst = new QVector2D(Pn - len_conn * T);
				 QVector2D *Psn = new QVector2D(Pn + len_conn * T);
				 QVector2D *Pnst = new QVector2D(Psc - len_conn * T);
				 QVector2D *Pnsn = new QVector2D(Psc + len_conn * T);

				 QVector2D *Pcvt = new QVector2D(Pn - 0.5 * len_conn * T);
				 QVector2D *Pcvn = new QVector2D(Pn + 0.5 * len_conn * T);

				 printEdgePts.push_back(Pst);
				 printEdgePts.push_back(Pnst);
				 printEdgePts.push_back(Pnsn);
				 printEdgePts.push_back(Psn);

				 if (cutedges.find(curHE) != cutedges.end())
				 {
				 //draw connector
				 printEdgePtsCarves.push_back(Pst);
				 printEdgePtsCarves.push_back(Pcvt);
				 printEdgePtsCarves.push_back(Pcvn);
				 printEdgePtsCarves.push_back(Psn);

				 cutedges.erase(curHE);
				 cutedges.erase(curHE->flip->cutTwin->flip);
				 }
				 else
				 {
				 //draw receiver
				 printEdgePtsCarves.push_back(Pcvt);
				 printEdgePtsCarves.push_back(Pcvn);
				 }
				 break;
				 }
				 case GEAR_CONNECTOR:
				 {
				 //calculate
				 QVector2D T = (*Pnext - *Pthis).normalized();
				 QVector2D d = (Pc - *Pthis);
				 QVector2D a = QVector2D::dotProduct(d, T) * T;
				 QVector2D n = (a - d).normalized();

				 QVector2D Pn = *Pthis + a;
				 QVector2D Psc = Pn + n * wid_conn;

				 //division number
				 int ndiv = 8;
				 //connector segment length
				 double len_seg = Pthis->distanceToPoint(*Pnext) / ndiv * 0.5;

				 QVector2D *Pst = Pthis;
				 for (int i = 0; i < ndiv; i++)
				 {
				 if (i > 0)
				 {
				 printEdgePts.push_back(Pst);
				 }
				 QVector2D seg_T = T * len_seg;
				 QVector2D *Pnst = new QVector2D(*Pst + n * len_seg);
				 QVector2D *Pnsn = new QVector2D(*Pnst + seg_T);
				 QVector2D *Psn = new QVector2D(*Pst + seg_T);

				 Pst = new QVector2D(*Psn + seg_T);

				 printEdgePts.push_back(Pnst);
				 printEdgePts.push_back(Pnsn);
				 printEdgePts.push_back(Psn);

				 }
				 break;
				 }
				 case SAW_CONNECTOR:
				 {
				 //calculate
				 QVector2D T = (*Pnext - *Pthis).normalized();
				 QVector2D d = (Pc - *Pthis);
				 QVector2D a = QVector2D::dotProduct(d, T) * T;
				 QVector2D n = (a - d).normalized();

				 QVector2D Pn = *Pthis + a;
				 QVector2D Psc = Pn + n * wid_conn * 0.6;

				 QVector2D *Pst = new QVector2D(Pn - len_conn * T * 0.5);
				 QVector2D *Psn = new QVector2D(Pn + len_conn * T * 0.5);
				 QVector2D *Pnst = new QVector2D(Psc - len_conn * T * 0.5);
				 QVector2D *Pnsn = new QVector2D(Psc + len_conn * T * 0.5);
				 QVector2D *Pnn = new QVector2D(Pn);

				 printEdgePts.push_back(Pst);
				 printEdgePts.push_back(Pnst);
				 printEdgePts.push_back(Pnsn);
				 printEdgePts.push_back(Psn);

				 printEdgePtsCarves.push_back(Pnn);
				 printEdgePtsCarves.push_back(Psn);

				 break;
				 }
				 case ADVSAW_CONNECTOR:
				 {
				 //calculate
				 double edge_len = Pthis->distanceToPoint(*Pnext);
				 QVector2D T = (*Pnext - *Pthis).normalized();
				 QVector2D d = (Pc - *Pthis);
				 QVector2D a = QVector2D::dotProduct(d, T) * T;
				 QVector2D n = (a - d).normalized();

				 QVector2D Pn = *Pthis + T * edge_len * 0.5;
				 QVector2D Psc = Pn + n * wid_conn * 1.5;

				 //connector segment length
				 double edgeConn_len = edge_len * 0.5;

				 QVector2D *Pst = new QVector2D(Pn - edgeConn_len * T * 0.5);
				 QVector2D *Psn = new QVector2D(Pn + edgeConn_len * T * 0.1);
				 QVector2D *Pnst = new QVector2D(Psc - edgeConn_len * T * 0.25);
				 QVector2D *Pnsn = new QVector2D(Psc);
				 QVector2D *Pnn = new QVector2D(Pn);

				 printEdgePts.push_back(Pst);
				 printEdgePts.push_back(Pnst);
				 printEdgePts.push_back(Pnsn);
				 printEdgePts.push_back(Psn);

				 printEdgePtsCarves.push_back(Pnn);
				 printEdgePtsCarves.push_back(Psn);

				 break;
				 }
				default:
					break;
				}

				//printEdgePts.push_back(Pnext);

				Pthis = Pnext;
				curHE = curHE->next;
			} while (curHE != he);
		}
		/************************************************************************/
		/* Write out edge for cut                                               */
		/************************************************************************/
		/*fprintf(SVG_File,
			"<g opacity=\"0.8\">\n" \
			"\t<polygon id=\"%d\" points=\"",
			face->index);
		for (int i = 0; i < printEdgePts.size(); i++)
		{
			fprintf(SVG_File, "%f,%f ", printEdgePts[i]->x(), printEdgePts[i]->y());
		}
		fprintf(SVG_File, "%f,%f\" style=\"fill:none;stroke:blue;stroke-width:0.01\" />\n",
			printEdgePts[0]->x(), printEdgePts[0]->y());*/
		/************************************************************************/
		/* Print carve edges                                                    */
		/************************************************************************/
		/*if (printEdgePtsCarves.size() > 0)
		{
			
			for (int i = 0; i < printEdgePtsCarves.size(); i += 2)
			{
				fprintf(SVG_File, "\t<polyline id=\"%d\" points=\"%f,%f %f,%f\" " \
					"style=\"fill:none;stroke:blue;stroke-width:0.8\" />\n",
					i,
					printEdgePtsCarves[i]->x(), printEdgePtsCarves[i]->y(),
					printEdgePtsCarves[i + 1]->x(), printEdgePtsCarves[i + 1]->y());

			}
		}*/
		
		/// draw connected faces
		/*set<HDS_Face*> neighbourFaces = face->connectedFaces();
		for (auto face : neighbourFaces)
		{
			HDS_HalfEdge *he = face->he;
			HDS_HalfEdge *curHE = he;
			// Write polygon header
			fprintf(SVG_File, "\t<polygon id=\"%d\" points=\"", face->index);
			// Write points of each edge
			do {
				{
					fprintf(SVG_File, "%f,%f ",
						curHE->v->pos.x(), curHE->v->pos.y());
				}
				curHE = curHE->next;
			} while (curHE != he);
			// Close face loop
			fprintf(SVG_File, "%f,%f\" style=\"fill:none;stroke:yellow;stroke-width:0.01\" />\n",
				curHE->v->pos.x(), curHE->v->pos.y());
		}
		//////////////////////////////////////////////////////////////////////////
		fprintf(SVG_File, "</g>\n");//set a new group for inner lines
	}*/
	
	/************************************************************************/
	/* End of SVG File End                                                  */
	/************************************************************************/
	fprintf(SVG_File, "</svg>");
	fclose(SVG_File);
	printf("SVG file %s saved successfully!\n", filename);
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

#if USE_REEB_GRAPH
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
#endif

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

