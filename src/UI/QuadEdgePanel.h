#pragma once
#ifndef QUADEDGEPANEL_H
#define QUADEDGEPANEL_H
#include "Utils/common.h"
#include <QWidget>

namespace Ui {
class QuadEdgePanel;
}

class QuadEdgePanel : public QWidget
{
	Q_OBJECT

public:
	explicit QuadEdgePanel(QWidget *parent = 0);
	~QuadEdgePanel();

	Float getFlapSize();
	Float getBridgerSize();
	Float getShift();
	int getType();

signals:
	void sig_saved();
	void sig_setBridger();

public slots:
	void slot_setbridgerLabel(int);
	void slot_setflapLabel(int);
	void slot_setshiftLabel(int);

	void slot_saved();
	void setPanelType(int);

private:
	Ui::QuadEdgePanel *ui;
	int flapType; //0 for one flap, 1 for extended multiple flaps
	int flapSize;
	int bridgerSize;
	int shiftAmount;
};

#endif // QUADEDGEPANEL_H
