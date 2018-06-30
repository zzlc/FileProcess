/********************************************************************************
** Form generated from reading UI file 'ProgressWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROGRESSWIDGET_H
#define UI_PROGRESSWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Process_Form
{
public:
    QProgressBar *progressBar_All;
    QProgressBar *progressBar_Child;
    QLabel *static_All;
    QLabel *static_Child;

    void setupUi(QWidget *Process_Form)
    {
        if (Process_Form->objectName().isEmpty())
            Process_Form->setObjectName(QStringLiteral("Process_Form"));
        Process_Form->resize(423, 142);
        progressBar_All = new QProgressBar(Process_Form);
        progressBar_All->setObjectName(QStringLiteral("progressBar_All"));
        progressBar_All->setGeometry(QRect(0, 0, 421, 23));
        progressBar_All->setValue(24);
        progressBar_Child = new QProgressBar(Process_Form);
        progressBar_Child->setObjectName(QStringLiteral("progressBar_Child"));
        progressBar_Child->setGeometry(QRect(0, 80, 421, 23));
        progressBar_Child->setValue(24);
        static_All = new QLabel(Process_Form);
        static_All->setObjectName(QStringLiteral("static_All"));
        static_All->setGeometry(QRect(0, 30, 421, 31));
        static_Child = new QLabel(Process_Form);
        static_Child->setObjectName(QStringLiteral("static_Child"));
        static_Child->setGeometry(QRect(0, 110, 421, 21));

        retranslateUi(Process_Form);

        QMetaObject::connectSlotsByName(Process_Form);
    } // setupUi

    void retranslateUi(QWidget *Process_Form)
    {
        Process_Form->setWindowTitle(QApplication::translate("Process_Form", "Form", nullptr));
        static_All->setText(QApplication::translate("Process_Form", "\346\211\200\346\234\211\347\232\204\346\226\207\344\273\266", nullptr));
        static_Child->setText(QApplication::translate("Process_Form", "\345\275\223\345\211\215\345\244\204\347\220\206\344\270\255\347\232\204\346\226\207\344\273\266", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Process_Form: public Ui_Process_Form {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROGRESSWIDGET_H
