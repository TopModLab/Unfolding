#include "WeavePanel.h"
#include "ui_WeavePanel.h"
#include <QDebug>

WeavePanel::WeavePanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::WeavePanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Weave Panel"));

	connect(ui->okBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->okBtn, SIGNAL(clicked()), this, SLOT(slot_setType()));
	connect(ui->okBtn, SIGNAL(clicked()), this, SIGNAL(sig_saved()));

	connect(ui->bridgerBtn, SIGNAL(clicked()), this, SIGNAL(sig_setBridger()));

	connect(ui->roundSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setRoundness(int)));
	connect(ui->thickSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setThickness(int)));

}

WeavePanel::~WeavePanel()
{
	delete ui;
}

void WeavePanel::slot_setRoundness(int value)
{
	ui->roundLabel->setText(QString::number((float)value/ui->roundSlider->maximum(), 'f', 2));
}

void WeavePanel::slot_setThickness(int value)
{
	ui->thickLabel->setText(QString::number((float)value/ui->thickSlider->maximum(), 'f', 2));
}

void WeavePanel::slot_setType()
{
	config["roundness"] = (float)ui->roundSlider->value()/ui->roundSlider->maximum();
	config["thickness"] = (float)ui->thickSlider->value()/ui->thickSlider->maximum();
	config["depth"] = (float)ui->depthSpinBox->value();
}
