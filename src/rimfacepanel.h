#ifndef RIMFACEPANEL_H
#define RIMFACEPANEL_H

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

    std::map<QString,bool> getConfig() {return config;}
	float getW() { return width;}
	float getH() { return height;}

signals:
	void sig_saved();
	void sig_setBridger();

public slots:
	void slot_setW(int);
	void slot_setH(int);
    void slot_setType();


private:
	Ui::RimFacePanel *ui;
	float width;
	float height;
    std::map<QString,bool> config;
};

#endif // RIMFACEPANEL_H
