#pragma once
#ifndef _CONNECTORSELECTIONPANEL_
#define _CONNECTORSELECTIONPANEL_

#include <QWidget>
#include <QDialog>
#include <QFileDialog>
#include <QString>
#include "ui_ConnectorSelectionPanel.h"

class ConnectorSelectionPanel : public QDialog
{
	Q_OBJECT

public:
	ConnectorSelectionPanel(QDialog *parent = nullptr);
	~ConnectorSelectionPanel();

	void setMeshType(int mt);
	QString getFilename() const;
private slots:
	void slot_getFileName();
	void slot_savePanelData(QAbstractButton* button);
private:
	Ui::ConnectorSelectionPanel *ui;
	int meshType;
	int connectorType;
	double scaleSize();
};


#endif