#-------------------------------------------------
#
# Project created by QtCreator 2014-09-04T16:35:12
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Unfolding2
TEMPLATE = app

CONFIG += c++11
CONFIG += console

QMAKE_CXXFLAGS += -std=c++11

#INCLUDEPATH += /usr/local/include /Users/phg/Utils/armadillo-4.450.4/include
#LIBS += -L/usr/local/lib/OpenMesh -lOpenMeshCore -lOpenMeshTools -L/usr/local/lib -framework Accelerate

SOURCES += main.cpp\
        mainwindow.cpp \
    hds_face.cpp \
    hds_vertex.cpp \
    hds_mesh.cpp \
    hds_halfedge.cpp \
    meshviewer.cpp \
    meshmanager.cpp \
    meshloader.cpp \
    stringutils.cpp \
    meshcutter.cpp \
    meshunfolder.cpp \
    extras/colormap_editor/colormapeditor.cpp \
    colormap.cpp \
    unionfind.cpp \
    meshsmoother.cpp \
    criticalpointspanel.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/BaseModel.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ExactMethodForDGP.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ICHWithFurtherPriorityQueue.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ImprovedCHWithEdgeValve.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/Point3D.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/PreviousCH.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/RichModel.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/stdafx.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/svg_precompute.cpp \
    morsesmalecomplex.cpp \
    GeodesicComputer.cpp \
    MeshExtender.cpp

HEADERS  += mainwindow.h \
    hds_face.h \
    hds_vertex.h \
    hds_mesh.h \
    hds_halfedge.h \
    meshviewer.h \
    common.h \
    glutils.hpp \
    meshmanager.h \
    meshloader.h \
    stringutils.h \
    mathutils.hpp \
    meshcutter.h \
    utils.hpp \
    meshunfolder.h \
    extras/colormap_editor/colormapeditor.h \
    colormap.h \
    unionfind.h \
    meshsmoother.h \
    criticalpointspanel.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/BaseModel.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ExactMethodForDGP.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ICHWithFurtherPriorityQueue.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ImprovedCHWithEdgeValve.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/Point3D.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/PreviousCH.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/RichModel.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/stdafx.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/svg_precompute.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/targetver.h \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/wxnTime.h \
    morsesmalecomplex.h \
    GeodesicComputer.h \
    MeshExtender.h
FORMS    += mainwindow.ui \
    extras/colormap_editor/colormapeditor.ui \
    criticalpointspanel.ui

RESOURCES += \
    icons.qrc

OTHER_FILES +=
LIBS += -glut32
LIBS += -Lc:\glut
#unix: LIBS += -L$$PWD/../../Utils/levmar-2.6/ -llevmar
#INCLUDEPATH += $$PWD/../../Utils/levmar-2.6
#DEPENDPATH += $$PWD/../../Utils/levmar-2.6
#unix: PRE_TARGETDEPS += $$PWD/../../Utils/levmar-2.6/liblevmar.a
