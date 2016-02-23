#include "bridgerpanel.h"
#include "ui_bridgerpanel.h"
//#include "bridgerpanelviewer.h"


BridgerPanel::BridgerPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::BridgerPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Bridger Panel"));

	ui->image->setPixmap(QPixmap(":icons/bezier.png"));

	//viewer = new BridgerPanelViewer(this);

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(slot_saved()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SIGNAL(sig_canceled()));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));

	connect(ui->shapeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_restrainSliders(int)));

	/*
	connect(ui->curvSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setCurvature(int)));
	connect(ui->shapeComboBox, SIGNAL(currentIndexChanged(int)), viewer, SLOT(setShape(int)));
	connect(ui->samplesSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setSamples(int)));
	connect(ui->sizeSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setSize(int)));
	connect(ui->convergeSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setConvergingPoint(int)));
	connect(ui->adhesiveComboBox, SIGNAL(currentIndexChanged(int)), viewer, SLOT(setOpeningType(int)));
	*/
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
	case 2: // flat
		ui->curvSlider->hide();
		ui->samplesSlider->hide();
		ui->convergeSlider->hide();
		ui->curvLabel->hide();
		ui->sampleLabel->hide();
		ui->convergeLabel->hide();
		ui->image->setPixmap(QPixmap(":icons/flat.png"));
		break;
	}
}

void BridgerPanel::slot_saved()
{
	 bridgerConfig["shape"] = ui->shapeComboBox->currentIndex();
	 bridgerConfig["curv"] = (double)ui->curvSlider->value()/(double)ui->curvSlider->maximum();
	 bridgerConfig["samples"] = ui->samplesSlider->value();
	 bridgerConfig["size"] = (double)ui->sizeSlider->value()/(double)ui->sizeSlider->maximum();
	 bridgerConfig["cp"] = (double)ui->convergeSlider->value()/(double)ui->convergeSlider->maximum();
	 bridgerConfig["opening"] = ui->adhesiveComboBox->currentIndex();

}

std::map<QString, double> BridgerPanel::getConfigValues()
{
	return bridgerConfig;
}


