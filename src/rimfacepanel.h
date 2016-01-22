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

private:
	Ui::RimFacePanel *ui;
};

#endif // RIMFACEPANEL_H
