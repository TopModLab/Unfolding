#ifndef GRSPANEL_H
#define GRSPANEL_H

#include <QWidget>

namespace Ui {
class GRSPanel;
}

class GRSPanel : public QWidget
{
	Q_OBJECT

public:
	explicit GRSPanel(QWidget *parent = 0);
	~GRSPanel();

	double getBridgerSize();
	void setPanelType(int);
signals:
	void sig_saved();
	void sig_setBridger();

public slots:
	void slot_saved();
	void slot_setSize(int);

private:
	Ui::GRSPanel *ui;
	int bridgerSize;
	int panelType;
};

#endif // GRSPANEL_H
