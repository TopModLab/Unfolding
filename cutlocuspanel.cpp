#include "cutlocuspanel.h"
#include "ui_cutlocuspanel.h"

CutLocusPanel::CutLocusPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CutLocusPanel)
{
    ui->setupUi(this);
}

CutLocusPanel::~CutLocusPanel()
{
    delete ui;
}
