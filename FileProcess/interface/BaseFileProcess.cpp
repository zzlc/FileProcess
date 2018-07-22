#include "interface/BaseFileProcess.h"
#include "IBaseProcess.h"
//#include <FileProcess.h>

IBaseFileProcess* IBaseFileProcess::_base_fp_ptr = nullptr;

FileProcessInterface* IBaseFileProcess::ObtainSingleInterface()
{
    if (!_base_fp_ptr) {
        _base_fp_ptr = new IBaseFileProcess;
    }
    return dynamic_cast<FileProcessInterface*>(_base_fp_ptr);
}

IBaseFileProcess::IBaseFileProcess()
{
    CSpdLog::Instance()->SetLogFileName("log.txt");
}

IBaseFileProcess::~IBaseFileProcess()
{
    CSpdLog::Instance()->GetLogger()->flush();
    _base_fp_ptr = nullptr;
}

int IBaseFileProcess::SliceFile(
    const std::string& aes_key,
    const std::string& uuid,
    const std::string& src_path,
    const std::string& src_file_name,
    const int& slice_num,
    const std::string& dest_path,
    __out std::string& dest_private_file_name,
    __out std::list<std::string>& dest_public_file_name_list)
{
    if (aes_key.empty() || uuid.empty() || src_path.empty() || src_file_name.empty()
        || slice_num <= 0 || dest_path.empty()) {
        // params error
        return -1;
    }
    shared_ptr<IBaseProcess> _file_fp_ptr(new IBaseProcess);
    return _file_fp_ptr->SliceFile(
        src_path, 
        src_file_name, 
        dest_path,
        slice_num,
        (char *)aes_key.c_str(),
        uuid,
        dest_private_file_name,
        dest_public_file_name_list
    );
}

int IBaseFileProcess::MergeFile(
    const std::string& src_private_file_name, 
    const list<std::string>& src_public_file_name_list, 
    const std::string& aes_key, 
    const std::string& dest_path, 
    __out std::string& dest_file_name)
{
    if (src_private_file_name.empty() || src_public_file_name_list.empty()
        || aes_key.empty()) {
        // params error
        return -1;
    }
    shared_ptr<IBaseProcess> _file_fp_ptr(new IBaseProcess);
    if (!CheckFiles(src_private_file_name, src_public_file_name_list)) {
        return -2;
    }

    return _file_fp_ptr->MergeFile(
        src_private_file_name,
        src_public_file_name_list,
        (char*)aes_key.c_str(),
        dest_path,
        dest_file_name
    );
}

bool IBaseFileProcess::CheckFiles(const string& private_file_name_, const list<string>& public_file_list_)
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
