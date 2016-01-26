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
	perspCamera(const QVector3D& eyePos = QVector3D(0, 0, 0),
		const QVector3D& targetPos = QVector3D(0, 0, 1),
		const QVector3D& upVec = QVector3D(0, 1, 0),
		Float verticalAngle = 90, Float aspectRatio = 1.67,
		Float nearPlane = 0.01, Float farPlane = 100);
	~perspCamera(){};

	QVector3D getTarget() const;

	void lookAt(const QVector3D& eyePos = QVector3D(0, 0, 0),
		const QVector3D& targetPos = QVector3D(0, 0, 1),
		const QVector3D& upVec = QVector3D(0, 1, 0));
	void setPerspective(Float verticalAngle = 180, Float aspectRatio = 1.67,
		Float nearPlane = 0.1, Float farPlane = 100);
	void updateProjection(const QMatrix4x4 &perspMat);
	void updateCamToWorld(const QMatrix4x4 &cam2wMat);

	void zoom(Float x_val = 0, Float y_val = 0, Float z_val = 0);
	void rotate(Float x_rot = 0, Float y_rot = 0, Float z_rot = 0);
	void resizeViewport(Float aspr = 1.0);
	
	const float* oglViewMatrix() const;
	const float* oglProjectionMatrix() const;

	QMatrix4x4 CameraToWorld, WorldToCamera, CameraToScreen, RasterToScreen;
protected:
	QVector3D pos, target;

	Float fov;
	int sample = 1;
private:
};
#endif