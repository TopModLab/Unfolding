#include "ConnectorSelectionPanel.h"

ConnectorSelectionPanel::ConnectorSelectionPanel(QDialog *parent)
	: ui(new Ui::ConnectorSelectionPanel)
{
	ui->setupUi(this);
	setWindowTitle(tr("Mesh Export Panel"));
	connect(ui->file_button, SIGNAL(clicked()), this, SLOT(slot_getFileName()));
}

ConnectorSelectionPanel::~ConnectorSelectionPanel()
{
}

void ConnectorSelectionPanel::setMeshType(int mt)
{
	meshType = mt;
}

void ConnectorSelectionPanel::slot_getFileName()
{
	QString filename = QFileDialog::getSaveFileName(this, "Export file as", "export/untitled.svg", tr("XML files (*.svg *.xml)"));
	ui->filename_text->setText(filename);
}
