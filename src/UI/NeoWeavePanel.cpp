#include "NeoWeavePanel.h"
#include "ui_NeoWeavePanel.h"

NeoWeavePanel::NeoWeavePanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::NeoWeavePanel)
{
	size = 2;//default object as 2 inch
	ui->setupUi(this);
	setWindowTitle(tr("Weave Panel"));
	connect(ui->okButton, &QPushButton::clicked, this, &QWidget::close);
	connect(ui->okButton, &QPushButton::clicked, this, &NeoWeavePanel::setConfig);
	connect(ui->okButton, &QPushButton::clicked, this, &NeoWeavePanel::sig_saved);

	connect(ui->patchSizeSlider, &QSlider::valueChanged, [&](int value) {
		ui->patchSizeSpinBox->setValue(value);
	});
	connect(ui->patchSizeSpinBox,
		static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[&](double value) { 
		ui->patchSizeSlider->setValue(value);
	});

    // Layer Offset Value
    connect(ui->layerOffsetSlider, &QSlider::valueChanged,
        [&](int value) { ui->layerOffsetSpinBox->setValue(value * 0.01f * size); });
    connect(ui->layerOffsetSpinBox,
        static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        [&](double value) { ui->layerOffsetSlider->setValue(value * 100.0f /size); });

    // Strip Scaling Value
    connect(ui->stripScaleSlider, &QSlider::valueChanged,
        [&](int value) { ui->stripScaleSpinBox->setValue( value * 0.01f); });
    connect(ui->stripScaleSpinBox,
        static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        [&](double value) { ui->stripScaleSlider->setValue(value * 100.0f); });

	// Strip Width Value
	connect(ui->stripWidthSlider, &QSlider::valueChanged,
		[&](int value) { ui->stripWidthSpinBox->setValue(value * 0.01f); });
	connect(ui->stripWidthSpinBox,
		static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[&](double value) { ui->stripWidthSlider->setValue(value * 100.0f); });
}

NeoWeavePanel::~NeoWeavePanel()
{
	delete ui;
}


void NeoWeavePanel::setConfig()
{
	config["patchScale"]      = ui->patchSizeSlider->value()
		                      / static_cast<Float>(ui->patchSizeSlider->maximum());
	config["patchUniform"]    = static_cast<Float>(ui->uniformSizeBtn->isChecked());
    config["layerOffset"]     = static_cast<Float>(ui->layerOffsetSpinBox->value());
    config["patchStripScale"] = static_cast<Float>(ui->stripScaleSpinBox->value());
	config["patchStripWidth"] = ui->stripWidthSlider->value()
								/ static_cast<Float>(ui->stripWidthSlider->maximum());
}