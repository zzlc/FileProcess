#pragma once
#include <memory>
#include <thread>
#include <string>
#include <vector>
#include <list>
#include <future>
#include <chrono>
#include <QUuid>
#include "Log.h"

using namespace std;

#define PUBLIC_FILE_NAME_FILTER "_public_"

const int block_size = 5 * 1024 * 1024;
const float private_file_percent = 0.0025f;  //˽�п�ռ��

typedef struct _TSliceParams 
{
    string          src_path;
    string          dest_path;
    string          file_name;             // �� src_path �����������������
    int             slice_count;           // �ֿ�����
    list<string>    public_file_name_list; // ���п��ļ����б�
    string          private_file_name;     // ˽�п��ļ���
    string          aes128_key;
    string          uuid;                  // ����ÿ���ļ�β�� UUID
}SliceParams, *PSliceParams;

class CGlobalConfig
{
public:
    ~CGlobalConfig();

    static CGlobalConfig* Instance();

    const uint8_t* GetAES128Key();
    const uint8_t* GetAES128Iv();

    void        SetFileCount(const int& file_count_) {
        _file_count.store(file_count_);
    }
    int         GetFileCount() {
        return _file_count.load();
    }
    void        SetCurrentFileIndex(const int& index_) {
        _current_file_index = index_;
    }
    int         GetCurrentFileIndex() {
        return _current_file_index.load();
    }
    void        SetCurrentFileName(const string& file_name_);
    const string& GetCurrentFileName();

    void        SetTotalProgress(float progress_) {
        _total_progress = progress_;
    }
    float       GetTotalProgress() {
        return _total_progress;
    }
    void        SetChildProgress(float progress_) {
        _child_progress = progress_;
    }
    float       GetChildProgress() {
        return _child_progress;
    }

    string      CreateUUID();

    int         GetEncrySize() {
        return _encry_size;
    }

protected:
    CGlobalConfig();

    CGlobalConfig(const CGlobalConfig&) = delete;
    CGlobalConfig operator = (const CGlobalConfig&) = delete;
    CGlobalConfig(const CGlobalConfig&&) = delete;

    static CGlobalConfig* _global_config_ptr;

private:
    const uint8_t   _aes128_key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xde, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    const uint8_t   _aes128_iv[16] = { 0xa1, 0xb2, 0xc3, 0xd4, 0xe5, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    mutex           _mutex;
    atomic_int      _file_count = 0;
    atomic_int      _current_file_index = 0;
    string          _current_file_name;    //���ڴ����е��ļ���
    atomic<float>   _total_progress = 0.0f;
    atomic<float>   _child_progress = 0.0f;
    const int       _encry_size = 8 * 1024; //���ܳ���
};

#define  GLOBALCONFIG CGlobalConfig::Instance()

extern std::wstring    utf8_to_wstring(const std::string& utf8_str_);
extern std::string     wstring_to_utf8(const std::wstring& wstr_);
extern std::string     unicode_to_utf(std::wstring str);
extern std::wstring    utf_to_unicode(std::string str);

extern bool            FileExist(const string& file_name_);
extern int64_t         GetFileSize(const string& file_name_);

