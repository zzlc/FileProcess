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
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FileProcessClass
{
public:
    QWidget *centralWidget;
    QLabel *static_All;
    QProgressBar *progressBar_Child;
    QLabel *static_Child;
    QProgressBar *progressBar_All;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *FileProcessClass)
    {
        if (FileProcessClass->objectName().isEmpty())
            FileProcessClass->setObjectName(QStringLiteral("FileProcessClass"));
        FileProcessClass->resize(454, 208);
        centralWidget = new QWidget(FileProcessClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        static_All = new QLabel(centralWidget);
        static_All->setObjectName(QStringLiteral("static_All"));
        static_All->setGeometry(QRect(10, 40, 421, 31));
        progressBar_Child = new QProgressBar(centralWidget);
        progressBar_Child->setObjectName(QStringLiteral("progressBar_Child"));
        progressBar_Child->setGeometry(QRect(10, 90, 421, 23));
        progressBar_Child->setValue(24);
        static_Child = new QLabel(centralWidget);
        static_Child->setObjectName(QStringLiteral("static_Child"));
        static_Child->setGeometry(QRect(10, 120, 421, 21));
        progressBar_All = new QProgressBar(centralWidget);
        progressBar_All->setObjectName(QStringLiteral("progressBar_All"));
        progressBar_All->setGeometry(QRect(10, 10, 421, 23));
        progressBar_All->setValue(24);
        FileProcessClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(FileProcessClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 454, 26));
        FileProcessClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(FileProcessClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        FileProcessClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(FileProcessClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        FileProcessClass->setStatusBar(statusBar);

        retranslateUi(FileProcessClass);

        QMetaObject::connectSlotsByName(FileProcessClass);
    } // setupUi

    void retranslateUi(QMainWindow *FileProcessClass)
    {
        FileProcessClass->setWindowTitle(QApplication::translate("FileProcessClass", "FileProcess", nullptr));
        static_All->setText(QApplication::translate("FileProcessClass", "\346\200\273\350\277\233\345\272\246", nullptr));
        static_Child->setText(QApplication::translate("FileProcessClass", "\345\275\223\345\211\215\345\244\204\347\220\206\344\270\255\347\232\204\346\226\207\344\273\266", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FileProcessClass: public Ui_FileProcessClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILEPROCESS_H
