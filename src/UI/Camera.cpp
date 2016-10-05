#include "Camera.h"

perspCamera::perspCamera(const QVector3D& eyePos, const QVector3D& targetPos,
	const QVector3D& upVec, Float verticalAngle, Float aspectRatio,
	Float nearPlane, Float farPlane)
	: target(targetPos)
{
	WorldToCamera.lookAt(eyePos, targetPos, upVec);
	CameraToWorld = WorldToCamera.inverted();
	CameraToScreen.perspective(verticalAngle, aspectRatio, nearPlane, farPlane);
}


void perspCamera::zoom(Float x_val, Float y_val, Float z_val)
{
	target += (CameraToWorld.column(0) * x_val
			+ CameraToWorld.column(1) * y_val).toVector3D();

	CameraToWorld.translate(x_val, y_val, z_val);
	WorldToCamera = CameraToWorld.inverted();
}

void perspCamera::rotate(Float x_rot, Float y_rot, Float z_rot)
{
	//pitch, yaw, roll
	// View Vector = current_camera_pos - target_pos
	QVector3D vt = CameraToWorld.column(3).toVector3DAffine() - target;
	// Rotation Sphere Radius
	Float vt_len = vt.length();
	Float upCoef = CameraToWorld(1, 1) < 0 ? -1 : 1;
	
	Float phi = atan2(vt.x(), vt.z()) + DegreeToRadian(y_rot) * upCoef;
	Float old_theta = asin(vt.y() / vt_len);
	Float theta = old_theta + DegreeToRadian(x_rot) * upCoef;
	
	if ((old_theta <  M_HALFPI && theta >  M_HALFPI) ||
		(old_theta > -M_HALFPI && theta < -M_HALFPI))
	{
		upCoef = -upCoef;
	}
	QVector3D newVt(sin(phi) * cos(theta), sin(theta), cos(phi) * cos(theta));
	WorldToCamera.setToIdentity();
	WorldToCamera.lookAt(target + newVt * vt_len,
						target, QVector3D(0, upCoef, 0));
	CameraToWorld = WorldToCamera.inverted();
}

void perspCamera::resizeViewport(Float aspr)
{
	CameraToScreen(0, 0) = -CameraToScreen(1, 1) / aspr;
}