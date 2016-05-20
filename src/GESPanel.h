#ifndef GESPANEL_H
#define GESPANEL_H

#include <QWidget>

namespace Ui {
class GESPanel;
}

class GESPanel : public QWidget
{
	Q_OBJECT

public:
	explicit GESPanel(QWidget *parent = 0);
	~GESPanel();

	double getBridgerSize();
	void setPanelType(int);
signals:
	void sig_saved();
	void sig_setBridger();

public slots:
	void slot_saved();
	void slot_setSize(int);

private:
	Ui::GESPanel *ui;
	int bridgerSize;
	int panelType;
};

#endif // GRSPANEL_H
