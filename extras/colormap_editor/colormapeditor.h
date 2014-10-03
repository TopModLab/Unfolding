#ifndef COLORMAPEDITOR_H
#define COLORMAPEDITOR_H

#include <QWidget>
#include <QPainter>

#include "../../common.h"
#include "../../mathutils.hpp"
#include "../../utils.hpp"

#include "../../colormap.h"

namespace Ui {
class ColormapEditor;
}

class ColorPatch : public QWidget
{
  Q_OBJECT

public:
  explicit ColorPatch(QColor c, QWidget *parent = 0): color(c) {}
  ~ColorPatch() {}

  void setColor(QColor c) { color = c; update(); }
  QColor getColor() {
    return color;
  }

signals:
  void sig_clicked();

protected:
  void mousePressEvent(QMouseEvent *) {}
  void mouseReleaseEvent(QMouseEvent *) { emit sig_clicked(); }

  void paintEvent(QPaintEvent *e)
  {
      QPainter painter(this);
      painter.fillRect(0, 0, width(), height(), color);
  }

private:
  QColor color;
};

class ColormapEditor : public QWidget
{
  Q_OBJECT

public:
  explicit ColormapEditor(QWidget *parent = 0);
  ~ColormapEditor();

  ColorMap getColormap();

  QColor getNegColor() {
    cout << "neg color:" << endl;
    Utils::printColor(pneg->getColor());
    return pneg->getColor();
  }
  QColor getPosColor() {
    cout << "pos color:" << endl;
    Utils::printColor(ppos->getColor());
    return ppos->getColor();
  }

signals:
  void colorChanged();

private:
  void initializeComponents();
  void connectComponents();

private slots:
  void slot_updateNegPatchWithSlider(int val);
  void slot_updateNegPatchWithSpin(double val);
  void slot_updatePosPatchWithSlider(int val);
  void slot_updatePosPatchWithSpin(double val);

  void updatePatches();
  void slot_changeColor();

private:
  Ui::ColormapEditor *ui;
  ColorPatch *pneg, *ppos;
};

#endif // COLORMAPEDITOR_H
