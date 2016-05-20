#pragma once
#ifndef _ConnectorPanel_
#define _ConnectorPanel_

#include "common.h"

#include <QWidget>
#include <QDialog>
#include <QFileDialog>
#include <QString>
#include "ui_ConnectorPanel.h"
#include "hds_mesh.h"
#include "MeshConnector.h"


class ConnectorPanel : public QDialog
{
	Q_OBJECT

public:
	ConnectorPanel(int meshProcType = HDS_Mesh::REGULAR_PROC);

	void setMeshConfigure();
	QString getFilename() const;
	//double getScale() const;
	//int getConnectorType() const;
	confMap getConfig() const;

	void resetParas(int procType);

	static QFont fontfamily;
private:
	//void slot_setScaleToSpinbox(int val);
	//void slot_setScaleToSlider(double val);
	void setFileName();
private:
	QScopedPointer<Ui::ConnectorPanel> ui;
	confMap conf;
	//int meshType;
	//int connectorType;
};


#endif
