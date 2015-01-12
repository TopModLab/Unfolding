#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include "GeodesicComputer.h"

#include "common.h"
#include <QMutex>
#include <QScopedPointer>
#include <QSharedPointer>

#include "meshloader.h"

#include "hds_mesh.h"
#include "hds_face.h"
#include "hds_halfedge.h"
#include "hds_vertex.h"

#include "Graph.hpp"

#include <vtkOBJReader.h>
#include <vtkSmartPointer.h>

class MeshManager
{
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

    HDS_Mesh* getHalfEdgeMesh() {
        return hds_mesh.data();
    }

    HDS_Mesh* getSmoothedMesh() {
        return smoothed_mesh.data();
    }

    HDS_Mesh* getCuttedMesh() {
        return cutted_mesh.data();
    }

    HDS_Mesh* getUnfoldedMesh() {
        return unfolded_mesh.data();
    }

    HDS_Mesh* getExtendedMesh() {
      return extended_mesh.data();
    }

    void updateReebGraph(const vector<double> &fvals = vector<double>());
    SimpleGraph* getReebGraph() {
      return &rbGraph;
    }

    vector<double> getInterpolatedGeodesics(int vidx, int lev0, int lev1, double alpha);
    vector<double> getInterpolatedCurvature(int lev0, int lev1, double alpha);
    vector<double> getInterpolatedPointNormalValue(int lev0, int lev1, double alpha, const QVector3D &pnormal);
    vector<double> getInterpolatedZValue(int lev0, int lev1, double alpha);

    void colorMeshByGeoDistance(int vidx);
    void colorMeshByGeoDistance(int vidx, int lev0, int lev1, double ratio);
    bool saveMeshes();

protected:
    /// should not be externally accessible
    static void drop()
    {
        static QMutex mutex;
        mutex.lock();
        delete instance;
        instance = NULL;
        mutex.unlock();
    }

private:
    MeshManager() {}
    MeshManager(const MeshManager &) = delete;
    MeshManager& operator=(const MeshManager &) = delete;

    static MeshManager* instance;

    friend class MeshViewer;

public:
    bool loadOBJFile(const string& filename);
    HDS_Mesh* buildHalfEdgeMesh(const vector<MeshLoader::face_t> &faces, const vector<MeshLoader::vert_t> &verts);
    void cutMeshWithSelectedEdges();
    void unfoldMesh();
    void smoothMesh();
    void extendMesh();

private:
    typedef HDS_Mesh mesh_t;
    typedef HDS_HalfEdge he_t;
    typedef HDS_Face face_t;
    typedef HDS_Vertex vert_t;
    
    QScopedPointer<HDS_Mesh> hds_mesh, extended_mesh, smoothed_mesh, cutted_mesh, unfolded_mesh;
    QScopedPointer<GeodesicComputer> gcomp;

    vector<QSharedPointer<HDS_Mesh>> hds_mesh_smoothed;
    vector<QSharedPointer<GeodesicComputer>> gcomp_smoothed;
    vtkSmartPointer<vtkPolyData> vtkMesh;
    SimpleGraph rbGraph;
};

#endif // MESHMANAGER_H
