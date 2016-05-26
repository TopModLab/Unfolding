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
	origamiConfig["testValue1"] = (double)ui->slider1->value() / (double)ui->slider1->maximum();
	origamiConfig["testValue2"] = (double)ui->slider2->value() / (double)ui->slider2->maximum();
}