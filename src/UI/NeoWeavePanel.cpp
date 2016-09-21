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

	connect(ui->patchSizeSlider, &QSlider::valueChanged,
		this, &NeoWeavePanel::setPatchSize);
}

NeoWeavePanel::~NeoWeavePanel()
{
	delete ui;
}

void NeoWeavePanel::setPatchSize(int value)
{
	ui->patchSizeLabel->setText(QString::number(value
		/ static_cast<Float>(ui->patchSizeSlider->maximum()), 'f', 2));
}

void NeoWeavePanel::setConfig()
{
	config["patchSize"] = ui->patchSizeSlider->value()
		/ static_cast<Float>(ui->patchSizeSlider->maximum());
}