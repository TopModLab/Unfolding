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
    meshviewer.cpp \
    meshmanager.cpp \
    meshloader.cpp \
    stringutils.cpp \
    meshcutter.cpp

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
    utils.hpp

FORMS    += mainwindow.ui

RESOURCES += \
    icons.qrc
