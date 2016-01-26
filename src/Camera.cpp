#include "Camera.h"

perspCamera::perspCamera(const QVector3D& eyePos, const QVector3D& targetPos,
	const QVector3D& upVec, Float verticalAngle, Float aspectRatio,
	Float nearPlane, Float farPlane)
	: target(targetPos)
{
	WorldToCamera.lookAt(eyePos, targetPos, upVec);
	CameraToWorld = WorldToCamera.inverted();
	CameraToScreen.perspective(verticalAngle, aspectRatio, nearPlane, farPlane);
	//setPerspective(verticalAngle, aspectRatio, nearPlane, farPlane);
}

QVector3D perspCamera::getTarget() const
{
	return target;
}

void perspCamera::lookAt(const QVector3D& eyePos,
	const QVector3D& targetPos, const QVector3D& upVec)
{
	//
}

void perspCamera::setPerspective(Float verticalAngle, Float aspectRatio,
	Float nearPlane, Float farPlane)
{
	//
}

const float* perspCamera::oglViewMatrix() const
{
	return WorldToCamera.constData();
}

const float* perspCamera::oglProjectionMatrix() const
{
	return CameraToScreen.constData();
}

void perspCamera::zoom(Float x_val, Float y_val, Float z_val)
{
	QMatrix4x4 cam2w = CameraToWorld;// .getMat();
	
	QVector3D _nx(cam2w.column(0)), _ny(cam2w.column(1));
	target += _nx * x_val + _ny * y_val;

	cam2w.translate(x_val, y_val, z_val);

	CameraToWorld = cam2w;
	WorldToCamera = CameraToWorld.inverted();
}

void perspCamera::rotate(Float x_rot, Float y_rot, Float z_rot)
{
	//pitch, yaw, roll
	QMatrix4x4 lookAtMat = CameraToWorld;

	QVector3D _pos(lookAtMat.column(3));
	QVector3D vt = _pos - target;
	Float vt_len = vt.length();
	Float upCoef = lookAtMat(1, 1) < 0 ? -1 : 1;
	
	Float phi = atan2(vt.x(), vt.z()) + DegreeToRadian(y_rot) * upCoef;
	Float old_theta = asin(vt.y() / vt_len);
	Float theta = old_theta + DegreeToRadian(x_rot) * upCoef;
	
	if ((old_theta < M_HALFPI && theta > M_HALFPI)
		|| (old_theta > -M_HALFPI && theta < -M_HALFPI))
	{
		upCoef *= -1;
	}
	QVector3D newVt(sin(phi) * cos(theta), sin(theta), cos(phi) * cos(theta));
	WorldToCamera.setToIdentity();
	WorldToCamera.lookAt(target + newVt * vt_len,
						target, QVector3D(0, upCoef, 0));
	CameraToWorld = WorldToCamera.inverted();
}

void perspCamera::resizeViewport(Float aspr)
{
	//QMatrix4x4 newProj= CameraToScreen;
	CameraToScreen(0, 0) = -CameraToScreen(1, 1) / aspr;
	//CameraToScreen = newProj;
}