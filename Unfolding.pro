#-------------------------------------------------
#
# Project created by QtCreator 2014-09-04T16:35:12
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Unfolding
TEMPLATE = app

CONFIG += c++11
CONFIG += console

QMAKE_CXXFLAGS += -std=c++11

#INCLUDEPATH += /usr/local/include /Users/phg/Utils/armadillo-4.450.4/include
#LIBS += -L/usr/local/lib/OpenMesh -lOpenMeshCore -lOpenMeshTools -L/usr/local/lib -framework Accelerate

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/hds_face.cpp \
    src/hds_vertex.cpp \
    src/hds_mesh.cpp \
    src/hds_halfedge.cpp \
    src/meshviewer.cpp \
    src/meshmanager.cpp \
    src/meshloader.cpp \
    src/stringutils.cpp \
    src/meshcutter.cpp \
    src/meshunfolder.cpp \
    extras/colormap_editor/colormapeditor.cpp \
    src/colormap.cpp \
    src/unionfind.cpp \
    src/meshsmoother.cpp \
    src/criticalpointspanel.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/BaseModel.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ExactMethodForDGP.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ICHWithFurtherPriorityQueue.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/ImprovedCHWithEdgeValve.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/Point3D.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/PreviousCH.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/RichModel.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/stdafx.cpp \
    extras/SVG_LC_code/SVG_precompute/LocalGeodesics/svg_precompute.cpp \
    src/morsesmalecomplex.cpp \
    src/GeodesicComputer.cpp \
    src/MeshExtender.cpp \
    src/discretegeocomputer.cpp \
    src/cutlocuspanel.cpp \
    src/hds_connector.cpp \
    src/connectorpanel.cpp \
    src/connectorpanelviewer.cpp

HEADERS  += src/mainwindow.h \
    src/hds_face.h \
    src/hds_vertex.h \
    src/hds_mesh.h \
    src/hds_halfedge.h \
    src/meshviewer.h \
    src/common.h \
    src/glutils.hpp \
    src/meshmanager.h \
    src/meshloader.h \
    src/stringutils.h \
    src/mathutils.hpp \
    src/meshcutter.h \
    src/utils.hpp \
    src/meshunfolder.h \
    extras/colormap_editor/colormapeditor.h \
    src/colormap.h \
    src/unionfind.h \
    src/meshsmoother.h \
    src/criticalpointspanel.h \
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
    src/morsesmalecomplex.h \
    src/GeodesicComputer.h \
    src/MeshExtender.h \
    src/discretegeocomputer.h \
    src/cutlocuspanel.h \
    src/hds_connector.h \
    src/connectorpanel.h \
    src/connectorpanelviewer.h
FORMS    += forms/mainwindow.ui \
    extras/colormap_editor/colormapeditor.ui \
    forms/criticalpointspanel.ui \
    forms/cutlocuspanel.ui \
    forms/connectorpanel.ui

RESOURCES += \
    icons.qrc

OTHER_FILES +=
LIBS += -glut32
LIBS += -Lc:\glut
#unix: LIBS += -L$$PWD/../../Utils/levmar-2.6/ -llevmar
#INCLUDEPATH += $$PWD/../../Utils/levmar-2.6
#DEPENDPATH += $$PWD/../../Utils/levmar-2.6
#unix: PRE_TARGETDEPS += $$PWD/../../Utils/levmar-2.6/liblevmar.a
