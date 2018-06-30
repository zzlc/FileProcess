#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FileProcess.h"
#include "Global/GlobalConfig.h"

class IBaseProcess;

class FileProcess : public QMainWindow
{
    Q_OBJECT

public:
    FileProcess(QWidget *parent = Q_NULLPTR);
    ~FileProcess();

    // �����ļ��н��д���������
    int         ProcessDirectory();

protected:
    // ѡ��һ���ļ�
    QString     SelectFile();

    // ѡ��Ի���
    QString     SelectDir();

    // ����ָ���ļ����������ļ�
    int         QueryDirectory(const string& path_, list<string>& dest_file_vec_);


private:
    Ui::FileProcessClass ui;
    shared_ptr<IBaseProcess> _base_process_ptr;
};
