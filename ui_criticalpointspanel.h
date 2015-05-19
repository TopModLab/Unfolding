/********************************************************************************
** Form generated from reading UI file 'criticalpointspanel.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CRITICALPOINTSPANEL_H
#define UI_CRITICALPOINTSPANEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CriticalPointsPanel
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *methodComboBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QSlider *smoothingSlider;
    QSpinBox *smoothingSpinBox;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *smoothingTypeCheckBox;

    void setupUi(QWidget *CriticalPointsPanel)
    {
        if (CriticalPointsPanel->objectName().isEmpty())
            CriticalPointsPanel->setObjectName(QStringLiteral("CriticalPointsPanel"));
        CriticalPointsPanel->resize(293, 130);
        verticalLayoutWidget = new QWidget(CriticalPointsPanel);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 10, 271, 111));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(verticalLayoutWidget);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        methodComboBox = new QComboBox(verticalLayoutWidget);
        methodComboBox->setObjectName(QStringLiteral("methodComboBox"));

        horizontalLayout->addWidget(methodComboBox);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label_2 = new QLabel(verticalLayoutWidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_2->addWidget(label_2);

        smoothingSlider = new QSlider(verticalLayoutWidget);
        smoothingSlider->setObjectName(QStringLiteral("smoothingSlider"));
        smoothingSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(smoothingSlider);

        smoothingSpinBox = new QSpinBox(verticalLayoutWidget);
        smoothingSpinBox->setObjectName(QStringLiteral("smoothingSpinBox"));

        horizontalLayout_2->addWidget(smoothingSpinBox);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        smoothingTypeCheckBox = new QCheckBox(verticalLayoutWidget);
        smoothingTypeCheckBox->setObjectName(QStringLiteral("smoothingTypeCheckBox"));

        horizontalLayout_3->addWidget(smoothingTypeCheckBox);


        verticalLayout->addLayout(horizontalLayout_3);


        retranslateUi(CriticalPointsPanel);

        QMetaObject::connectSlotsByName(CriticalPointsPanel);
    } // setupUi

    void retranslateUi(QWidget *CriticalPointsPanel)
    {
        CriticalPointsPanel->setWindowTitle(QApplication::translate("CriticalPointsPanel", "CriticalPointsPanel", 0));
        label->setText(QApplication::translate("CriticalPointsPanel", "Method", 0));
        methodComboBox->clear();
        methodComboBox->insertItems(0, QStringList()
         << QApplication::translate("CriticalPointsPanel", "Geodesic", 0)
         << QApplication::translate("CriticalPointsPanel", "Z-Value", 0)
         << QApplication::translate("CriticalPointsPanel", "Point-Normal", 0)
         << QApplication::translate("CriticalPointsPanel", "Curvature", 0)
         << QApplication::translate("CriticalPointsPanel", "Quadratic", 0)
         << QApplication::translate("CriticalPointsPanel", "Random", 0)
        );
        label_2->setText(QApplication::translate("CriticalPointsPanel", "Smoothing", 0));
        smoothingTypeCheckBox->setText(QApplication::translate("CriticalPointsPanel", "Smoothing on Actual Mesh", 0));
    } // retranslateUi

};

namespace Ui {
    class CriticalPointsPanel: public Ui_CriticalPointsPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CRITICALPOINTSPANEL_H
