#include "ConnectorPanel.h"

QFont ConnectorPanel::fontfamily = QFont("Arial");

ConnectorPanel::ConnectorPanel(int procType)
	: ui(new Ui::ConnectorPanel)
{
	ui->setupUi(this);

	//setMeshConfigure();
	resetParas(procType);
	//setWindowTitle(tr("Mesh Export Panel"));
	connect(ui->file_button, &QAbstractButton::clicked, this, &ConnectorPanel::setFileName);
	//connect(ui->save_button, &QAbstractButton::clicked, this, &ConnectorPanel::savePanelData);
	//connect(ui->scale_slider, SIGNAL(valueChanged(int)), this, SLOT(slot_setScaleToSpinbox(int)));
	//connect(ui->scale_val, SIGNAL(valueChanged(double)), this, SLOT(slot_setScaleToSlider(double)));
}

void ConnectorPanel::setMeshConfigure()
{
	//meshType = mt;
	// Enable connector selection combobox
	/*switch (meshType)
	{
	case HDS_Mesh::REGULAR_PROC:
		ui->reg_conn_type->setDisabled(false);
		break;
	case HDS_Mesh::HOLLOWED_PROC:
		ui->hollow_conn_type->setDisabled(false);
		break;
	case HDS_Mesh::RIMMED_PROC:
		ui->rim_conn_type->setDisabled(false);
		break;
	default:
		break;
	}*/
}

QString ConnectorPanel::getFilename() const
{
	//cout << "File name is: " << ui->filename_text->text() << endl;
	return ui->filename_text->text();
}

/*
double ConnectorPanel::getScale() const
{
	return ui->scale_val->value();
}*/

/*
int ConnectorPanel::getConnectorType() const
{
	return ui->connector_type->currentIndex();
}*/

confMap ConnectorPanel::getConfig() const
{
	confMap ret;
	fontfamily = ui->font_val->currentFont();

	ret.insert(make_pair("connector", (double)ui->connector_type->currentIndex()));
	

	ret.insert(make_pair("scale", ui->scale_val->value()));
	ret.insert(make_pair("width", ui->width_val->value()));
	ret.insert(make_pair("length", ui->length_val->value()));

	ret.insert(make_pair("pinUnit", (double)ui->pinholeunit_type->currentIndex()));
	ret.insert(make_pair("pinSize", ui->pinholesize_val->value()));
	ret.insert(make_pair("pinCount",
		(double)ui->pinholecount_type->currentIndex()));

	ret.insert(make_pair("etchSeg", (double)ui->etchseg_val->value()));
	ret.insert(make_pair("scoreType", (double)ui->score_type->currentIndex()));
	ret.insert(make_pair("dashLen", ui->scoredash_len->value()));
	ret.insert(make_pair("dashGap", ui->scoredash_gap->value()));
	ret.insert(make_pair("dashUnit", (double)ui->scoredash_unit->currentIndex()));
	return ret;
}

void ConnectorPanel::resetParas(int procType)
{
	//meshType = procType;
	ui->mesh_type->setCurrentIndex(procType);
	switch (procType)
	{
	case HDS_Mesh::REGULAR_PROC:
		ui->connector_type->addItem("Simple");
		ui->connector_type->addItem("Insert");
		ui->connector_type->addItem("Gear");
		ui->connector_type->addItem("Saw");
		ui->connector_type->addItem("Advanced Saw");
		break;
	case HDS_Mesh::HOLLOWED_PROC:
	case HDS_Mesh::HOLLOWED_MF_PROC:
		//ui->pinholeunit_label->setDisabled(false);
		ui->pinholeunit_type->setDisabled(false);
		ui->pinholesize_label->setDisabled(false);
		ui->pinholesize_val->setDisabled(false);
		ui->connector_type->addItem("Holes");
		ui->connector_type->addItem("Faces");
		break;
	case HDS_Mesh::RIMMED_PROC:
		ui->connector_type->addItem("Arch");
		ui->connector_type->addItem("Ring");
		break;
	default:
		break;
	}
}

void ConnectorPanel::setFileName()
{
	QString filename = QFileDialog::getSaveFileName(this, "Export file as", "default", tr("SVG files (*.svg)"));
	ui->filename_text->setText(filename);
}
