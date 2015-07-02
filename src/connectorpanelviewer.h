#ifndef CONNECTORPANELVIEWER_H
#define CONNECTORPANELVIEWER_H

#include <QGLWidget>

class ConnectorPanelViewer : public QGLWidget
{
	Q_OBJECT
public:
	explicit ConnectorPanelViewer(QWidget *parent = 0);
	~ConnectorPanelViewer();

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);

	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);


public slots:
	void setShape(int index);
	void setCurvature(int curv);
	void setSamples(int n);
	void setSize(int size);
	void setConvergingPoint(int pos);
	void setOpeningType(int index);

private:
	void setXRotation(int angle);
	void setYRotation(int angle);
	void setZRotation(int angle);
	void draw();

private:
	int shapeIndex;
	int curvature;
	int nSamples;
	int size;
	int convergingPointPos;
	int openingIndex;

	QPoint lastPos;

	int xRot;
	int yRot;
	int zRot;

};

#endif // CONNECTORPANELVIEWER_H
