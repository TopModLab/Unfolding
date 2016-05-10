#include "rimfacepanel.h"
#include "ui_rimfacepanel.h"
#include "common.h"

RimFacePanel::RimFacePanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::RimFacePanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Rim Face Panel"));
	width = (float)ui->roundSlider->value()/(float)ui->roundSlider->maximum();
	height = (float)ui->heightSlider->value()/(float)ui->heightSlider->maximum();
	flap = (float)ui->flapSlider->value()/(float)ui->flapSlider->maximum();

	ui->widthText->setText(QString::number(width));
	ui->heightText->setText(QString::number(height));
	ui->flapText->setText(QString::number(flap));

	ui->parallelBtn->hide();

	//set buttongroup id
	ui->pieceCenterGrp->setId(ui->centerVBtn, 0);
	ui->pieceCenterGrp->setId(ui->centerFBtn, 1);

	ui->bezierGrp->setId(ui->quaBtn, 0);
	ui->bezierGrp->setId(ui->cubicBtn, 1);
	ui->piecePositionGrp->setId(ui->edgeBtn, 0);
	ui->piecePositionGrp->setId(ui->faceBtn, 1);
	ui->pieceSizeGrp->setId(ui->halfBtn, 0);
	ui->pieceSizeGrp->setId(ui->wholeBtn, 1);
	ui->pieceShapeGrp->setId(ui->trapeBtn, 0);
	ui->pieceShapeGrp->setId(ui->diamBtn, 1);
	ui->pieceShapeGrp->setId(ui->parallelBtn, 2);


	ui->smoothBox->hide();

	connect (ui->pieceCenterGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
			[=](int id){
		 switch(id) {
		 case 0:
			 ui->bezierGrpBox->show();
			 ui->pieceGrpBox->show();
			 ui->optionsGrpBox->show();
			 break;
		 case 1:
			 ui->bezierGrpBox->hide();
			 ui->pieceGrpBox->hide();
			 ui->optionsGrpBox->hide();
			 break;
		 }
	});
	connect(ui->pieceShapeGrp,static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
			[=](int id){
		switch(id) {
		case 0:
			ui->pieceSizeGrpBox->show();
			ui->optionsGrpBox->show();
			break;
		case 1:
			ui->pieceSizeGrpBox->hide();
			ui->quaBtn->setChecked(true);
			ui->optionsGrpBox->hide();

			break;
		case 2:
			ui->pieceSizeGrpBox->hide();
			ui->cubicBtn->setChecked(true);
			ui->optionsGrpBox->hide();

			break;

		}

	});

	connect(ui->piecePositionGrp,static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
			[=](int id){
		if (id == 0) {
			ui->optionsGrpBox->hide();
			ui->intersectionBox->setChecked(false);
		}else {
			if (!ui->quaBtn->isChecked())
			ui->optionsGrpBox->show();
			if (!ui->halfBtn->isChecked())
			ui->intersectionBox->show();

		}
	});

	connect(ui->pieceSizeGrp,static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
			[=](int id){
		if (id == 1) {
			ui->connectorBox->setEnabled(false);
			ui->connectorBox->setChecked(false);
			if (!ui->faceBtn->isChecked()) {
				ui->intersectionBox->show();
			}

		}else {
			ui->connectorBox->setEnabled(true);
			ui->intersectionBox->hide();
			ui->intersectionBox->setChecked(false);

		}
	});

	connect(ui->bezierGrp,static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
			[=](int id){
		if (id == 1) {
			if (!ui->faceBtn->isChecked()) {
				ui->smoothBox->show();
			}
		}else {
			ui->smoothBox->hide();
			ui->smoothBox->setChecked(false);
		}
	});

	connect(ui->okBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->okBtn, SIGNAL(clicked()), this, SLOT(slot_setType()));
	connect(ui->okBtn, SIGNAL(clicked()), this, SIGNAL(sig_saved()));

	connect(ui->setBridgerBtn, SIGNAL(clicked()), this, SIGNAL(sig_setBridger()));

	connect(ui->roundSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setW(int)));
	connect(ui->heightSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setH(int)));
	connect(ui->flapSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setFlap(int)));
}

RimFacePanel::~RimFacePanel()
{
	delete ui;
}


void RimFacePanel::slot_setW(int value)
{
	width = (float)value/ui->roundSlider->maximum();
	ui->widthText->setText(QString::number(width));

}

void RimFacePanel::slot_setH(int value)
{
	height = (float)value/ui->heightSlider->maximum();
	ui->heightText->setText(QString::number(height));

}

void RimFacePanel::slot_setFlap(int value)
{
	flap = (float)value/ui->flapSlider->maximum();
	ui->flapText->setText(QString::number(flap));

}

void RimFacePanel::slot_setType()
{
	config["center"] = ui->pieceCenterGrp->checkedId();
	if (config["center"] == 0) {
		config["type"] = ui->pieceShapeGrp->checkedId();

		config["onEdge"] = ui->edgeBtn->isChecked();
		config["isHalf"] = ui->halfBtn->isChecked();
		config["isQuadratic"] = ui->quaBtn->isChecked();
		config["addConnector"] = ui->connectorBox->isChecked();
		config["smoothEdge"] = ui->smoothBox->isChecked();
		config["avoidIntersect"] = ui->intersectionBox->isChecked();
	}else {
		config["thicknessOfCon"] = flap;
	}
	config["roundness"] = width;
	config["thicknessOfBridger"] = height;

}
