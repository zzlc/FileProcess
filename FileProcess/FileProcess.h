#pragma once

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include "ui_FileProcess.h"
#include "ui_ProgressWidget.h"
#include "Global/GlobalConfig.h"

class IBaseProcess;

class FileProcess : public QMainWindow
{
    Q_OBJECT

public:
    FileProcess(QWidget *parent = Q_NULLPTR);
    ~FileProcess();

    // 按照文件夹进行分块、加密处理，走流程
    // @param aes_key_ 加密秘钥
    // @param slice_count_ 分块数量，私有块不在计算范围内
    // @param use_src_dir_ 是否将分块后的文件存储在源文件目录下
    int         SliceProcessDirectory(char* aes_key_, const int& slice_count_,
        bool use_src_dir_);

    // 合并文件处理流程
    // 选择一个数据块所在目录，并选择一个目标文件所在目录
    // @param aes_key_ 加密秘钥
    // @param use_src_dir_ 是否将分块后的文件存储在源文件目录下
    int         MergeProcessDirectory(char* aes_key_, bool use_src_dir_);

protected:
    // 选择一个文件
    QString     SelectFile(const QString& dlg_title_);

    // 选择多个文件
    QStringList SelectFiles(const QString& dlg_title_);

    // 选择对话框
    QString     SelectDir(const QString& dlg_title_);

    // 遍历指定文件夹中所有文件
    int         QueryDirectory(const string& path_, list<string>& dest_file_vec_);

signals:

public slots:
    void        UpdateProgressSlot();

private:
    Ui::FileProcessClass ui;
    //Ui::Process_Form*        _progress_ui;
    shared_ptr<IBaseProcess> _base_process_ptr;

    std::thread  _slice_thread;
    std::thread  _merge_thread;
    QTimer       _timer;
};
