#include "GlobalConfig.h"
#include <io.h>

CSpdLog* CSpdLog::_spd_logger_ptr = nullptr;
CGlobalConfig* CGlobalConfig::_global_config_ptr = nullptr;

CGlobalConfig::CGlobalConfig()
{
    LOGGER->info("{}", __FUNCTION__);
}

CGlobalConfig::~CGlobalConfig()
{
    LOGGER->info("{}", __FUNCTION__);
    LOGGER->flush();
}

CGlobalConfig* CGlobalConfig::Instance()
{
    if (!_global_config_ptr) {
        _global_config_ptr = new CGlobalConfig;
    }
    return _global_config_ptr;
}

const uint8_t* CGlobalConfig::GetAES128Key()
{
    return (uint8_t *)_aes128_key;
}

const uint8_t* CGlobalConfig::GetAES128Iv()
{
    return (uint8_t*)_aes128_iv;
}

void CGlobalConfig::SetCurrentFileName(const string& file_name_)
{
    lock_guard<mutex> lock_(_mutex);
    _current_file_name = file_name_;
}

const std::string& CGlobalConfig::GetCurrentFileName()
{
    lock_guard<mutex> lock_(_mutex);
    return _current_file_name;
}

std::string CGlobalConfig::CreateUUID()
{
    //QUuid uid = QUuid::createUuid();
    //return uid.toString().remove("{").remove("}").remove("-").toStdString();
    return "";
}

std::wstring utf8_to_wstring(const std::string& utf8_str_)
{
    int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, utf8_str_.c_str(), utf8_str_.size(), NULL, 0);
    wchar_t* pUnicode;
    pUnicode = new wchar_t[unicodeLen + 1];
    memset((void*)pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
    ::MultiByteToWideChar(CP_UTF8, 0, utf8_str_.c_str(), utf8_str_.size(), (LPWSTR)pUnicode, unicodeLen);
    std::wstring wstrReturn(pUnicode);
    delete[] pUnicode;
    return wstrReturn;
}

std::string wstring_to_utf8(const std::wstring& wstr_)
{
    char* pElementText;
    int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)wstr_.c_str(), wstr_.size(), NULL, 0, NULL, NULL);
    pElementText = new char[iTextLen + 1];
    memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
    ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)wstr_.c_str(), wstr_.size(), pElementText, iTextLen, NULL, NULL);
    std::string strReturn(pElementText);
    delete[] pElementText;
    return strReturn;
}

std::string unicode_to_utf(std::wstring str)
{
    std::string return_value;
    //获取缓冲区的大小，并申请空间，缓冲区大小是按字节计算的
    int len = WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
    char *buffer = new(std::nothrow) char[len + 1];
    if (NULL == buffer) {
        return NULL;
    }
    WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), buffer, len, NULL, NULL);
    buffer[len] = '\0';
    //删除缓冲区并返回值
    return_value.append(buffer);
    delete[]buffer;
    return return_value;
}

std::wstring utf_to_unicode(std::string str)
{
    //获取缓冲区的大小，并申请空间，缓冲区大小是按字符计算的
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    TCHAR *buffer = new(std::nothrow) TCHAR[len + 1];
    if (NULL == buffer) {
        return NULL;
    }
    //多字节编码转换成宽字节编码
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
    buffer[len] = '\0';//添加字符串结尾
                       //删除缓冲区并返回值
    std::wstring return_value;
    return_value.append(buffer);
    delete[]buffer;
    return return_value;
}

extern bool FileExist(const string& file_name_)
{
    if (_access(file_name_.c_str(), 0) != 0) {
        return false;
    } else {
        return true;
    }
}

extern int64_t GetFileSize(const string& file_name_)
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

