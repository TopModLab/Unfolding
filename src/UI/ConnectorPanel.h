#pragma once
#ifndef _ConnectorPanel_
#define _ConnectorPanel_

#include "Utils/common.h"

#include <QWidget>
#include <QDialog>
#include <QFileDialog>
#include <QString>
#include "ui_ConnectorPanel.h"
#include "HDS/HDS_Mesh.h"
#include "GeomProc/MeshConnector.h"


class ConnectorPanel : public QDialog
{
	Q_OBJECT

public:
	ConnectorPanel(int meshProcType = HDS_Mesh::HALFEDGE_PROC);

	QString getFilename() const;
	confMap getConfig() const { return conf; }

	void resetParas(int procType);

	static QFont fontfamily;
	static Float fontSize;
signals:
	void sig_saved();
private:
	void setFileName();
	void save();
private:
	QScopedPointer<Ui::ConnectorPanel> ui;
	confMap conf;
};


#endif
