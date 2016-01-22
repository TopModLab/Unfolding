#include "rimfacepanel.h"
#include "ui_rimfacepanel.h"

RimFacePanel::RimFacePanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::RimFacePanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Rim Face Panel"));


}

RimFacePanel::~RimFacePanel()
{
	delete ui;
}
