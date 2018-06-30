#include "FileProcess.h"
#include "demux/demuxer.h"
#include "IBaseProcess.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

FileProcess::FileProcess(QWidget *parent)
    : QMainWindow(parent)
    , _base_process_ptr(new IBaseProcess)
{
    ui.setupUi(this);
    demuxer_init_ffmpeg();

    // Debug
    //demuxer_init_ffmpeg();
    //_base_process_ptr = make_shared<IBaseProcess>();

    //_base_process_ptr->OpenFile("D:\\1\\1.mp4");
    //vector<string> child_file_name_vec;
    //_base_process_ptr->StartSlice(4, child_file_name_vec);

    ProcessDirectory();
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
                5, 
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

