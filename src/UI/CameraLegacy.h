#include "common.h"
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>

struct CameraLegacy {
	CameraLegacy() :zNear(1.0), zFar(7000.0), fov(45.0){
		translation = QVector3D(0, 0, -5);
		angularChange = 0;
	}


	void updateViewport(int w, int h) {
		viewport.x = 0;
		viewport.y = 0;
		viewport.w = w;
		viewport.h = h;

		aspect = (qreal)w / (qreal)h;
	}

	void updateProjection() {
		projection.setToIdentity();
		projection.perspective(fov, aspect, zNear, zFar);
	}

	void updateModelView() {
		QMatrix4x4 matrix;
		matrix.translate(translation);
		matrix.rotate(rotation);
		modelview = matrix;
	}

	QMatrix4x4 projectionModelView() const {
		return projection * modelview;
	}

	struct {
		int x, y, w, h;
	} viewport;

	void print() {
		qDebug() << "viewstate:";
		qDebug() << viewport.x << ", " << viewport.y << ", " << viewport.w << ", " << viewport.h;
		/*     qDebug() << modelview;
		qDebug() << projection;
		qDebug() << rotationAxis;
		qDebug() << angularChange;
		qDebug() << rotation;
		qDebug() << translation;
		qDebug() << zNear << ", " << zFar << ", " << fov;
		qDebug() << aspect;
		*/
	}

	QMatrix4x4 modelview;
	QMatrix4x4 projection;

	QVector3D rotationAxis;
	qreal angularChange;
	QQuaternion rotation;
	QVector3D translation;

	qreal zNear, zFar, fov;
	qreal aspect;
};