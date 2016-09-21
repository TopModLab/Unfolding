#include "UI/WeavePanel.h"
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
	connect(ui->pivotSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setPivot(int)));
	connect(ui->flapSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_setFlap(int)));

}

WeavePanel::~WeavePanel()
{
	delete ui;
}

void WeavePanel::slot_setRoundness(int value)
{
	ui->roundLabel->setText(QString::number(
		value / static_cast<Float>(ui->roundSlider->maximum(), 'f', 2)));
}

void WeavePanel::slot_setThickness(int value)
{
	ui->thickLabel->setText(QString::number(
		value / static_cast<Float>(ui->thickSlider->maximum(), 'f', 2)));
}

void WeavePanel::slot_setPivot(int value)
{
	ui->pivotLabel->setText(QString::number(
		value/static_cast<Float>(ui->pivotSlider->maximum(), 'f', 2)));
}

void WeavePanel::slot_setFlap(int value)
{
	ui->flapLabel->setText(QString::number(
		value/static_cast<Float>(ui->flapSlider->maximum(), 'f', 2)));
}

void WeavePanel::slot_setType()
{
	config["shapeCone"] = static_cast<Float>(ui->shapeConeBtn->isChecked());
	config["scaleBilinear"] = static_cast<Float>(ui->scaleBilinearBtn->isChecked());
	config["roundness"] = ui->roundSlider->value()
						/ static_cast<Float>(ui->roundSlider->maximum());
	config["thickness"] = ui->thickSlider->value()
						/ static_cast<Float>(ui->thickSlider->maximum());
	config["depth"] = static_cast<Float>(ui->depthSpinBox->value());
	config["pivot"] = (ui->pivotSlider->value()
					/ static_cast<Float>(ui->pivotSlider->maximum()) + 1.0f) / 2.0f;
	config["flap"] = ui->flapSlider->value()
					/ static_cast<Float>(ui->flapSlider->maximum());
}
