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

    void setType(int type) {this->type = type;}
    int getType() {return type;}
	float getW() { return width;}
	float getH() { return height;}

signals:
	void sig_saved();
	void sig_setBridger();

public slots:
	void slot_setW(int);
	void slot_setH(int);

private:
	Ui::RimFacePanel *ui;
	float width;
	float height;
    int type;
};

#endif // RIMFACEPANEL_H
