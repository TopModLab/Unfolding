#include "rimfacepanel.h"
#include "ui_rimfacepanel.h"
#include "common.h"

RimFacePanel::RimFacePanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::RimFacePanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Rim Face Panel"));

	width = (float)ui->widthSlider->value()/(float)ui->widthSlider->maximum();
	height = (float)ui->heightSlider->value()/(float)ui->heightSlider->maximum();
	cout<<ui->widthSlider->value()<<endl;
	cout<<ui->widthSlider->maximum()<<endl;

	ui->widthText->setText(QString::number(width));
	ui->heightText->setText(QString::number(height));

	connect(ui->okBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->okBtn, SIGNAL(clicked()), this, SIGNAL(sig_saved()));
	connect(ui->setBridgerBtn, SIGNAL(clicked()), this, SIGNAL(sig_setBridger()));

	connect(ui->widthSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setW(int)));
	connect(ui->heightSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setH(int)));

}

RimFacePanel::~RimFacePanel()
{
	delete ui;
}

void RimFacePanel::slot_setW(int value)
{
	width = (float)value/ui->widthSlider->maximum();
	ui->widthText->setText(QString::number(width));

}

void RimFacePanel::slot_setH(int value)
{
	height = (float)value/ui->heightSlider->maximum();
	ui->heightText->setText(QString::number(height));

}

