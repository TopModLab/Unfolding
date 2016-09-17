#pragma once
#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include "GeomUtils/GeodesicComputer.h"
#include "GeomUtils/DiscreteGeoComputer.h"

#include "Utils/common.h"
#include <QMutex>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QProgressDialog>
#include <QString>
#include <QMessageBox>
#include <QTime>

#include "GeomProc/meshloader.h"
#include "GeomProc/MeshConnector.h"
#include "GeomUtils/OperationStack.h"

#include "HDS/hds_mesh.h"
#include "HDS/hds_face.h"
#include "HDS/hds_halfedge.h"
#include "HDS/hds_vertex.h"
#include "GeomUtils/hds_bridger.h"

#include "GeomUtils/Graph.h"

#define USE_REEB_GRAPH 0
#if USE_REEB_GRAPH
#include <vtkOBJReader.h>
#include <vtkSmartPointer.h>
#endif

class MeshManager
{
private:
	MeshManager() : operationStack(new OperationStack), meshloader(new OBJLoader) {}
	MeshManager(const MeshManager &) = delete;
	MeshManager& operator=(const MeshManager &) = delete;
public:
	static MeshManager* getInstance()
	{
		static QMutex mutex;
		if (!instance)
		{
			mutex.lock();

			if (!instance)
				instance = new MeshManager;

			mutex.unlock();
		}

		return instance;
	}

	OperationStack* getMeshStack() {
		return operationStack.data();
	}
	HDS_Mesh* getSmoothedMesh() {
		return smoothed_mesh.data();
	}


#if USE_REEB_GRAPH
	void updateReebGraph(const doubles_t &fvals = doubles_t());
	SimpleGraph* getReebGraph() {
		return &rbGraph;
	}
#endif

	doubles_t getInterpolatedGeodesics(int vidx, int lev0, int lev1, double alpha);
	doubles_t getInterpolatedCurvature(int lev0, int lev1, double alpha);
	doubles_t getInterpolatedPointNormalValue(int lev0, int lev1, double alpha, const QVector3D &pnormal);
	doubles_t getInterpolatedZValue(int lev0, int lev1, double alpha);

	void colorMeshByGeoDistance(int vidx);
	void colorMeshByGeoDistance(int vidx, int lev0, int lev1, double ratio);

	bool loadOBJFile(const string& filename);
	HDS_Mesh* buildHalfEdgeMesh(const doubles_t &inVerts, const vector<PolyIndex*> &inFaces);
	void cutMeshWithSelectedEdges();
	bool initSparseGraph();
	//void mapToExtendedMesh();
	bool unfoldMesh();
	bool smoothMesh();

	bool setGES();
	bool setQuadEdge(double fsize, int type, double shift);
	bool setGRS(const confMap &conf);
	bool rimMesh(double rimSize = 0.0);
	bool set3DRimMesh(const confMap &conf);
	bool setWeaveMesh(const confMap &conf);
	bool createDFormMesh();
	// Export as SVG files
	bool exportSVGFile(const QString &filename, const confMap &conf);
	bool saveMeshes(const string &filename);

	void setCurrentOpFlag(OperationStack::Flag curFlag) { operationStack->setCurrentFlag(curFlag); }
public slots:
	static void setPanelType(int type){ panelType = type;}

protected:
	// should not be externally accessible
	static void drop()
	{
		static QMutex mutex;
		mutex.lock();
		delete instance;
		instance = nullptr;
		mutex.unlock();
	}
	
private:
	typedef HDS_Vertex vert_t;
	typedef HDS_HalfEdge he_t;
	typedef HDS_Face face_t;
	typedef HDS_Mesh mesh_t;

	static MeshManager* instance;

	static int panelType;
	QScopedPointer<OperationStack> operationStack;
	QScopedPointer<OBJLoader> meshloader;
	QScopedPointer<HDS_Mesh> smoothed_mesh;
	QScopedPointer<GeodesicComputer> gcomp;
	QScopedPointer<DiscreteGeoComputer> dis_gcomp;


	vector<QSharedPointer<HDS_Mesh>> hds_mesh_smoothed;
	vector<QSharedPointer<GeodesicComputer>> gcomp_smoothed;
#if USE_REEB_GRAPH
	vtkSmartPointer<vtkPolyData> vtkMesh;
	SimpleGraph rbGraph;
#endif

	friend class MeshViewerLegacy;
	friend class MeshViewer;
	friend class MeshConnector;
};

#endif // MESHMANAGER_H
