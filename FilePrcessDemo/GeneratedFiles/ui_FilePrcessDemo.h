/********************************************************************************
** Form generated from reading UI file 'FilePrcessDemo.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILEPRCESSDEMO_H
#define UI_FILEPRCESSDEMO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FilePrcessDemoClass
{
public:
    QWidget *centralWidget;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QPushButton *sliceButton;
    QPushButton *mergeButton;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *FilePrcessDemoClass)
    {
        if (FilePrcessDemoClass->objectName().isEmpty())
            FilePrcessDemoClass->setObjectName(QStringLiteral("FilePrcessDemoClass"));
        FilePrcessDemoClass->resize(300, 200);
        FilePrcessDemoClass->setMinimumSize(QSize(300, 200));
        FilePrcessDemoClass->setMaximumSize(QSize(300, 200));
        centralWidget = new QWidget(FilePrcessDemoClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayoutWidget = new QWidget(centralWidget);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(40, 40, 160, 80));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        sliceButton = new QPushButton(verticalLayoutWidget);
        sliceButton->setObjectName(QStringLiteral("sliceButton"));

        verticalLayout->addWidget(sliceButton);

        mergeButton = new QPushButton(verticalLayoutWidget);
        mergeButton->setObjectName(QStringLiteral("mergeButton"));

        verticalLayout->addWidget(mergeButton);

        FilePrcessDemoClass->setCentralWidget(centralWidget);
        mainToolBar = new QToolBar(FilePrcessDemoClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        FilePrcessDemoClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(FilePrcessDemoClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        FilePrcessDemoClass->setStatusBar(statusBar);

        retranslateUi(FilePrcessDemoClass);

        QMetaObject::connectSlotsByName(FilePrcessDemoClass);
    } // setupUi

    void retranslateUi(QMainWindow *FilePrcessDemoClass)
    {
        FilePrcessDemoClass->setWindowTitle(QApplication::translate("FilePrcessDemoClass", "FilePrcessDemo", nullptr));
        sliceButton->setText(QApplication::translate("FilePrcessDemoClass", "\345\210\206\345\211\262\346\226\207\344\273\266\346\265\213\350\257\225", nullptr));
        mergeButton->setText(QApplication::translate("FilePrcessDemoClass", "\345\220\210\346\210\220\346\226\207\344\273\266\346\265\213\350\257\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FilePrcessDemoClass: public Ui_FilePrcessDemoClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILEPRCESSDEMO_H
