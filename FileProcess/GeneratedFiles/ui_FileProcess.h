/********************************************************************************
** Form generated from reading UI file 'FileProcess.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILEPROCESS_H
#define UI_FILEPROCESS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FileProcessClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *FileProcessClass)
    {
        if (FileProcessClass->objectName().isEmpty())
            FileProcessClass->setObjectName(QStringLiteral("FileProcessClass"));
        FileProcessClass->resize(600, 400);
        menuBar = new QMenuBar(FileProcessClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        FileProcessClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(FileProcessClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        FileProcessClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(FileProcessClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        FileProcessClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(FileProcessClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        FileProcessClass->setStatusBar(statusBar);

        retranslateUi(FileProcessClass);

        QMetaObject::connectSlotsByName(FileProcessClass);
    } // setupUi

    void retranslateUi(QMainWindow *FileProcessClass)
    {
        FileProcessClass->setWindowTitle(QApplication::translate("FileProcessClass", "FileProcess", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FileProcessClass: public Ui_FileProcessClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILEPROCESS_H
