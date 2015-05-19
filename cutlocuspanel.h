#ifndef CUTLOCUSPANEL_H
#define CUTLOCUSPANEL_H

#include <QWidget>

namespace Ui {
class CutLocusPanel;
}

class CutLocusPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CutLocusPanel(QWidget *parent = 0);
    ~CutLocusPanel();

private:
    Ui::CutLocusPanel *ui;
};

#endif // CUTLOCUSPANEL_H
