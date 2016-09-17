#ifndef RIMFACEPANEL_H
#define RIMFACEPANEL_H

#include "Utils/common.h"
#include <QWidget>

namespace Ui {
class RimFacePanel;
}

class RimFacePanel : public QWidget
{
	Q_OBJECT

public:

	explicit RimFacePanel(QWidget *parent = 0);
	~RimFacePanel();

	confMap getConfig() const { return config; }

signals:
	void sig_saved();
	void sig_setBridger();

public slots:
	void slot_setW(int);
	void slot_setH(int);
	void slot_setFlap(int);
    void slot_setType();


private:
	Ui::RimFacePanel *ui;
	float width;
	float height;
	float flap;
	confMap config;
};


#endif // RIMFACEPANEL_H
