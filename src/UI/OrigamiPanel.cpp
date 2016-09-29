#include "OrigamiPanel.h"
#include "ui_OrigamiPanel.h"

OrigamiPanel::OrigamiPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::OrigamiPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Origami Panel"));
	connect(ui->okButton, &QPushButton::clicked, this, &QWidget::close);
	connect(ui->okButton, &QPushButton::clicked, this, &OrigamiPanel::setConfig);
	connect(ui->okButton, &QPushButton::clicked, this, &OrigamiPanel::sig_saved);

	connect(ui->sizeSlider, &QSlider::valueChanged, [&](int value) {
		ui->sizeSpinBox->setValue(
			value / static_cast<double>(ui->sizeSlider->maximum()));
	});
	connect(ui->depthSlider, &QSlider::valueChanged, [&](int value) {
		ui->depthSpinBox->setValue(
			value / static_cast<double>(ui->depthSlider->maximum()));
	});
	connect(ui->sizeSpinBox,
		static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[&](double value) {
		ui->sizeSlider->setValue(
			value * (ui->sizeSlider->maximum()));
	});
	connect(ui->depthSpinBox,
		static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[&](double value) {
		ui->depthSlider->setValue(
			value * (ui->depthSlider->maximum()));
	});
}

OrigamiPanel::~OrigamiPanel()
{
	delete ui;
}

void OrigamiPanel::setConfig()
{
	config["patchScale"] = ui->sizeSlider->value()
		/ static_cast<Float>(ui->sizeSlider->maximum());
	config["foldDepth"] = ui->depthSlider->value()
		/ static_cast<Float>(ui->depthSlider->maximum());
}