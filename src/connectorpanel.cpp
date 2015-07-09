#include "connectorpanel.h"
#include "ui_connectorpanel.h"
#include "connectorpanelviewer.h"


ConnectorPanel::ConnectorPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ConnectorPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Connector Panel"));

	viewer = new ConnectorPanelViewer(this);
	ui->viewerLayout->addWidget(viewer);

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(slot_saved()));
	connect(ui->buttonBox, SIGNAL(accepted()), this, SIGNAL(sig_saved()));

	connect(ui->buttonBox, SIGNAL(rejected()), this, SIGNAL(sig_canceled()));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));

	connect(ui->shapeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_restrainSliders(int)));

	connect(ui->curvSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setCurvature(int)));
	connect(ui->shapeComboBox, SIGNAL(currentIndexChanged(int)), viewer, SLOT(setShape(int)));
	connect(ui->samplesSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setSamples(int)));
	connect(ui->sizeSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setSize(int)));
	connect(ui->convergeSlider, SIGNAL(valueChanged(int)), viewer, SLOT(setConvergingPoint(int)));
	connect(ui->adhesiveComboBox, SIGNAL(currentIndexChanged(int)), viewer, SLOT(setOpeningType(int)));

}

ConnectorPanel::~ConnectorPanel()
{
	delete ui;
}
void ConnectorPanel::slot_restrainSliders(int index)
{
	switch(index){
	case 0: // bezier
		ui->curvSlider->setEnabled(true);
		ui->samplesSlider->setEnabled(true);
		ui->convergeSlider->setEnabled(true);
		break;
	case 1: // original
		ui->curvSlider->setEnabled(false);
		ui->samplesSlider->setEnabled(false);
		ui->convergeSlider->setEnabled(false);

		ui->curvSlider->setValue(ui->curvSlider->maximum());
		ui->samplesSlider->setValue(2);
		ui->convergeSlider->setValue(0);
		break;
	case 2: // flat
		ui->curvSlider->setEnabled(false);
		ui->samplesSlider->setEnabled(false);
		ui->convergeSlider->setEnabled(false);

		ui->curvSlider->setValue(ui->curvSlider->minimum());
		ui->samplesSlider->setValue(1);
		ui->convergeSlider->setValue(0);
		break;
	}
}

void ConnectorPanel::slot_saved()
{
	 connectorConfig["shape"] = ui->shapeComboBox->currentIndex();
	 connectorConfig["curv"] = (double)ui->curvSlider->value()/(double)ui->curvSlider->maximum();
	 connectorConfig["samples"] = ui->samplesSlider->value();
	 connectorConfig["size"] = (double)ui->sizeSlider->value()/(double)ui->sizeSlider->maximum();
	 connectorConfig["cp"] = (double)ui->convergeSlider->value()/(double)ui->convergeSlider->maximum();
	 connectorConfig["opening"] = ui->adhesiveComboBox->currentIndex();

}

std::map<QString, double> ConnectorPanel::getConfigValues()
{
	return connectorConfig;
}

void ConnectorPanel::closeEvent(QCloseEvent *e)
{
	close();
	emit sig_canceled();
}
