#pragma once
#include <memory>
#include <thread>
#include <string>
#include <vector>
#include <list>
#include <future>
#include <chrono>
#include "Log.h"

using namespace std;

#define PUBLIC_FILE_NAME_FILTER "_public_"

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
    CGlobalConfig();
    ~CGlobalConfig();

    void        SetAES128Key(const char* key_);
    const char* GetAES128Key();

private:
    string _aes128_key = "0f1571c947d9e859abb7add6af7f6798";
};

extern std::wstring    utf8_to_wstring(const std::string& utf8_str_);
extern std::string     wstring_to_utf8(const std::wstring& wstr_);
extern std::string     unicode_to_utf(std::wstring str);
extern std::wstring    utf_to_unicode(std::string str);

extern bool            FileExist(const string& file_name_);

