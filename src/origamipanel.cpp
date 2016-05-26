#include "OrigamiPanel.h"
#include "ui_OrigamiPanel.h"

OrigamiPanel::OrigamiPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::OrigamiPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Origami Panel"));

	ui->image->setPixmap(QPixmap(":icons/bezier.png"));


	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(slot_saved()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SIGNAL(sig_canceled()));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));

	connect(ui->shapeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_restrainSliders(int)));

}

OrigamiPanel::~OrigamiPanel()
{
	delete ui;
}

void OrigamiPanel::setSaveMode(bool isExtend)
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

void OrigamiPanel::slot_restrainSliders(int index)
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

void OrigamiPanel::slot_saved()
{
	 origamiConfig["shape"] = ui->shapeComboBox->currentIndex();
	 origamiConfig["curv"] = (double)ui->curvSlider->value()/(double)ui->curvSlider->maximum();
	 origamiConfig["samples"] = ui->samplesSlider->value();
	 origamiConfig["size"] = (double)ui->sizeSlider->value()/(double)ui->sizeSlider->maximum();
	 origamiConfig["cp"] = (double)ui->convergeSlider->value()/(double)ui->convergeSlider->maximum();

}


