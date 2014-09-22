#include "colormapeditor.h"
#include "ui_colormapeditor.h"

#include <QColorDialog>

ColormapEditor::ColormapEditor(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ColormapEditor)
{
  ui->setupUi(this);
  setWindowTitle("Colormap Editor");

  initializeComponents();
  connectComponents();

  ui->posSlider->setValue(5000);
  ui->negSlider->setValue(5000);

  updatePatches();
}

ColormapEditor::~ColormapEditor()
{
  delete ui;
}

ColorMap ColormapEditor::getColormap() {
  ColorMap cmap;
  cmap.setColor(ui->negSpin->value(), pneg->getColor());
  cmap.setColor(ui->posSpin->value(), ppos->getColor());
  cmap.setColor(0.0, Qt::gray);

  cmap.buildColormap();
  return cmap;
}

void ColormapEditor::initializeComponents()
{
  ui->negSlider->setMinimum(0);
  ui->negSlider->setMaximum(10000);

  ui->posSlider->setMinimum(0);
  ui->posSlider->setMaximum(10000);

  ui->negSpin->setMinimum(-PI);
  ui->negSpin->setMaximum(0);
  ui->negSpin->setSingleStep(0.01);

  ui->posSpin->setMinimum(0);
  ui->posSpin->setMaximum(PI);
  ui->posSpin->setSingleStep(0.01);

  pneg = new ColorPatch(Qt::red);
  ppos = new ColorPatch(Qt::blue);
  ui->pathLayout->addWidget(pneg);
  ui->pathLayout->addWidget(ppos);
}

void ColormapEditor::connectComponents()
{
  connect(ui->negSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_updateNegPatchWithSlider(int)));
  connect(ui->posSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_updatePosPatchWithSlider(int)));
  connect(ui->negSpin, SIGNAL(valueChanged(double)), this, SLOT(slot_updateNegPatchWithSpin(double)));
  connect(ui->posSpin, SIGNAL(valueChanged(double)), this, SLOT(slot_updatePosPatchWithSpin(double)));

  connect(pneg, SIGNAL(sig_clicked()), this, SLOT(slot_changeColor()));
  connect(ppos, SIGNAL(sig_clicked()), this, SLOT(slot_changeColor()));
}

void ColormapEditor::slot_updateNegPatchWithSpin(double val)
{
  cout << val << endl;
  ui->negSlider->setValue(ui->negSlider->maximum() * (1.0 + val / PI));
}

void ColormapEditor::slot_updatePosPatchWithSpin(double val)
{
  cout << val << endl;
  ui->posSlider->setValue(val / PI * 10000);
}

void ColormapEditor::updatePatches()
{
  pneg->update();
  ppos->update();
}

void ColormapEditor::slot_changeColor()
{
  ColorPatch *patch = dynamic_cast<ColorPatch*>(sender());
  QColor c = QColorDialog::getColor(patch->getColor());
  patch->setColor(c);
  emit colorChanged();
}

void ColormapEditor::slot_updateNegPatchWithSlider(int val)
{
  double dval = (ui->negSlider->maximum() - val) / (double) ui->negSlider->maximum() * PI;
  cout << dval << endl;
  ui->negSpin->setValue(-dval);
}

void ColormapEditor::slot_updatePosPatchWithSlider(int val)
{
  double dval = val / (double) ui->posSlider->maximum() * PI;
  cout << dval << endl;
  ui->posSpin->setValue(dval);
}
