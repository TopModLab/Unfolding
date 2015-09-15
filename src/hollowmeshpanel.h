#ifndef HOLLOWMESHPANEL_H
#define HOLLOWMESHPANEL_H

#include <QWidget>

namespace Ui {
class HollowMeshPanel;
}

class HollowMeshPanel : public QWidget
{
	Q_OBJECT

public:
	explicit HollowMeshPanel(QWidget *parent = 0);
	~HollowMeshPanel();

	double getFlapSize();
	double getConnectorSize();

signals:
	void sig_saved();

public slots:
	void slot_saved();

private:
	Ui::HollowMeshPanel *ui;
	int flapSize;
	int connectorSize;
};

#endif // HOLLOWMESHPANEL_H
