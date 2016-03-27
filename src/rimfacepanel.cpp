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
	ui->widthText->setText(QString::number(width));
	ui->heightText->setText(QString::number(height));

	//set buttongroup id
	ui->bezierGrp->setId(ui->quaBtn, 0);
	ui->bezierGrp->setId(ui->cubicBtn, 1);
	ui->piecePositionGrp->setId(ui->edgeBtn, 0);
	ui->piecePositionGrp->setId(ui->faceBtn, 1);
	ui->pieceSizeGrp->setId(ui->halfBtn, 0);
	ui->pieceSizeGrp->setId(ui->wholeBtn, 1);

	connect(ui->piecePositionGrp,static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
			[=](int id){
		if (id == 1) {
			ui->smoothBox->setEnabled(false);
		}else {
			ui->smoothBox->setEnabled(true);
		}
	});

	connect(ui->pieceSizeGrp,static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
			[=](int id){
		if (id == 1) {
			ui->connectorBox->setEnabled(false);
		}else {
			ui->connectorBox->setEnabled(true);
		}
	});

	connect(ui->okBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->okBtn, SIGNAL(clicked()), this, SLOT(slot_setType()));
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

void RimFacePanel::slot_setType()
{
	config["onEdge"] = ui->piecePositionGrp->checkedId() == 0? true:false;
	config["isHalf"] = ui->pieceSizeGrp->checkedId() == 0? true:false;
	config["isQuadratic"] = ui->bezierGrp->checkedId() == 0? true:false;
	config["addConnector"] = ui->connectorBox->isChecked();
	config["smoothEdge"] = ui->smoothBox->isChecked();

}
