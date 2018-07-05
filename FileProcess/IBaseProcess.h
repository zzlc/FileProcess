#pragma once
#include "Global/GlobalConfig.h"
#include <QObject>
extern "C" {
#include "aes128/aes.h"
}

struct SliceFileInfo
{
    FILE* fp                     = nullptr;
    uint8_t* buffer              = nullptr;
    int encry_size               = 0;        //加密长度；0：不加密；-1：全部加密；> 0:加密指定长度,必须为 16 的整倍数
    int max_buffer_size          = 0;        // buffer 缓冲区大小
    int payload_size             = 0;        // 目前缓冲区内有效数据长度
    mutex buffer_muter;
    atomic_bool update_flag      = false;
    bool exit_flag               = false;
    thread write_thread;
    struct AES_ctx  ctx;
    string  file_name;

    SliceFileInfo(char* file_name_, int buffer_size_, char* aes_key_) {
        file_name = file_name_;
        encry_size = GLOBALCONFIG->GetEncrySize();
        AES_init_ctx_iv(&ctx, (uint8_t *)aes_key_, GLOBALCONFIG->GetAES128Iv());

        errno_t ret = fopen_s(&fp, file_name_, "wb");
        if (0 != ret) {
            LOGGER->warn("{} open file:%s failed, errno:{}", __FUNCTION__, file_name_, ret);
        } else {
            max_buffer_size = buffer_size_;
            buffer          = (uint8_t *)malloc(buffer_size_);
        }

        write_thread = thread(&SliceFileInfo::WriteThread, this);
    }
    ~SliceFileInfo() {
        exit_flag = true;
        write_thread.join();

        if (fp) {
            fflush(fp);
            fclose(fp);
            fp = nullptr;
        }
        if (buffer) {
            free(buffer);
            buffer = nullptr;
        }
    }
    int SetData(uint8_t* buffer_, const int& len_) {
        if (!buffer_ || len_ <= 0) {
            return -1;
        }
        // 必须等待旧数据被写入，否则会丢失数据
        while (update_flag) {
            this_thread::sleep_for(chrono::milliseconds(1));
        }
        lock_guard<mutex> lock_(buffer_muter);
        if (len_ > max_buffer_size) {
            return -1;
        }
        memcpy(buffer, buffer_, len_);
        payload_size = len_;
        update_flag = true;
        return payload_size;
    }
    void WriteThread() {
        while (!exit_flag)
        {
            do {
                lock_guard<mutex> lock_(buffer_muter);
                if (!update_flag) {
                    break;
                }
                //将文件转换成16字节的int型数组加密、解密
                if (payload_size < encry_size) {
                    fwrite((void *)&payload_size, 1, sizeof(payload_size), fp);
                    fwrite(buffer, 1, payload_size, fp);
                } else {
                    fwrite((char *)&payload_size, 1, sizeof(payload_size), fp);
                    AES_CBC_encrypt_buffer(&ctx, buffer, encry_size);
                    fwrite(buffer, 1, payload_size, fp);
                }
                update_flag = false;
            } while (false);
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    }

protected:
    SliceFileInfo() = delete;
    const SliceFileInfo(const SliceFileInfo&) = delete;
    SliceFileInfo operator = (const SliceFileInfo&) = delete;
};

struct MergeFileInfo {
    FILE* fp = NULL;
    int decrypt_size = 0;
    struct AES_ctx  ctx;
    MergeFileInfo(char* file_name_, uint8_t* aes_key_) {
        decrypt_size = GLOBALCONFIG->GetEncrySize();
        AES_init_ctx_iv(&ctx, (uint8_t *)aes_key_, GLOBALCONFIG->GetAES128Iv());

        errno_t ret = fopen_s(&fp, file_name_, "rb");
        if (0 != ret) {
            fp = NULL;
            LOGGER->warn("{} open file:%s failed, errno:{}", __FUNCTION__, file_name_, ret);
        }
    }
    ~MergeFileInfo() {
        if (fp) {
            fclose(fp);
            fp = NULL;
        }
    }
    int ReadData(uint8_t* buffer_, const int& len_) {
        if (!buffer_ || len_ <= 0 || !fp) {
            return 0;
        }
        int ret = fread(buffer_, 1, len_, fp);
        if (ret >= decrypt_size) {
            // 解密
            AES_CBC_decrypt_buffer(&ctx, buffer_, decrypt_size);
        }
        return ret;
    }
};

class IBaseProcess : public QObject
{
    Q_OBJECT
public:
    IBaseProcess();
    virtual ~IBaseProcess();

    // 打开并分块
    virtual int SliceFile(
        const string& src_path_,
        const string& src_file_name_, 
        const string& dest_path_,
        int slice_count_, 
        char* aes_key_,
        const string& uuid_,
        __out string& private_child_file_name,
        __out list<string>& publish_child_file_name_list_
    );

    // 指定私有块和公有块文件进行合并，并输出目标文件
    virtual int MergeFile(
        const string& src_private_file_name_, 
        const list<string>& src_publish_file_name__list_,
        char* aes_key_,
        const string& dest_path_, 
        __out string& dest_file_name_
    );

    virtual int CloseFile();

    // 开始加密
    //virtual int StartEncrpt();

protected:
    // 生成公有块和私有块文件名
    int         GenerateChildFileName(
        const string& src_path_,
        const string& src_file_name_,
        const string& dest_path_,
        int slice_count_,
        const string& uuid_str_,
        __out string& private_child_file_name,
        __out list<string>& publish_child_file_name_list_
    );

    void InitAesKey(const uint8_t* key_);

protected:
    string      _file_name;
    struct AES_ctx  ctx;
};

