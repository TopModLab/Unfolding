#include "UI/BridgerPanel.h"
#include "ui_BridgerPanel.h"

BridgerPanel::BridgerPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::BridgerPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Bridger Panel"));

	ui->image->setPixmap(QPixmap(":icons/bezier.png"));


	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(slot_saved()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SIGNAL(sig_canceled()));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));

	connect(ui->shapeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_restrainSliders(int)));

}

BridgerPanel::~BridgerPanel()
{
	delete ui;
}

void BridgerPanel::setSaveMode(bool isExtend)
{
	//reset old signal emissions on buttonBox
	ui->buttonBox->disconnect(SIGNAL(accepted()));
	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(slot_saved()));


	if (isExtend){
		connect(ui->buttonBox, SIGNAL(accepted()), this, SIGNAL(sig_save2extend()));
	}
	else
		connect(ui->buttonBox, SIGNAL(accepted()), this, SIGNAL(sig_saved()));

}

void BridgerPanel::slot_restrainSliders(int index)
{
	switch(index){
	case 0: // bezier
		ui->curvSlider->show();
		ui->samplesSlider->show();
		ui->convergeSlider->show();
		ui->curvLabel->show();
		ui->sampleLabel->show();
		ui->convergeLabel->show();
		ui->image->setPixmap(QPixmap(":icons/bezier.png"));
		break;
	case 1: // original
		ui->curvSlider->hide();
		ui->samplesSlider->hide();
		ui->convergeSlider->hide();
		ui->curvLabel->hide();
		ui->sampleLabel->hide();
		ui->convergeLabel->hide();
		ui->image->setPixmap(QPixmap(":icons/original.png"));
		break;
	}
}

void BridgerPanel::slot_saved()
{
	 bridgerConfig["shape"] = ui->shapeComboBox->currentIndex();
	 bridgerConfig["curv"] = ui->curvSlider->value()
		 / static_cast<Float>(ui->curvSlider->maximum());
	 bridgerConfig["samples"] = ui->samplesSlider->value();
	 bridgerConfig["size"] = ui->sizeSlider->value()
		 / static_cast<Float>(ui->sizeSlider->maximum());
	 bridgerConfig["cp"] = ui->convergeSlider->value()
		 / static_cast<Float>(ui->convergeSlider->maximum());

}


