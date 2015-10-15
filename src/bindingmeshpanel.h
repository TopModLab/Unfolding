#ifndef BINDINGMESHPANEL_H
#define BINDINGMESHPANEL_H

#include <QWidget>

namespace Ui {
class BindingMeshPanel;
}

class BindingMeshPanel : public QWidget
{
    Q_OBJECT

public:
    explicit BindingMeshPanel(QWidget *parent = 0);
    ~BindingMeshPanel();

private:
    Ui::BindingMeshPanel *ui;
};

#endif // BINDINGMESHPANEL_H
