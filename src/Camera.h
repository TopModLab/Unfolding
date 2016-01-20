//
//  Camera.h
//
//  Created by Shenyao Ke on 1/29/15.
//  Copyright (c) 2015 AKIKA. All rights reserved.
//
#pragma once
#ifndef __Camera__
#define __Camera__


typedef double Float;

#include "common.h"
#include "mathutils.hpp"
#include <QVector3D>
#include <QMatrix4x4>

class perspCamera
{
public:
	perspCamera(const QVector3D& eyePos, const QVector3D& targetPos, const QVector3D& upVec);
	~perspCamera(){};

	void setResolution(int resX, int resY);
	void setSample(int aaSample);
	void setFocLen(Float fl);

	QVector3D getTarget() const;

	void updateProjection(const QMatrix4x4 &perspMat);
	void updateCamToWorld(const QMatrix4x4 &cam2wMat);

	void zoom(Float x_val = 0, Float y_val = 0, Float z_val = 0);
	void rotate(Float x_rot = 0, Float y_rot = 0, Float z_rot = 0);
	void resizeViewport(Float aspr = 1.0);
	void exportVBO(vector<float>* view = nullptr, vector<float>* proj = nullptr, vector<float>* raster = nullptr) const;

	QMatrix4x4 CameraToWorld, WorldToCamera, CameraToScreen, RasterToScreen;
	//Transform CameraToWorld;
	//Transform CameraToScreen, RasterToScreen;
protected:
	QVector3D pos, target;

	Float fov;
	int sample = 1;
private:
};
#endif