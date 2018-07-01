#include "FileProcess.h"
#include "IBaseProcess.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

#pragma execution_character_set("utf-8") 

FileProcess::FileProcess(QWidget *parent)
    : QMainWindow(parent)
    , _base_process_ptr(new IBaseProcess)
{
    ui.setupUi(this);

    ui.progressBar_All->setRange(0, 100);
    ui.progressBar_All->setValue(0);
    ui.progressBar_Child->setRange(0, 100);
    ui.progressBar_Child->setValue(0);
    
    GLOBALCONFIG->SetAES128Key("0f1571c947d9e859abb7add6af7f6798");
    // 分块测试
    //SliceProcessDirectory(GLOBALCONFIG->GetAES128Key(), 5, false);

    // 合并文件测试
    MergeProcessDirectory(GLOBALCONFIG->GetAES128Key(), false);

    show();

    connect(&_timer, SIGNAL(timeout()), this, SLOT(UpdateProgressSlot()));
    _timer.start(1000);
}

FileProcess::~FileProcess()
{
    if (_slice_thread.joinable()) {
        _slice_thread.join();
    }
    if (_merge_thread.joinable()) {
        _merge_thread.join();
    }
    LOGGER->flush();
}

int FileProcess::SliceProcessDirectory(char* aes_key_, const int& slice_count_,
    bool use_src_dir_)
{
    LOGGER->info("{} Start: aes key ptr:{}, slice_count:{}, use src dir:{}", 
        __FUNCTION__, (char*)aes_key_, slice_count_, use_src_dir_);
    QString src_dir = SelectDir(tr("选择源文件夹"));
    if (!src_dir.isEmpty()) {
        list<string> file_name_list;
        QueryDirectory(src_dir.toStdString(), file_name_list);
        if (file_name_list.empty()) {
            LOGGER->warn("{} path:{} nothing!", __FUNCTION__, src_dir.toStdString());
        }

        // 确定目标文件夹，默认同原文件夹
        string dest_dir = src_dir.toStdString();
        if (!use_src_dir_) {
            dest_dir = SelectDir(tr("选择目标文件夹")).toStdString();
        }

        GLOBALCONFIG->SetFileCount(file_name_list.size());

        _slice_thread = std::thread(
            [=]() {
            int index(0);
            for (auto&& itor : file_name_list) {

                //emit UpdateTotalSliceProgress(src_dir.toStdString(), ++index, file_name_list.size());
                GLOBALCONFIG->SetCurrentFileName(itor);
                GLOBALCONFIG->SetTotalProgress(1.0f * index / file_name_list.size());
                GLOBALCONFIG->SetCurrentFileIndex(++index);

                string dest_private_file_name;
                list<string> dest_public_file_name_list;
                _base_process_ptr->SliceFile(
                    src_dir.toStdString(),
                    itor,
                    dest_dir,
                    slice_count_,
                    aes_key_,
                    dest_private_file_name,
                    dest_public_file_name_list
                );

                GLOBALCONFIG->SetTotalProgress(1.0f * index / file_name_list.size());
            }
        }
        );
    }
    LOGGER->info("{} End", __FUNCTION__);
    return 0;
}

int FileProcess::MergeProcessDirectory(char* aes_key_, bool use_src_dir_)
{
    LOGGER->info("{} Start: aes key ptr:{}, use src dir:{}", 
        __FUNCTION__, (char *)aes_key_, use_src_dir_);

    QString private_file_name = SelectFile(tr("选择私有块文件"));
    if (private_file_name.isEmpty()) {
        return -1;
    }
    QStringList public_file_name_list = SelectFiles(tr("选择公有块文件"));
    if (public_file_name_list.empty()) {
        return -1;
    }
    QString dest_path = private_file_name;
    if (!use_src_dir_) {
        dest_path = SelectDir(tr("选择合成文件存储文件夹"));
    }

    // 转换成 stl
    list<string> public_file_list;
    for (auto&& itor : public_file_name_list) {
        public_file_list.push_back(itor.toStdString());
    }
    _merge_thread = std::thread([=]() {
        string dest_file_name;
        _base_process_ptr->MegerFile(
            private_file_name.toStdString(), 
            public_file_list,
            dest_path.toStdString(), 
            dest_file_name
            );
    });

    LOGGER->info("{} End", __FUNCTION__);
    return 0;

}

QString FileProcess::SelectFile(const QString& dlg_title_)
{
    return QFileDialog::getOpenFileName(
        this,
        dlg_title_,
#ifdef _WIN32
        "C:/"
#else
        "/home/"
#endif // _WIN32
    );
}

QStringList FileProcess::SelectFiles(const QString& dlg_title_)
{
    QStringList filters;
    return QFileDialog::getOpenFileNames(
        this,
        dlg_title_,
#ifdef _WIN32
        "C:/",
#else
        "/home/",
#endif // _WIN32
        tr("Any files(*)")
    );
}

QString FileProcess::SelectDir(const QString& dlg_title_)
{
    return QFileDialog::getExistingDirectory(
        this,
        dlg_title_,
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

void FileProcess::UpdateProgressSlot()
{
    QString total_progress_str;
    if (1.0f == GLOBALCONFIG->GetTotalProgress()) {
        total_progress_str =
            QString(tr("总进度：共 %1 个文件，全部处理完成！"))
            .arg(GLOBALCONFIG->GetFileCount());
    } else {
        total_progress_str =
            QString(tr("总进度：共 %1 个文件，正在处理第 %2 个文件..."))
            .arg(GLOBALCONFIG->GetFileCount())
            .arg(GLOBALCONFIG->GetCurrentFileIndex());
    }
    ui.static_All->setText(total_progress_str);
    ui.progressBar_All->setValue(GLOBALCONFIG->GetTotalProgress() * 100);

    QString child_progress_str;
    if (1.0f == GLOBALCONFIG->GetChildProgress()) {
        child_progress_str =
            QString(tr("文件：%1 处理完成！"))
            .arg(GLOBALCONFIG->GetCurrentFileName().c_str());
    } else {
        child_progress_str =
            QString(tr("正在处理文件：%1，当前进度%2%..."))
            .arg(GLOBALCONFIG->GetCurrentFileName().c_str())
            .arg(GLOBALCONFIG->GetChildProgress() * 100);
    }
    ui.static_Child->setText(child_progress_str);
    ui.progressBar_Child->setValue(GLOBALCONFIG->GetChildProgress() * 100);
}

