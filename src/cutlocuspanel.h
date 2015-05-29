#ifndef CUTLOCUSPANEL_H
#define CUTLOCUSPANEL_H

#include <QtWidgets/QWidget>

namespace Ui {
class CutLocusPanel;
}

class CutLocusPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CutLocusPanel(QWidget *parent = 0);
    ~CutLocusPanel();

signals:
    void sig_methodChanged(int);
    void sig_displayCut(bool);
    void sig_displayMinMax(bool);
    void closedSignal();

private:
    void closeEvent(QCloseEvent *e);
private:
    Ui::CutLocusPanel *ui;
};

#endif // CUTLOCUSPANEL_H
