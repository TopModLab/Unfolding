/********************************************************************************
** Form generated from reading UI file 'colormapeditor.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORMAPEDITOR_H
#define UI_COLORMAPEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QSlider>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ColormapEditor
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *pathLayout;
    QHBoxLayout *horizontalLayout;
    QSlider *negSlider;
    QDoubleSpinBox *negSpin;
    QSlider *posSlider;
    QDoubleSpinBox *posSpin;

    void setupUi(QWidget *ColormapEditor)
    {
        if (ColormapEditor->objectName().isEmpty())
            ColormapEditor->setObjectName(QStringLiteral("ColormapEditor"));
        ColormapEditor->resize(261, 122);
        verticalLayoutWidget = new QWidget(ColormapEditor);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 20, 241, 80));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        pathLayout = new QHBoxLayout();
        pathLayout->setObjectName(QStringLiteral("pathLayout"));

        verticalLayout->addLayout(pathLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        negSlider = new QSlider(verticalLayoutWidget);
        negSlider->setObjectName(QStringLiteral("negSlider"));
        negSlider->setMaximum(9999);
        negSlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(negSlider);

        negSpin = new QDoubleSpinBox(verticalLayoutWidget);
        negSpin->setObjectName(QStringLiteral("negSpin"));
        negSpin->setMinimum(-3.14);
        negSpin->setMaximum(0);

        horizontalLayout->addWidget(negSpin);

        posSlider = new QSlider(verticalLayoutWidget);
        posSlider->setObjectName(QStringLiteral("posSlider"));
        posSlider->setMaximum(9999);
        posSlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(posSlider);

        posSpin = new QDoubleSpinBox(verticalLayoutWidget);
        posSpin->setObjectName(QStringLiteral("posSpin"));
        posSpin->setMaximum(3.14);

        horizontalLayout->addWidget(posSpin);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(ColormapEditor);

        QMetaObject::connectSlotsByName(ColormapEditor);
    } // setupUi

    void retranslateUi(QWidget *ColormapEditor)
    {
        ColormapEditor->setWindowTitle(QApplication::translate("ColormapEditor", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class ColormapEditor: public Ui_ColormapEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORMAPEDITOR_H
