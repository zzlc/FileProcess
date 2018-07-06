// Domain.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <windows.h>
#include <string>
#include <process.h>

using namespace std;

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // 设置入口地址

int main(int argc, char *argv[])
{
    cout << __FUNCTION__ << endl;
    for (int i(0); i < argc; ++i) {
        cout << argv[i] << endl;
    }
    char strModule[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, strModule, 256); //得到当前模块路径

    cout << strModule << endl;

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

    // 参数分割
    string tmp_str = argv[1];
    tmp_str = tmp_str.erase(0, strlen("mediafileprocess://"));
    size_t tmp_pos(0);
    while ((tmp_pos = tmp_str.find("%20", tmp_pos)) != tmp_str.npos) {
        tmp_str = tmp_str.replace(tmp_pos, strlen("%20"), " ");
    }

    cmd_str += " ";
    cmd_str += tmp_str;
    cmd_str = cmd_str.erase(cmd_str.size() - 1, 1);
    cout << cmd_str.c_str() << endl;
    WinExec(cmd_str.c_str(), SW_SHOWNORMAL);

    getchar();
    getchar();
    cout << __FUNCTION__ << " end!" << endl;
    return 0;
}

