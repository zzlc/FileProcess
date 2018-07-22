#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FilePrcessDemo.h"
#include "FileProcessInterface.h"
#include <QFileDialog>
#include <QDir>
#include <list>
#include <string>
#include <io.h>
#include <thread>
#include <QMessageBox>

#pragma comment(lib, "FileProcess.lib")

using namespace std;

class FilePrcessDemo : public QMainWindow
{
    Q_OBJECT

public:
    FilePrcessDemo(QWidget *parent = Q_NULLPTR);
    ~FilePrcessDemo();

protected:
    // ѡ��һ���ļ�
    QString     SelectFile(const QString& dlg_title_);

    // ѡ�����ļ�
    QStringList SelectFiles(const QString& dlg_title_);

    // ѡ��Ի���
    QString     SelectDir(const QString& dlg_title_);

    // ����ָ���ļ����������ļ�
    int         QueryDirectory(const QString& path_, list<string>& dest_file_vec_);

protected slots:
    void OnSliceBtnClick();
    void OnMergeBtnClick();

private:
    Ui::FilePrcessDemoClass ui;

    FileProcessInterface* _fp_ptr = nullptr;
    const uint8_t   _aes128_key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xde, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
};
