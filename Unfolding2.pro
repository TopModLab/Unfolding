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

SOURCES += main.cpp\
        mainwindow.cpp \
    hds_face.cpp \
    hds_vertex.cpp \
    hds_mesh.cpp \
    hds_halfedge.cpp \
    thirdparty/tinyobjloader/tiny_obj_loader.cc \
    meshviewer.cpp \
    meshmanager.cpp \
    meshloader.cpp \
    stringutils.cpp

HEADERS  += mainwindow.h \
    hds_face.h \
    hds_vertex.h \
    hds_mesh.h \
    hds_halfedge.h \
    thirdparty/tinyobjloader/tiny_obj_loader.h \
    meshviewer.h \
    common.h \
    glutils.hpp \
    meshmanager.h \
    meshloader.h \
    stringutils.h

FORMS    += mainwindow.ui

RESOURCES += \
    icons.qrc
