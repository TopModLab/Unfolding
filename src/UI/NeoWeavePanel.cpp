#include "NeoWeavePanel.h"
#include "ui_NeoWeavePanel.h"

NeoWeavePanel::NeoWeavePanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::NeoWeavePanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Weave Panel"));
	connect(ui->okButton, &QPushButton::clicked, this, &QWidget::close);
	connect(ui->okButton, &QPushButton::clicked, this, &NeoWeavePanel::setConfig);
	connect(ui->okButton, &QPushButton::clicked, this, &NeoWeavePanel::sig_saved);

	connect(ui->patchSizeSlider, &QSlider::valueChanged, [&](int value) {
		ui->patchSizeSpinBox->setValue(
			value / static_cast<double>(ui->patchSizeSlider->maximum()));
	});

	connect(ui->patchSizeSpinBox,
		static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[&](double value) { 
		ui->patchSizeSlider->setValue(
			value * (ui->patchSizeSlider->maximum()));
	});


}

NeoWeavePanel::~NeoWeavePanel()
{
	delete ui;
}


void NeoWeavePanel::setConfig()
{
	config["patchScale"] = ui->patchSizeSlider->value()
		/ static_cast<Float>(ui->patchSizeSlider->maximum());
	config["patchUniform"] = static_cast<Float>(ui->uniformSizeBtn->isChecked());
    config["LayerOffset"] = static_cast<Float>(ui->layerOffsetSpinBox->value());
    config["PatchStripLenScale"] = static_cast<Float>(ui->stripScaleSpinBox->value());
}