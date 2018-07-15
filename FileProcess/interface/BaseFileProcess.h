#pragma once
#include "interface/FileProcessInterface.h"
#include <memory>

using namespace std;

class IBaseFileProcess : public FileProcessInterface
{
public:
    static FileProcessInterface* ObtainSingleInterface();

protected:
    IBaseFileProcess();
    virtual ~IBaseFileProcess();

    virtual int SliceFile(
        const std::string& aes_key,
        const std::string& uuid,
        const std::string& src_path,
        const std::string& src_file_name,
        const int& slice_num,
        const std::string& dest_path,
        __out std::string& dest_private_file_name,
        __out std::list<std::string>& dest_public_file_name_list
    );

    virtual int MergeFile(
        const std::string& src_private_file_name,
        const list<std::string>& src_public_file_name_list,
        const std::string& aes_key,
        const std::string& dest_path,
        __out std::string& dest_file_name
    );

protected:
    static IBaseFileProcess* _base_fp_ptr;
};

