#include "interface/BaseFileProcess.h"
#include "IBaseProcess.h"
#include <FileProcess.h>

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
}

IBaseFileProcess::~IBaseFileProcess()
{
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
    if (!FileProcess::CheckFiles(src_private_file_name, src_public_file_name_list)) {
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
