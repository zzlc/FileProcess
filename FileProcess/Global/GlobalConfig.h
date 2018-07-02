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
const float private_file_percent = 0.0025f;  //私有块占比

typedef struct _TSliceParams 
{
    string          src_path;
    string          dest_path;
    string          file_name;             // 与 src_path 互补，具体意义待定
    int             slice_count;           // 分块数量
    list<string>    public_file_name_list; // 公有块文件名列表
    string          private_file_name;     // 私有块文件名
    string          aes128_key;
    string          uuid;                  // 加在每个文件尾的 UUID
}SliceParams, *PSliceParams;

class CGlobalConfig
{
public:
    ~CGlobalConfig();

    static CGlobalConfig* Instance();

    void        SetAES128Key(const char* key_);
    char*       GetAES128Key();

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

protected:
    CGlobalConfig();

    CGlobalConfig(const CGlobalConfig&) = delete;
    CGlobalConfig operator = (const CGlobalConfig&) = delete;
    CGlobalConfig(const CGlobalConfig&&) = delete;

    static CGlobalConfig* _global_config_ptr;

private:
    char            _aes128_key[16] = { 0 };
    mutex           _mutex;
    atomic_int      _file_count = 0;
    atomic_int      _current_file_index = 0;
    string          _current_file_name;    //正在处理中的文件名
    atomic<float>   _total_progress = 0.0f;
    atomic<float>   _child_progress = 0.0f;
};

#define  GLOBALCONFIG CGlobalConfig::Instance()

extern std::wstring    utf8_to_wstring(const std::string& utf8_str_);
extern std::string     wstring_to_utf8(const std::wstring& wstr_);
extern std::string     unicode_to_utf(std::wstring str);
extern std::wstring    utf_to_unicode(std::string str);

extern bool            FileExist(const string& file_name_);
extern int64_t         GetFileSize(const string& file_name_);

