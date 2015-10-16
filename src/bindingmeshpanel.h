#ifndef BINDINGMESHPANEL_H
#define BINDINGMESHPANEL_H

#include <QWidget>

namespace Ui {
class BindingMeshPanel;
}

class BindingMeshPanel : public QWidget
{
	Q_OBJECT

public:
	explicit BindingMeshPanel(QWidget *parent = 0);
	~BindingMeshPanel();

	double getConnectorSize();

signals:
	void sig_saved();
	void sig_setConnector(bool);

public slots:
	void slot_saved();

private:
	Ui::BindingMeshPanel *ui;
	int connectorSize;

};

#endif // BINDINGMESHPANEL_H
