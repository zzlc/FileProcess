// Domain.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <windows.h>
#include <string>
#include <process.h>
#include <algorithm>  
#include <cmath>  
#include <vector>  
#include <string>  
#include <cstring>
#include <atlbase.h>

#pragma warning(disable:4996)  

using namespace std;

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // 设置入口地址

#ifdef _WIN32
// 判断是否已经写过注册表了
int CheckReg()
{
    HKEY hkey;
    LPCTSTR data_set = _T("MediaFileProcess");
    LSTATUS ret;
    if (ERROR_SUCCESS == (ret = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, data_set, 0, KEY_READ, &hkey))) {
        cout << "Check reg success!" << endl;
        ::RegCloseKey(hkey);
        return 0;
    } else {
        cout << "Check reg failed!" << endl;
        return -1;
    }
}

void WriteReg(const string& value_)
{
    HKEY hTempKey;

    LSTATUS ret;
    LPCTSTR data_set = _T("");
    char value_buf[256] = { 0 };
    if (ERROR_SUCCESS == (ret = ::RegCreateKey(HKEY_CLASSES_ROOT, _T("MediaFileProcess"), &hTempKey))) {
        if (ERROR_SUCCESS != (ret = ::RegSetValueEx(hTempKey, _T(""), 0, REG_SZ, (const BYTE*)"MediaFileProcess", strlen("MediaFileProcess")))) {
            cout << "写入错误, ret:" << ret << endl;
        }
        if (ERROR_SUCCESS != (ret = ::RegSetValueEx(hTempKey, _T("URL Protocol"), 0, REG_SZ, NULL, 0))) {
            cout << "写入错误, ret:" << ret << endl;
        }
    } else {
        cout << "写入错误, ret:" << ret << endl;
    }
    ::RegCloseKey(hTempKey);

    if (ERROR_SUCCESS == (ret = ::RegCreateKey(HKEY_CLASSES_ROOT, _T("MediaFileProcess\\DefaultIcon"), &hTempKey))) {
        int len = snprintf(value_buf, 256, "%s,1", value_.c_str());
        if (ERROR_SUCCESS != (ret = ::RegSetValueEx(hTempKey, _T(""), 0, REG_SZ, (const BYTE*)value_buf, len))) {
            cout << "写入错误, ret:" << ret << endl;
        }
    }

    if (ERROR_SUCCESS == (ret = ::RegCreateKey(HKEY_CLASSES_ROOT, _T("MediaFileProcess\\shell\\open\\command"), &hTempKey))) {

        int len = snprintf(value_buf, 256, "\"%s\"", value_.c_str());
        string tmp_str(value_buf, len);
        tmp_str += " %1";

        if (ERROR_SUCCESS != (ret = ::RegSetValueEx(hTempKey, _T(""), 0, REG_SZ, (const BYTE*)tmp_str.data(), tmp_str.size()))) {
            cout << "写入错误, ret:" << ret << endl;
        }
    }
    ::RegCloseKey(hTempKey);
}
#endif // _WIN32

int main(int argc, char *argv[])
{
    cout << __FUNCTION__ << endl;
    for (int i(0); i < argc; ++i) {
        cout << argv[i] << endl;
    }
    if (argc < 2) {
        return -1;
    }

    char strModule[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, strModule, 256); //得到当前模块路径

    cout << strModule << endl;

    // 首先判断注册表是否已经注册
    if (argc == 2 && strcmp(argv[1], "addreg") == 0) {
        printf("Check reg!\n");
        if (CheckReg() < 0) {
            printf("Begin write reg!\n");
            WriteReg(strModule);
        }
        return 0;
    }

    string a(strModule);
    size_t pos = a.find_last_of("\\");
    if (pos > 0) {
        a = a.erase(pos, a.size() - pos);
    }
    SetCurrentDirectory(a.c_str());  //设置

    GetCurrentDirectory(1000, strModule);
    cout << strModule << endl;

    string cmd_str = strModule;
    cmd_str += "\\FileProcess.exe";

#ifdef _WRITE_LOG
    FILE* fp = fopen("domain.log", "a+");
    char buf[128] = { 0 };
    itoa(argc, buf, 10);
    fwrite(buf, 1, strlen(buf), fp);
    fwrite("\n", 1, 1, fp);
    for (int i(0); i < argc; ++i) {
        fwrite(argv[i], 1, strlen(argv[i]), fp);
        fwrite("\n", 1, 1, fp);
        fflush(fp);
    }
    //fclose(fp);
#endif // _WRITE_LOG

    // 参数分割
    string tmp_str = argv[1];
    // 切掉头部
    tmp_str = tmp_str.erase(0, strlen("mediafileprocess://"));

    size_t tmp_pos(0);
    if (tmp_str.find("_") != tmp_str.npos) {
        while ((tmp_pos = tmp_str.find("_", tmp_pos)) != tmp_str.npos) {
            tmp_str = tmp_str.replace(tmp_pos, strlen("_"), " ");
        }
    }
    cmd_str += " ";
    cmd_str += tmp_str;
    cmd_str = cmd_str.erase(cmd_str.size() - 1, 1);

    // 如果还有更多参数 
    if (argc > 2) {
        for (int i(2); i < argc; ++i) {
            cmd_str += " ";
            cmd_str += argv[i];
        }
        cmd_str = cmd_str.erase(cmd_str.size() - 1, 1);
    }

    cout << cmd_str.c_str() << endl;
#ifdef _WRITE_LOG
    fwrite("cmd:", 1, 4, fp);
    fwrite(cmd_str.c_str(), 1, cmd_str.size(), fp);
    fwrite("\n", 1, 1, fp);
    fflush(fp);
    fclose(fp);
#endif // _WRITE_LOG

    WinExec(cmd_str.c_str(), SW_SHOWNORMAL);
    cout << __FUNCTION__ << " end!" << endl;
    return 0;
}

