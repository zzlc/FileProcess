// 此文件编码格式为 UTF-8 格式，接口形参务必均以 UTF-8 编码格式传入
// 否则易产生中文乱码现象

#pragma once
#include <list>
#include <string>

#ifdef DLL_EXPORT
#define _EXPORT_INTERFACE _declspec(dllexport)
#else 
#define _EXPORT_INTERFACE _declspce(dllimport)
#endif // DLL_EXPORT

class _EXPORT_INTERFACE FileProcessInterface
{
public:
    // 获取 FileProcessInterface 接口类的实例指针，用于后续的所有操作，此接口为单实例
    static FileProcessInterface* ObtainSingleInterface();

    // 释放 FileProcessInterface 对象实例
    // @param [in] interface_ptr
    //        由 FileProcessInterface::ObtainSingleInterface() 生成
    // @return 0:success; -1:failed
    int ReleaseInterface(FileProcessInterface* interface_ptr);

    // 分割文件并加密存储
    // @param [in] aes_key
    //        aes128 加密算法秘钥， 16 个字节（字符串长度为 32 字节），如：
    //        { 0xa1, 0xb2, 0xc3, 0xd4, 0xe5, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f }
    // @param [in] uuid
    //        GUID，不带{} 和 -,用于写入分块文件的尾部
    // @param [in] src_path
    //        带分割文件所在路径
    // @param [in] src_file_name
    //        待分割处理文件名
    // @param [in] slice_num
    //        分块数量
    // @param [in] dest_path
    //        分割后存储文件的目录
    // @param [out] dest_private_file_name
    //        私有块文件目标文件名
    // @param [out] dest_public_file_name_list
    //        公有块文件目标文件名列表
    // @return 0:success; -1:failed
    virtual int SliceFile(
        const std::string& aes_key,
        const std::string& uuid,
        const std::string& src_path,
        const std::string& src_file_name, 
        const int& slice_num,
        const std::string& dest_path,
        __out std::string& dest_private_file_name,
        __out std::list<std::string>& dest_public_file_name_list
    ) = 0;

    // 指定私有块和公有块文件进行合并，并输出到目标文件夹 
    // @param [in] src_private_file_name
    //        私有块文件名（带绝对路径）
    // @param [in] src_public_file_name_list
    //        公有文件名列表（带绝对路径）
    // @param [in] aes_key
    //        aes128 加密算法秘钥， 16 个字节（字符串长度为 32 字节），如：
    //        { 0xa1, 0xb2, 0xc3, 0xd4, 0xe5, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f }
    // @param [in] dest_path
    //        目标文件存储路径
    // @param [out] dest_file_name
    //        输出的合成后的目标文件名
    // @return 0:success; -1:failed; -2:文件 uuid 不匹配
    virtual int MergeFile(
        const std::string& src_private_file_name,
        const std::list<std::string>& src_public_file_name_list,
        const std::string& aes_key,
        const std::string& dest_path,
        __out std::string& dest_file_name
    ) = 0;

protected:
    virtual ~FileProcessInterface() {};
};