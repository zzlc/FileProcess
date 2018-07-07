#include "FileProcess.h"
#include "IBaseProcess.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

#pragma execution_character_set("utf-8") 

FileProcess::FileProcess(bool slice_, QWidget *parent)
    : QMainWindow(parent)
    , _base_process_ptr(new IBaseProcess)
{
    ui.setupUi(this);
    this->setWindowTitle(tr("文件处理客户端"));

    ui.progressBar_All->setRange(0, 100);
    ui.progressBar_All->setValue(0);
    ui.progressBar_Child->setRange(0, 100);
    ui.progressBar_Child->setValue(0);

    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());

    //if (slice_) {
    //    // 分块测试 
    //    SliceProcessDirectory((char *)GLOBALCONFIG->GetAES128Key(), 3, false);
    //} else {
    //    // 合并文件测试 
    //    MergeProcessDirectory((char *)GLOBALCONFIG->GetAES128Key(), false);
    //}

    connect(&_timer, SIGNAL(timeout()), this, SLOT(UpdateProgressSlot()));
    _timer.start(500);
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
        __FUNCTION__, (void*)aes_key_, slice_count_, use_src_dir_);
    QString src_dir = SelectDir(tr("选择源文件夹"));
    if (!src_dir.isEmpty()) {
        list<string> file_name_list;
        QueryDirectory(src_dir, file_name_list);
        if (file_name_list.empty()) {
            LOGGER->warn("{} path:{} nothing!", __FUNCTION__, src_dir.toStdString());
            return -1;
        }

        // 确定目标文件夹，默认同原文件夹 
        string dest_dir = unicode_to_utf(src_dir.toStdWString());
        if (!use_src_dir_) {
            dest_dir = unicode_to_utf(SelectDir(tr("选择目标文件夹")).toStdWString());
        }

        // 判断文件大小 
        auto itor = file_name_list.begin();
        while (itor != file_name_list.end()) {
            int64_t file_length = GetFileSize(unicode_to_utf(src_dir.toStdWString()) + "/" + *itor);
            if (file_length < (slice_count_ + 1) * block_size) {
                QString msg = QString::fromLocal8Bit("文件 %1 太小，此客户端暂不处理")
                    .arg(utf_to_unicode(*itor).c_str());
                QMessageBox::warning(NULL, QString::fromLocal8Bit("警告"),
                    msg);
                itor = file_name_list.erase(itor);
            } else {
                ++itor;
            }
        }
        if (file_name_list.empty()) {
            return -1;
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
                    unicode_to_utf(src_dir.toStdWString()),
                    itor,
                    dest_dir,
                    slice_count_,
                    aes_key_,
                    GLOBALCONFIG->CreateUUID(),
                    dest_private_file_name,
                    dest_public_file_name_list
                );

                GLOBALCONFIG->SetTotalProgress(1.0f * index / file_name_list.size());
            }
        }
        );
    } else {
        return -1;
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
    int pos = dest_path.lastIndexOf("/");
    dest_path = dest_path.remove(pos, dest_path.size() - pos);
    if (!use_src_dir_) {
        dest_path = SelectDir(tr("选择合成文件存储文件夹"));
        if (dest_path.isEmpty()) {
            return -1;
        }
    }

    // 转换成 stl
    list<string> public_file_list;
    for (auto&& itor : public_file_name_list) {
        public_file_list.push_back(unicode_to_utf(itor.toStdWString()));
    }

    // 通过 UUID 检查是否为同一文件来源 
    if (!CheckFiles(unicode_to_utf(private_file_name.toStdWString()), public_file_list)) {
        LOGGER->warn("{} CheckFiles failed!", __FUNCTION__);
        QMessageBox::warning(nullptr, tr("错误："), tr("文件 UUID 校验失败！"));
        return -1;
    }

    _merge_thread = std::thread([=]() {
        string dest_file_name;
        _base_process_ptr->MergeFile(
            unicode_to_utf(private_file_name.toStdWString()),
            public_file_list,
            aes_key_,
            unicode_to_utf(dest_path.toStdWString()),
            dest_file_name
        );
    });

    LOGGER->info("{} End", __FUNCTION__);
    return 0;

}

bool FileProcess::CheckFiles(const string& private_file_name_, const list<string>& public_file_list_)
{
    if (private_file_name_.empty() || public_file_list_.empty()) {
        LOGGER->warn("{} private file:{} ,public file count:{} ,params error!",
            __FUNCTION__, private_file_name_, public_file_list_.size());
        return false;
    }
    // check exist?
    if (!FileExist(private_file_name_)) {
        LOGGER->warn("{} file:{} not exist!", __FUNCTION__, private_file_name_);
        return false;
    }
    for (auto && itor : public_file_list_) {
        if (!FileExist(itor)) {
            LOGGER->warn("{} file:{} not exist!", __FUNCTION__, private_file_name_);
            return false;
        }
    }
    FILE *tmp_fp = NULL;
    uint8_t src_uuid_buf[32] = { 0 };
    uint8_t tmp_uuid_buf[32] = { 0 };
    errno_t ret = fopen_s(&tmp_fp, private_file_name_.c_str(), "rb");
    if (ret != 0) {
        LOGGER->warn("{} file:{} open failed!", __FUNCTION__, private_file_name_);
        return false;
    }
    if (0 != _fseeki64(tmp_fp, -32LL, SEEK_END)) {
        LOGGER->warn("{} seek file:{} failed!", __FUNCTION__, private_file_name_);
        fclose(tmp_fp);
        return false;
    }
    ret = fread(src_uuid_buf, 1, sizeof(src_uuid_buf), tmp_fp);
    if (ret != sizeof(src_uuid_buf)) {
        LOGGER->warn("{} read file:{} failed!", __FUNCTION__, private_file_name_);
        fclose(tmp_fp);
        return false;
    }
    fclose(tmp_fp);

    // 检查公有块 UUID 是否与私有块一致 
    for (auto&& itor : public_file_list_) {
        shared_ptr<FILE> fp_ptr(fopen(itor.c_str(), "rb"), [](FILE* fp) { fclose(fp); });
        if (!fp_ptr) {
            LOGGER->warn("{} open file:{} failed!", __FUNCTION__, itor);
            return false;
        }
        if (0 != _fseeki64(fp_ptr.get(), -32LL, SEEK_END)) {
            LOGGER->warn("{} seek file:{} failed!", __FUNCTION__, itor);
            return false;
        }
        ret = fread(tmp_uuid_buf, 1, sizeof(src_uuid_buf), fp_ptr.get());
        if (ret != sizeof(src_uuid_buf)) {
            LOGGER->warn("{} read file:{} failed!", __FUNCTION__, itor);
            return false;
        }
        if (memcmp(src_uuid_buf, tmp_uuid_buf, sizeof(src_uuid_buf)) != 0) {
            LOGGER->warn("{} file:{} uuid not equal private uuid!", __FUNCTION__, itor);
            return false;
        }
    }
    return true;
}

QString FileProcess::SelectFile(const QString& dlg_title_)
{
    return QFileDialog::getOpenFileName(
        this,
        dlg_title_,
#ifdef _WIN32
        ""
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
        "",
        tr("Any files(*)")
    );
}

QString FileProcess::SelectDir(const QString& dlg_title_)
{
    return QFileDialog::getExistingDirectory(
        this,
        dlg_title_,
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
}

int FileProcess::QueryDirectory(const QString& path_, list<string>& dest_file_vec_)
{
    LOGGER->info("{} path:{}", __FUNCTION__, unicode_to_utf(path_.toStdWString()));
    if (path_.isEmpty()) {
        return -1;
    }
    QDir dir(path_);
    if (!dir.exists()) {
        LOGGER->error("{} path:{} not exist!", __FUNCTION__, unicode_to_utf(path_.toStdWString()));
        return -1;
    }
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList file_list = dir.entryInfoList();
    for (auto&& itor : file_list) {
        dest_file_vec_.emplace_back(unicode_to_utf(itor.fileName().toStdWString()));
        LOGGER->info("{} path:{} cantain file name:{}",
            __FUNCTION__, unicode_to_utf(path_.toStdWString()), itor.fileName().toStdString());
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
            .arg(QString::fromStdWString(utf_to_unicode(GLOBALCONFIG->GetCurrentFileName())));
    } else {
        child_progress_str =
            QString(tr("正在处理文件：%1"))
            .arg(QString::fromStdWString(utf_to_unicode(GLOBALCONFIG->GetCurrentFileName())));
    }
    ui.static_Child->setText(child_progress_str);
    ui.progressBar_Child->setValue(GLOBALCONFIG->GetChildProgress() * 100);
}

