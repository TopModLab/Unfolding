#pragma once
#ifndef _CONNECTORSELECTIONPANEL_
#define _CONNECTORSELECTIONPANEL_

#include <QWidget>
#include "ui_ConnectorSelectionPanel.h"

class ConnectorSelectionPanel : public QWidget
{
	Q_OBJECT

public:
	ConnectorSelectionPanel(QWidget *parent  = nullptr);
	~ConnectorSelectionPanel();

private:
	Ui::ConnectorSelectionPanel *ui;
	int meshType;
	int connectorType;
};


#endif