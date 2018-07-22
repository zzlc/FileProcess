#include "FilePrcessDemo.h"
#include <windows.h>

std::string unicode_to_utf(std::wstring str)
{
    std::string return_value;
    //��ȡ�������Ĵ�С��������ռ䣬��������С�ǰ��ֽڼ����
    int len = WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
    char *buffer = new(std::nothrow) char[len + 1];
    if (NULL == buffer) {
        return NULL;
    }
    WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), buffer, len, NULL, NULL);
    buffer[len] = '\0';
    //ɾ��������������ֵ
    return_value.append(buffer);
    delete[]buffer;
    return return_value;
}

std::wstring utf_to_unicode(std::string str)
{
    //��ȡ�������Ĵ�С��������ռ䣬��������С�ǰ��ַ������
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    TCHAR *buffer = new(std::nothrow) TCHAR[len + 1];
    if (NULL == buffer) {
        return NULL;
    }
    //���ֽڱ���ת���ɿ��ֽڱ���
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
    buffer[len] = '\0';//����ַ�����β
                       //ɾ��������������ֵ
    std::wstring return_value;
    return_value.append(buffer);
    delete[]buffer;
    return return_value;
}

static bool FileExist(const string& file_name_)
{
    if (_access(file_name_.c_str(), 0) != 0) {
        return false;
    } else {
        return true;
    }
}

static int64_t GetFileSize(const string& file_name_)
{
    if (file_name_.empty()) {
        return -1;
    }
    if (!FileExist(file_name_)) {
        return -1;
    }
    FILE* fp = fopen(file_name_.c_str(), "rb");
    if (!fp) {
        return -1;
    }
    _fseeki64(fp, 0, SEEK_END);
    int64_t len = _ftelli64(fp);
    fclose(fp);
    return len;
}

FilePrcessDemo::FilePrcessDemo(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    connect(ui.sliceButton, SIGNAL(clicked()), this, SLOT(OnSliceBtnClick()));
    connect(ui.mergeButton, SIGNAL(clicked()), this, SLOT(OnMergeBtnClick()));

    _fp_ptr = FileProcessInterface::ObtainSingleInterface();
}

FilePrcessDemo::~FilePrcessDemo()
{
    if (_fp_ptr) {
        FileProcessInterface::ReleaseInterface(_fp_ptr);
        _fp_ptr = nullptr;
    }
}

void FilePrcessDemo::OnSliceBtnClick()
{
    if (!_fp_ptr) {
        return;
    }
    QString src_dir = SelectDir(QString::fromStdWString(L"ѡ��Դ�ļ���"));
    if (!src_dir.isEmpty()) {
        list<string> file_name_list;
        QueryDirectory(src_dir, file_name_list);
        if (file_name_list.empty()) {
            return ;
        }

        // ȷ��Ŀ���ļ��У�Ĭ��ͬԭ�ļ��� 
        string dest_dir = unicode_to_utf(SelectDir(QString::fromStdWString(L"ѡ��Ŀ���ļ���")).toStdWString());

        // �ж��ļ���С 
        int slice_count_(5), block_size(5 * 1024 * 1204);
        auto itor = file_name_list.begin();
        while (itor != file_name_list.end()) {
            int64_t file_length = GetFileSize(unicode_to_utf(src_dir.toStdWString()) + "/" + *itor);
            if (file_length < (slice_count_ + 1) * block_size) {
                QString msg = QString::fromLocal8Bit("�ļ� %1 ̫С���˿ͻ����ݲ�����")
                    .arg(utf_to_unicode(*itor).c_str());
                QMessageBox::warning(NULL, QString::fromLocal8Bit("����"),
                    msg);
                itor = file_name_list.erase(itor);
            } else {
                ++itor;
            }
        }
        if (file_name_list.empty()) {
            return;
        }

        int index(0);
        for (auto&& itor : file_name_list) {
            string dest_private_file_name;
            list<string> dest_public_file_name_list;
            _fp_ptr->SliceFile(
                string((char*)_aes128_key),
                "9e7f615a358f47d789d2a6580df5436d",
                unicode_to_utf(src_dir.toStdWString()),
                itor,
                slice_count_,
                dest_dir,
                dest_private_file_name,
                dest_public_file_name_list
            );
        }
    }
}

void FilePrcessDemo::OnMergeBtnClick()
{
    if (!_fp_ptr) {
        return;
    }
    QString private_file_name = SelectFile(QString::fromStdWString(L"ѡ��˽�п��ļ�"));
    if (private_file_name.isEmpty()) {
        return;
    }
    QStringList public_file_name_list = SelectFiles(QString::fromStdWString(L"ѡ���п��ļ�"));
    if (public_file_name_list.empty()) {
        return;
    }
    QString dest_path = SelectDir(QString::fromStdWString(L"ѡ��ϳ��ļ��洢�ļ���"));
    if (dest_path.isEmpty()) {
        return;
    }

    // ת���� stl
    list<string> public_file_list;
    for (auto&& itor : public_file_name_list) {
        public_file_list.push_back(unicode_to_utf(itor.toStdWString()));
    }
    
    string dest_file_name;
    _fp_ptr->MergeFile(
        unicode_to_utf(private_file_name.toStdWString()),
        public_file_list,
        string((char*)_aes128_key),
        unicode_to_utf(dest_path.toStdWString()),
        dest_file_name
    );
}

QString FilePrcessDemo::SelectFile(const QString& dlg_title_)
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

QStringList FilePrcessDemo::SelectFiles(const QString& dlg_title_)
{
    QStringList filters;
    return QFileDialog::getOpenFileNames(
        this,
        dlg_title_,
        "",
        tr("Any files(*)")
    );
}

QString FilePrcessDemo::SelectDir(const QString& dlg_title_)
{
    return QFileDialog::getExistingDirectory(
        this,
        dlg_title_,
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
}

int FilePrcessDemo::QueryDirectory(const QString& path_, list<string>& dest_file_vec_)
{
    if (path_.isEmpty()) {
        return -1;
    }
    QDir dir(path_);
    if (!dir.exists()) {
        return -1;
    }
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList file_list = dir.entryInfoList();
    for (auto&& itor : file_list) {
        dest_file_vec_.emplace_back(unicode_to_utf(itor.fileName().toStdWString()));
    }
    return 0;
}
