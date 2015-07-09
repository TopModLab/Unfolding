#ifndef CONNECTORPANEL_H
#define CONNECTORPANEL_H

#include <QWidget>
#include <map>
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
	std::map<QString, double> getConfigValues();

signals:
	void sig_saved();
	void sig_canceled();

public slots:
	void slot_saved();
	void slot_restrainSliders(int);

private:
	Ui::ConnectorPanel *ui;
	ConnectorPanelViewer *viewer;
	std::map<QString,double> connectorConfig;

private:
	void closeEvent(QCloseEvent *e);

};

#endif // CONNECTORPANEL_H
