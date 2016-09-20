#include "NeoWeavePanel.h"
#include "ui_NeoWeavePanel.h"

NeoWeavePanel::NeoWeavePanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::NeoWeavePanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Weave Panel"));
	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(slot_setConfig()));
	connect(ui->okButton, SIGNAL(clicked()), this, SIGNAL(sig_saved()));

	connect(ui->patchSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setPatchSize(int)));

}

NeoWeavePanel::~NeoWeavePanel()
{
	delete ui;
}

void NeoWeavePanel::slot_setPatchSize(int value)
{
	ui->patchSizeLabel->setText(QString::number((double)value / ui->patchSizeSlider ->maximum(), 'f', 2));
}

void NeoWeavePanel::slot_setConfig()
{
	config["patchSize"] = (double)ui->patchSizeSlider->value() / ui->patchSizeSlider->maximum();
}