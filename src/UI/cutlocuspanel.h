#ifndef CUTLOCUSPANEL_H
#define CUTLOCUSPANEL_H

#include <QWidget>

namespace Ui {
class CutLocusPanel;
}

class CutLocusPanel : public QWidget
{
	Q_OBJECT

public:
	explicit CutLocusPanel(QWidget *parent = 0);
	~CutLocusPanel();

	bool isMinMaxChecked();
signals:
	void sig_methodChanged(int);
	void sig_displayMinMax(int);
	void sig_toggleCutMode(int);
	void sig_closedSignal();
	void sig_toggleCut();

public slots:
	void slot_toggleCut(int);
private:
	void closeEvent(QCloseEvent *e);
private:
	Ui::CutLocusPanel *ui;
};

#endif // CUTLOCUSPANEL_H
