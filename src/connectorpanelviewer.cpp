#include "connectorpanelviewer.h"

#include <QMouseEvent>
#include <QMessageBox>

ConnectorPanelViewer::ConnectorPanelViewer(QWidget *parent)
	: QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{

	shapeIndex = 0;
	curvature = 5;
	nSamples = 3;
	size = 80;
	convergingPointPos = 0;
	openingIndex = 0;
}

ConnectorPanelViewer::~ConnectorPanelViewer()
{

}

static void qNormalizeAngle(int &angle)
{
	while (angle < 0)
		angle += 360 * 16;
	while (angle > 360)
		angle -= 360 * 16;
}

void ConnectorPanelViewer::setXRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != xRot) {
		xRot = angle;
		updateGL();
	}
}

void ConnectorPanelViewer::setYRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != yRot) {
		yRot = angle;
		updateGL();
	}
}

void ConnectorPanelViewer::setZRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != zRot) {
		zRot = angle;
		updateGL();
	}
}

void ConnectorPanelViewer::initializeGL()
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

void ConnectorPanelViewer::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -10.0);
	glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
	glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
	glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);
	draw();
}

void ConnectorPanelViewer::resizeGL(int width, int height)
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

void ConnectorPanelViewer::mousePressEvent(QMouseEvent *event)
{
	lastPos = event->pos();

}

void ConnectorPanelViewer::mouseMoveEvent(QMouseEvent *event)
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

void ConnectorPanelViewer::draw()
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

void ConnectorPanelViewer::setShape(int)
{

}

void ConnectorPanelViewer::setCurvature(int curv)
{

}

void ConnectorPanelViewer::setSamples(int n)
{

}

void ConnectorPanelViewer::setSize(int size)
{

}

void ConnectorPanelViewer::setConvergingPoint(int pos)
{

}

void ConnectorPanelViewer::setOpeningType(int index)
{

}
