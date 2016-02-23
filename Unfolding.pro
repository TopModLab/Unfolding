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

INCLUDEPATH += extras/colormap_editor src

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/hds_common.cpp \
    src/hds_vertex.cpp \
    src/hds_halfedge.cpp \
    src/hds_face.cpp \
    src/hds_mesh.cpp \
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
	src/hds_bridger.cpp \
	src/bridgerpanel.cpp \
	src/bridgerpanelviewer.cpp \
    src/hollowmeshpanel.cpp \
    src/meshhollower.cpp \
    src/BBox.cpp \
    src/bindingmeshpanel.cpp \
    src/ConnectorPanel.cpp \
    src/MeshConnector.cpp \
    src/meshrimface.cpp \
    src/OperationStack.cpp \
    src/rimfacepanel.cpp \
    src/Camera.cpp \
    src/MeshViewerModern.cpp \
	src/ViewerGrid.cpp

HEADERS  += src/mainwindow.h \
    src/hds_common.h \
    src/hds_vertex.h \
    src/hds_halfedge.h \
    src/hds_face.h \
    src/hds_mesh.h \
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
	src/hds_bridger.h \
	src/bridgerpanel.h \
	src/bridgerpanelviewer.h \
    src/hollowmeshpanel.h \
    src/meshhollower.h \
    src/BBox.h \
    src/bindingmeshpanel.h \
    src/ConnectorPanel.h \
    src/MeshConnector.h \
    src/meshrimface.h \
    src/OperationStack.h \
    src/rimfacepanel.h \
    src/Camera.h \
    src/MeshViewerModern.h \
    src/CameraLegacy.h \
    src/ViewerGrid.h

FORMS    += forms/mainwindow.ui \
    extras/colormap_editor/colormapeditor.ui \
    forms/criticalpointspanel.ui \
    forms/cutlocuspanel.ui \
	forms/bridgerpanel.ui \
    forms/hollowmeshpanel.ui \
    forms/bindingmeshpanel.ui \
    forms/ConnectorPanel.ui \
    forms/rimfacepanel.ui

RESOURCES += \
    icons.qrc

OTHER_FILES +=
LIBS += -glut32
LIBS += -Lc:\glut

DISTFILES += \
    shaders/edge_fs.glsl \
    shaders/edge_vs.glsl \
    shaders/face_fs.glsl \
    shaders/face_gs.glsl \
    shaders/face_vs.glsl \
