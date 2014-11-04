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

INCLUDEPATH += /usr/local/include /Users/phg/Utils/armadillo-4.450.4/include
LIBS += -L/usr/local/lib/OpenMesh -lOpenMeshCore -lOpenMeshTools -L/usr/local/lib -framework Accelerate

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
    trimesh.cpp \
    polymesh.cpp \
    meshsmoother.cpp

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
    trimesh.h \
    polymesh.h \
    meshsmoother.h \
    numerical.h
FORMS    += mainwindow.ui \
    extras/colormap_editor/colormapeditor.ui

RESOURCES += \
    icons.qrc

OTHER_FILES +=

unix: LIBS += -L$$PWD/../../Utils/levmar-2.6/ -llevmar

INCLUDEPATH += $$PWD/../../Utils/levmar-2.6
DEPENDPATH += $$PWD/../../Utils/levmar-2.6

unix: PRE_TARGETDEPS += $$PWD/../../Utils/levmar-2.6/liblevmar.a
