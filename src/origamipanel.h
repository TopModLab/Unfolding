#pragma once
#ifndef ORIGAMIPANEL_H
#define ORIGAMIPANEL_H

#include "common.h"
#include <QWidget>

namespace Ui {
class OrigamiPanel;
}

class OrigamiPanel : public QWidget
{
    Q_OBJECT

public:
    explicit OrigamiPanel(QWidget *parent = 0);
    ~OrigamiPanel();
	confMap getConfigValues() const { return origamiConfig; }

signals:
	void sig_save2origami();
	void sig_saved();
	void sig_canceled();

public slots:
	void slot_saved();

private:
    Ui::OrigamiPanel *ui;
	confMap origamiConfig;
};

#endif // ORIGAMIPANEL_H
