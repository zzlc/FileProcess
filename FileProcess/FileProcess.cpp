#include "FileProcess.h"
#include "IBaseProcess.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

FileProcess::FileProcess(QWidget *parent)
    : QMainWindow(parent)
    , _base_process_ptr(new IBaseProcess)
{
    ui.setupUi(this);

    // 分块测试
    ProcessDirectory();
    //return;

    // 合并文件测试
    list<string> public_file_list = {
        "D:/2/VID_20180626_193222.mp4_public_1",
        "D:/2/VID_20180626_193222.mp4_public_2",
        "D:/2/VID_20180626_193222.mp4_public_3",
        "D:/2/VID_20180626_193222.mp4_public_4",
        "D:/2/VID_20180626_193222.mp4_public_5",
        "D:/2/VID_20180626_193222.mp4_public_6",
        "D:/2/VID_20180626_193222.mp4_public_7",
        "D:/2/VID_20180626_193222.mp4_public_8",
        "D:/2/VID_20180626_193222.mp4_public_9"
    };
    string dest_file;
    _base_process_ptr->MegerFile("D:/2/VID_20180626_193222.mp4_private", public_file_list,
        "D:/2/", dest_file);
    dest_file.clear();
}

FileProcess::~FileProcess()
{
    LOGGER->flush();
}

int FileProcess::ProcessDirectory()
{
    LOGGER->info("{} Start", __FUNCTION__);
    QString select_path = /*SelectDir()*/"D:/1";
    if (!select_path.isEmpty()) {
        list<string> file_name_list;
        QueryDirectory(select_path.toStdString(), file_name_list);
        if (file_name_list.empty()) {
            LOGGER->warn("{} path:{} nothing!", __FUNCTION__, select_path.toStdString());
        }
        for (auto&& itor : file_name_list) {
            string private_file_name;
            list<string> publish_file_name_list;
            _base_process_ptr->SliceFile(
                select_path.toStdString(), 
                itor, 
                "D:/2", 
                9, 
                private_file_name, 
                publish_file_name_list
            );
        }
    }
    LOGGER->info("{} End", __FUNCTION__);
    return 0;
}

QString FileProcess::SelectFile()
{
    return QFileDialog::getOpenFileName(
        this,
        tr("选择文件"),
#ifdef _WIN32
        "C:/"
#else
        "/home/"
#endif // _WIN32
    );
}

QString FileProcess::SelectDir()
{
    return QFileDialog::getExistingDirectory(
        this,
        tr("选择文件夹"),
#ifdef _WIN32
        "C:/",
#else
        "/home/",
#endif // _WIN32
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
}

int FileProcess::QueryDirectory(const string& path_, list<string>& dest_file_vec_)
{
    LOGGER->info("{} path:{}", __FUNCTION__, path_);
    if (path_.empty()) {
        return -1;
    }
    QDir dir(QString::fromStdString(path_));
    if (!dir.exists()) {
        LOGGER->error("{} path:{} not exist!", __FUNCTION__, path_);
        return -1;
    }
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList file_list = dir.entryInfoList();
    for (auto&& itor : file_list) {
        dest_file_vec_.emplace_back(itor.fileName().toStdString());
        LOGGER->info("{} path:{} cantain file name:{}", 
            __FUNCTION__, path_, itor.fileName().toStdString());
    }
    LOGGER->flush();
    return 0;
}

