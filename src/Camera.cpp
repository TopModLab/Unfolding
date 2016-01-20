#include "Camera.h"

perspCamera::perspCamera(const QVector3D& eyePos, const QVector3D& targetPos,
	const QVector3D& upVec)
	: target(targetPos)
{
	CameraToWorld.lookAt(eyePos, target, upVec);
	CameraToScreen.perspective(90, 1.6, 0.001, 100);
}

QVector3D perspCamera::getTarget() const
{
	return target;
}

void perspCamera::exportVBO(vector<float>* view, vector<float>* proj, vector<float>* raster) const
{
	if (view != nullptr)
	{
		delete view;
		auto w2c_data = WorldToCamera.data();
		view = new vector<float>(w2c_data, w2c_data + 16);
		//WorldToCamera.mInv.exportVBO(view);
	}
	if (proj != nullptr)
	{
		delete proj;
		auto c2s = CameraToScreen.data();
		proj = new vector<float>(c2s, c2s + 16);
		//CameraToScreen.m.exportVBO(proj);
	}
	if (raster != nullptr)
	{
		delete raster;
		auto r2s = RasterToScreen.data();
		raster = new vector<float>(r2s, r2s + 16);
		//RasterToScreen.m.exportVBO(raster);
	}
}
void perspCamera::zoom(Float x_val, Float y_val, Float z_val)
{
	QMatrix4x4 cam2w = CameraToWorld;// .getMat();
	
	QVector3D _nx(cam2w.column(0)), _ny(cam2w.column(1));
	target += _nx * x_val + _ny * y_val;

	cam2w.translate(x_val, y_val, z_val);

	CameraToWorld = cam2w;
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
	
	if ((old_theta < M_HALFPI && theta > M_HALFPI) || (old_theta > -M_HALFPI && theta < -M_HALFPI))
	{
		upCoef *= -1;
	}
	QVector3D newVt(sin(phi) * cos(theta), sin(theta), cos(phi) * cos(theta));
	CameraToWorld.setToIdentity();
	CameraToWorld.lookAt(target + newVt * vt_len, target, QVector3D(0, upCoef, 0));
}

void perspCamera::resizeViewport(Float aspr)
{
	QMatrix4x4 newProj= CameraToScreen;
	newProj(0, 0) = -newProj(1, 1) / aspr;
	CameraToScreen = newProj;
	
}