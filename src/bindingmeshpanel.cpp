#include "bindingmeshpanel.h"
#include "ui_bindingmeshpanel.h"

BindingMeshPanel::BindingMeshPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::BindingMeshPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Bind Mesh Panel"));

	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(slot_saved()));
	connect(ui->okButton, SIGNAL(clicked()), this, SIGNAL(sig_saved()));
	connect(ui->bridgerPanelButton, SIGNAL(clicked(bool)), this, SIGNAL(sig_setBridger(bool)));

}

BindingMeshPanel::~BindingMeshPanel()
{
	delete ui;
}

void BindingMeshPanel::slot_saved()
{
	bridgerSize = ui->sizeSlider->value();
	close();
}

double BindingMeshPanel::getBridgerSize()
{
	return 1.0 -(double)bridgerSize / (double)ui->sizeSlider->maximum();
}
