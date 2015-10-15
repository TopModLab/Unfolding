#include "bindingmeshpanel.h"
#include "ui_bindingmeshpanel.h"

BindingMeshPanel::BindingMeshPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BindingMeshPanel)
{
    ui->setupUi(this);
}

BindingMeshPanel::~BindingMeshPanel()
{
    delete ui;
}
