#include "ConnectorPanel.h"

QFont ConnectorPanel::fontfamily = QFont("Arial");

ConnectorPanel::ConnectorPanel(int procType)
	: ui(new Ui::ConnectorPanel)
{
	ui->setupUi(this);

	resetParas(procType);

	connect(ui->file_button, &QAbstractButton::clicked, this, &ConnectorPanel::setFileName);
	connect(ui->save_button, &QDialogButtonBox::clicked, this, &QDialog::close);
	connect(ui->save_button, &QDialogButtonBox::clicked, this, &ConnectorPanel::save);
}

QString ConnectorPanel::getFilename() const
{
	return ui->filename_text->text();
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
	case HDS_Mesh::QUAD_PROC:
	case HDS_Mesh::WINGED_PROC:
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
	QString filename = QFileDialog::getSaveFileName(this, "Export file as", "", tr("SVG files (*.svg)"));
	if (!filename.isEmpty())
	{
		ui->filename_text->setText(filename);
	}
}

void ConnectorPanel::save()
{
	fontfamily = ui->font_val->currentFont();

	conf["connector"] = (double)ui->connector_type->currentIndex();

	conf["scale"] = ui->scale_val->value();
	conf["width"] = ui->width_val->value();
	conf["length"] = ui->length_val->value();

	conf["pinUnit"] = (double)ui->pinholeunit_type->currentIndex();
	conf["pinSize"] = ui->pinholesize_val->value();
	conf["pinCount"] = (double)ui->pinholecount_type->currentIndex();

	conf["etchSeg"] = (double)ui->etchseg_val->value();
	conf["scoreType"] = (double)ui->score_type->currentIndex();
	conf["dashLen"] = ui->scoredash_len->value();
	conf["dashGap"] = ui->scoredash_gap->value();
	conf["dashUnit"] = (double)ui->scoredash_unit->currentIndex();

	emit sig_saved();
}
