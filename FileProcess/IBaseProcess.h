#pragma once
#include "demux/demuxer.h"
#include "Global/GlobalConfig.h"
extern "C" {
#include "aes128/aes128.h"
}

struct SliceFileInfo
{
    FILE* fp                     = nullptr;
    uint8_t* buffer              = nullptr;
    uint8_t* encry_buffer        = nullptr;
    int encry_size               = -1;       //���ܳ��ȣ�0�������ܣ�-1��ȫ�����ܣ�> 0:����ָ������
    int max_buffer_size          = 0;        // buffer ��������С
    int payload_size             = 0;        // Ŀǰ����������Ч���ݳ���
    mutex buffer_muter;
    atomic_bool update_flag      = false;
    bool exit_flag               = false;
    thread write_thread;
    char *key                    = "0f1571c947d9e859abb7add6af7f6798";
    int password[4][4]           = { 0 };

    SliceFileInfo(char* file_name_, int buffer_size_) {
        errno_t ret = fopen_s(&fp, file_name_, "wb");
        if (0 != ret) {
            LOGGER->warn("{} open file:%s failed, errno:{}", __FUNCTION__, file_name_, ret);
        } else {
            max_buffer_size = buffer_size_;
            buffer          = (uint8_t *)malloc(buffer_size_);
            encry_buffer    = (uint8_t *)malloc(1024);
        }

        for (int m = 0; m < 4; ++m) {
            for (int i = 0; i < 4; ++i) {
                int indx = 4 * i + m;
                password[i][m] = 16 * c2i(key[indx]) + c2i(key[indx + 1]);
            }
        }
        write_thread = thread(&SliceFileInfo::WriteThread, this);
    }
    ~SliceFileInfo() {
        exit_flag = true;
        write_thread.join();

        if (fp) {
            fclose(fp);
            fp = nullptr;
        }
        if (buffer) {
            free(buffer);
            buffer = nullptr;
        }
        if (encry_buffer) {
            free(encry_buffer);
            encry_buffer = nullptr;
        }
    }
    int SetData(uint8_t* buffer_, const int& len_) {
        if (!buffer_ || len_ <= 0) {
            return -1;
        }
        // ����ȴ������ݱ�д�룬����ᶪʧ����
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
                //���ļ�ת����16�ֽڵ�int��������ܡ�����
                if (payload_size < 1024) {
                    fwrite((void *)&payload_size, 1, sizeof(payload_size), fp);
                    fwrite(buffer, 1, payload_size, fp);
                } else {
                    for (int i = 0; i < 1024 / 16; ++i) {

                        int content_to_int[4][4];
                        for (int j = 0; j < 4; ++j) {
                            for (int k = 0; k < 4; ++k) {
                                content_to_int[j][k] = buffer[j * 4 + k + 16 * i];
                            }
                        }
                        aes_detail(content_to_int, password, 1);
                        for (int j = 0; j < 4; ++j) {
                            for (int k = 0; k < 4; ++k) {
                                encry_buffer[j * 4 + k + 16 * i] = content_to_int[j][k];
                            }
                        }
                    }
                    // �Ȳ�����
                    //fwrite(encry_buffer, 1, 1024, fp);
                    //fwrite(buffer + 1024, 1, payload_size - 1024, fp);
                    fwrite((char *)&payload_size, 1, sizeof(payload_size), fp);
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

class IBaseProcess
{
public:
    IBaseProcess();
    virtual ~IBaseProcess();

    // �򿪲��ֿ�
    virtual int SliceFile(
        const string& src_path_,
        const string& src_file_name_, 
        const string& dest_path_,
        int slice_count_, 
        __out string& private_child_file_name,
        __out list<string>& publish_child_file_name_list_
    );

    // ָ��˽�п�͹��п��ļ����кϲ��������Ŀ���ļ�
    virtual int MegerFile(
        const string& src_private_file_name_, 
        const list<string>& src_publish_file_name__list_,
        const string& dest_path_, 
        __out string& dest_file_name_
    );

    virtual int CloseFile();

    // ��ʼ����
    //virtual int StartEncrpt();

protected:
    // ���ɹ��п��˽�п��ļ���
    int         GenerateChildFileName(
        const string& src_path_,
        const string& src_file_name_,
        const string& dest_path_,
        int slice_count_,
        __out string& private_child_file_name,
        __out list<string>& publish_child_file_name_list_
    );


protected:
    demuxer_t*  _ffmpeg_demux_ptr = nullptr;
    string      _file_name;
    string      _key;
};

