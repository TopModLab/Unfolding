#include "origamipanel.h"
#include "ui_origamipanel.h"

OrigamiPanel::OrigamiPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrigamiPanel)
{
    ui->setupUi(this);
	setWindowTitle(tr("Origami Panel"));
	
	ui->image->setPixmap(QPixmap(":icons/origami.png"));

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(slot_saved()));
	connect(ui->buttonBox, SIGNAL(accepted()), this, SIGNAL(sig_save2origami()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SIGNAL(sig_canceled()));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));


}

OrigamiPanel::~OrigamiPanel()
{
    delete ui;
}

void OrigamiPanel::slot_saved()
{
	origamiConfig["scale"] = (double)ui->slider_scale->value() / (double)(ui->slider_scale->maximum() + 1);
	origamiConfig["tucked_length"] = (double)ui->slider_tucked_length->value() / (double)ui->slider_tucked_length->maximum();
	origamiConfig["tucked_smooth"] = (double)ui->slider_tucked_smooth->value() / (double)ui->slider_tucked_smooth->maximum();
	origamiConfig["angle"] = (double)ui->slider_angle->value() / (double)(ui->slider_angle->maximum() + 1);
}