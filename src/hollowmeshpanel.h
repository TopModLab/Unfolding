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
	double getShift();
	int getType();

signals:
	void sig_saved();
	void sig_setConnector(bool);

public slots:
	void slot_saved();
	void slot_restrainSliders(int);

private:
	Ui::HollowMeshPanel *ui;
	int flapType; //0 for one flap, 1 for extended multiple flaps
	int flapSize;
	int connectorSize;
	int shiftAmount;
};

#endif // HOLLOWMESHPANEL_H
