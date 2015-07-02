#ifndef CONNECTORPANEL_H
#define CONNECTORPANEL_H

#include <QWidget>
#include "connectorpanelviewer.h"

namespace Ui {
class ConnectorPanel;
}

class ConnectorPanel : public QWidget
{
	Q_OBJECT

public:
	explicit ConnectorPanel(QWidget *parent = 0);
	~ConnectorPanel();

signals:



private:
	Ui::ConnectorPanel *ui;

	ConnectorPanelViewer *viewer;
};

#endif // CONNECTORPANEL_H
