#include "bridgerpanelviewer.h"

#include <QMouseEvent>
#include <QMessageBox>

BridgerPanelViewer::BridgerPanelViewer(QWidget *parent)
	: QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{

	shapeIndex = 0;
	curvature = 5;
	nSamples = 3;
	size = 80;
	convergingPointPos = 0;
	openingIndex = 0;
}

BridgerPanelViewer::~BridgerPanelViewer()
{

}

static void qNormalizeAngle(int &angle)
{
	while (angle < 0)
		angle += 360 * 16;
	while (angle > 360)
		angle -= 360 * 16;
}

void BridgerPanelViewer::setXRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != xRot) {
		xRot = angle;
		updateGL();
	}
}

void BridgerPanelViewer::setYRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != yRot) {
		yRot = angle;
		updateGL();
	}
}

void BridgerPanelViewer::setZRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != zRot) {
		zRot = angle;
		updateGL();
	}
}

void BridgerPanelViewer::initializeGL()
{
	qglClearColor(Qt::white);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);

	static GLfloat lightPosition[4] = { 0, 0, 10, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

void BridgerPanelViewer::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -10.0);
	glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
	glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
	glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);
	draw();
}

void BridgerPanelViewer::resizeGL(int width, int height)
{
	int side = qMin(width, height);
	glViewport((width - side) / 2, (height - side) / 2, side, side);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#ifdef QT_OPENGL_ES_1
	glOrthof(-2, +2, -2, +2, 1.0, 15.0);
#else
	glOrtho(-2, +2, -2, +2, 1.0, 15.0);
#endif

	glMatrixMode(GL_MODELVIEW);
}

void BridgerPanelViewer::mousePressEvent(QMouseEvent *event)
{
	lastPos = event->pos();

}

void BridgerPanelViewer::mouseMoveEvent(QMouseEvent *event)
{
	int dx = event->x() - lastPos.x();
	int dy = event->y() - lastPos.y();

	if (event->buttons() & Qt::LeftButton) {
		setXRotation(xRot + 8 * dy);
		setYRotation(yRot + 8 * dx);
	} else if (event->buttons() & Qt::RightButton) {
		setXRotation(xRot + 8 * dy);
		setZRotation(zRot + 8 * dx);
	}

	lastPos = event->pos();
}

void BridgerPanelViewer::draw()
{
	glColor4f(0.25, 0.25, 0.25, 1);
	glBegin(GL_QUADS);
	glVertex3f(0,-1,1);
	glVertex3f(0,1,0);
	glVertex3f(1,0,0);
	glVertex3f(1,-2,1);
	glEnd();

	glColor4f(0.75, 0.75, 0.75, 1);
	glBegin(GL_QUADS);
	glVertex3f(0,-1,1);
	glVertex3f(0,1,0);
	glVertex3f(-1,0,0);
	glVertex3f(-1,-2,1);
	glEnd();

}

void BridgerPanelViewer::setShape(int)
{

}

void BridgerPanelViewer::setCurvature(int curv)
{

}

void BridgerPanelViewer::setSamples(int n)
{

}

void BridgerPanelViewer::setSize(int size)
{

}

void BridgerPanelViewer::setConvergingPoint(int pos)
{

}

void BridgerPanelViewer::setOpeningType(int index)
{

}
