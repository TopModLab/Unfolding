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

	double getBridgerSize();

signals:
	void sig_saved();
	void sig_setBridger(bool);

public slots:
	void slot_saved();

private:
	Ui::BindingMeshPanel *ui;
	int bridgerSize;

};

#endif // BINDINGMESHPANEL_H
