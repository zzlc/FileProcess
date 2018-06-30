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

    // 按照文件夹进行处理，走流程
    int         ProcessDirectory();

protected:
    // 选择一个文件
    QString     SelectFile();

    // 选择对话框
    QString     SelectDir();

    // 遍历指定文件夹中所有文件
    int         QueryDirectory(const string& path_, list<string>& dest_file_vec_);


private:
    Ui::FileProcessClass ui;
    shared_ptr<IBaseProcess> _base_process_ptr;
};
