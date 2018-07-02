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
    // �ֿ����
    //SliceProcessDirectory(GLOBALCONFIG->GetAES128Key(), 5, false);

    // �ϲ��ļ�����
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
    QString src_dir = SelectDir(tr("ѡ��Դ�ļ���"));
    if (!src_dir.isEmpty()) {
        list<string> file_name_list;
        QueryDirectory(src_dir.toStdString(), file_name_list);
        if (file_name_list.empty()) {
            LOGGER->warn("{} path:{} nothing!", __FUNCTION__, src_dir.toStdString());
        }

        // ȷ��Ŀ���ļ��У�Ĭ��ͬԭ�ļ���
        string dest_dir = src_dir.toStdString();
        if (!use_src_dir_) {
            dest_dir = SelectDir(tr("ѡ��Ŀ���ļ���")).toStdString();
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
                    GLOBALCONFIG->CreateUUID(),
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

    QString private_file_name = SelectFile(tr("ѡ��˽�п��ļ�"));
    if (private_file_name.isEmpty()) {
        return -1;
    }
    QStringList public_file_name_list = SelectFiles(tr("ѡ���п��ļ�"));
    if (public_file_name_list.empty()) {
        return -1;
    }
    QString dest_path = private_file_name;
    if (!use_src_dir_) {
        dest_path = SelectDir(tr("ѡ��ϳ��ļ��洢�ļ���"));
    }

    // ת���� stl
    list<string> public_file_list;
    for (auto&& itor : public_file_name_list) {
        public_file_list.push_back(itor.toStdString());
    }

    // ͨ�� UUID ����Ƿ�Ϊͬһ�ļ���Դ
    if (!CheckFiles(private_file_name.toStdString(), public_file_list)) {
        LOGGER->warn("{} CheckFiles failed!", __FUNCTION__);
        QMessageBox::warning(nullptr, tr("�ļ� UUID У��ʧ�ܣ�"), tr("����"));
        return -1;
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
    uint8_t tmp_buffer[16] = { 0 };
    string uuid_str;
    errno_t ret = fopen_s(&tmp_fp, private_file_name_.c_str(), "rb");
    if (ret != 0) {
        LOGGER->warn("{} file:{} open failed!", __FUNCTION__, private_file_name_);
        return false;
    }
    if (0 != _fseeki64(tmp_fp, -16LL, SEEK_END)) {
        LOGGER->warn("{} seek file:{} failed!", __FUNCTION__, private_file_name_);
        fclose(tmp_fp);
        return false;
    }
    ret = fread(tmp_buffer, 1, sizeof(tmp_buffer), tmp_fp);
    if (ret != sizeof(tmp_buffer)) {
        LOGGER->warn("{} read file:{} failed!", __FUNCTION__, private_file_name_);
        fclose(tmp_fp);
        return false;
    }
    uuid_str.append((char *)tmp_buffer, ret);
    fclose(tmp_fp);

    // ��鹫�п� UUID �Ƿ���˽�п�һ��
    for (auto&& itor : public_file_list_) {
        shared_ptr<FILE> fp_ptr(fopen(itor.c_str(), "rb"), [](FILE* fp) { fclose(fp); });
        if (!fp_ptr) {
            LOGGER->warn("{} open file:{} failed!", __FUNCTION__, itor);
            return false;
        }
        if (0 != _fseeki64(fp_ptr.get(), -16LL, SEEK_END)) {
            LOGGER->warn("{} seek file:{} failed!", __FUNCTION__, itor);
            fclose(tmp_fp);
            return false;
        }
        ret = fread(fp_ptr.get(), 1, sizeof(tmp_buffer), tmp_fp);
        if (ret != sizeof(tmp_buffer)) {
            LOGGER->warn("{} read file:{} failed!", __FUNCTION__, itor);
            fclose(tmp_fp);
            return false;
        }
        string tmp_uuid((char *)tmp_buffer, ret);
        if (uuid_str.compare(tmp_uuid) != 0) {
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
            QString(tr("�ܽ��ȣ��� %1 ���ļ���ȫ��������ɣ�"))
            .arg(GLOBALCONFIG->GetFileCount());
    } else {
        total_progress_str =
            QString(tr("�ܽ��ȣ��� %1 ���ļ������ڴ���� %2 ���ļ�..."))
            .arg(GLOBALCONFIG->GetFileCount())
            .arg(GLOBALCONFIG->GetCurrentFileIndex());
    }
    ui.static_All->setText(total_progress_str);
    ui.progressBar_All->setValue(GLOBALCONFIG->GetTotalProgress() * 100);

    QString child_progress_str;
    if (1.0f == GLOBALCONFIG->GetChildProgress()) {
        child_progress_str =
            QString(tr("�ļ���%1 ������ɣ�"))
            .arg(GLOBALCONFIG->GetCurrentFileName().c_str());
    } else {
        child_progress_str =
            QString(tr("���ڴ����ļ���%1����ǰ����%2%..."))
            .arg(GLOBALCONFIG->GetCurrentFileName().c_str())
            .arg(GLOBALCONFIG->GetChildProgress() * 100);
    }
    ui.static_Child->setText(child_progress_str);
    ui.progressBar_Child->setValue(GLOBALCONFIG->GetChildProgress() * 100);
}

