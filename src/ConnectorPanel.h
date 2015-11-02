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
	ConnectorPanel(int mesh_process_type = HDS_Mesh::REGULAR_PROC);
	~ConnectorPanel();

	void setMeshConfigure();
	QString getFilename() const;
	double getScale() const;
	int getConnectorType() const;
	unordered_map<ConnectorConf, double> getConfiguration() const;
private:
	void initConnectorType();
private slots:
	//void slot_setScaleToSpinbox(int val);
	//void slot_setScaleToSlider(double val);
	void slot_setFileName();
	void slot_savePanelData(QAbstractButton* button);
private:
	Ui::ConnectorPanel *ui;
	int meshType;
	int connectorType;
	//double scale;
};


#endif