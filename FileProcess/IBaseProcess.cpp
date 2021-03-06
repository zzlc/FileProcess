#include "IBaseProcess.h"
#include <QMessageBox>
#include <io.h>
#include <direct.h>

#pragma execution_character_set("utf-8") 

IBaseProcess::IBaseProcess()
{
}

IBaseProcess::~IBaseProcess()
{
}

int IBaseProcess::SliceFile(const string& src_path_, const string& src_file_name_,
    const string& dest_path_, int slice_count_, char* aes_key_,
    const string& uuid_,
    __out string& private_child_file_name,
    __out list<string>& publish_child_file_name_list_)
{
    LOGGER->info("{} open file:{}, {}, dest_path:{}, slice count:{}, uuid:{}", 
        __FUNCTION__, src_path_, src_file_name_, dest_path_, slice_count_, uuid_);
    if (src_path_.empty() || src_file_name_.empty() || slice_count_ <= 1) {
        return -1;
    }
    char absolute_path[MAX_PATH] = { 0 };
    snprintf(absolute_path, MAX_PATH, "%s/%s", src_path_.c_str(), src_file_name_.c_str());
    if (_access(absolute_path, 0) != 0) {
        LOGGER->error("{} file:{} not exit!", __FUNCTION__, absolute_path);
        return -1;
    }
    _file_name = src_path_ + "/" + src_file_name_;

    if (GenerateChildFileName(src_path_, src_file_name_, 
        dest_path_, slice_count_, uuid_,
        private_child_file_name, publish_child_file_name_list_) < 0) {
        LOGGER->error("{} Generate child file name failed!", __FUNCTION__);
        return -1;
    }

    const int64_t file_length = GetFileSize(_file_name);
    int harf_header_size = file_length * private_file_percent;

    // 打开源文件
    shared_ptr<FILE> source_file_ptr(fopen(_file_name.c_str(), "rb"),
        [](FILE *fp) {if (fp) fclose(fp); });
    if (!source_file_ptr) {
        LOGGER->error("{} {} Open file to get metadata failed!", __FUNCTION__, __LINE__);
        return -1;
    }

    // 私有块
    shared_ptr<SliceFileInfo> private_slice_ptr =
        make_shared<SliceFileInfo>((char *)private_child_file_name.c_str(), block_size, aes_key_);
    // 公有块
    vector<shared_ptr<SliceFileInfo>> public_slice_vec;
    for (auto&& itor : publish_child_file_name_list_) {
        public_slice_vec.push_back(
            make_shared<SliceFileInfo>((char *)itor.c_str(), block_size, aes_key_)
        );
    }

    // 初始化读写文件的变量
    shared_ptr<uint8_t> read_buffer((uint8_t *)malloc(block_size),
        [](uint8_t *ptr) {free(ptr); });
    int read_length(0);
    int64_t rest_size(harf_header_size);

    // 获取文件头，并写入私有块
    while (rest_size > 0) {
        if (rest_size >= block_size) {
            read_length = fread(read_buffer.get(), 1, block_size, source_file_ptr.get());
        } else {
            read_length = fread(read_buffer.get(), 1, rest_size, source_file_ptr.get());
        }
        rest_size -= read_length;
        private_slice_ptr->SetData(read_buffer.get(), read_length);
    }
    GLOBALCONFIG->SetChildProgress(private_file_percent);

    // 继续写入公有块数据
    int public_index(0), current_size(0);
    rest_size = file_length - harf_header_size;
    while ((read_length = fread(read_buffer.get(), 1, block_size, source_file_ptr.get())) > 0) {
        if (rest_size - read_length < harf_header_size) {
            // 要留下 harf_header_size 大小的数据量用于写入私有块
            public_slice_vec[public_index++]->SetData(read_buffer.get(), rest_size - harf_header_size);
            private_slice_ptr->SetData(
                read_buffer.get() + rest_size - harf_header_size, 
                read_length - rest_size + harf_header_size
            );
            // 读取剩余的数据，并写入私有块
            while ((read_length = fread(read_buffer.get(), 1, block_size, source_file_ptr.get())) > 0) {
                private_slice_ptr->SetData(
                    read_buffer.get(),
                    read_length
                );
            }
            //文件读取完毕
            break;
        } else {
            public_slice_vec[public_index++]->SetData(read_buffer.get(), read_length);
        }
        if (public_index >= slice_count_) {
            public_index = 0;
        }
        rest_size -= read_length;

        GLOBALCONFIG->SetChildProgress(1.0f - 1.0f * rest_size / file_length);
    }
    GLOBALCONFIG->SetChildProgress(1.0f);

    // 统一写入 uuid
    private_slice_ptr->SetData((uint8_t *)uuid_.data(), uuid_.size());
    for (auto&& itor : public_slice_vec) {
        itor->SetData((uint8_t *)uuid_.data(), uuid_.size());
    }

    LOGGER->info("{} open media file:{} success.", __FUNCTION__, src_file_name_);
    return 0;
}

int IBaseProcess::MergeFile(
    const string& src_private_file_name_, 
    const list<string>& src_public_file_name_list_, 
    char* aes_key_,
    const string& dest_path_, 
    __out string& dest_file_name_)
{
    LOGGER->info("{} src private file name:{}, dest_path:{}", __FUNCTION__, src_private_file_name_, dest_path_);
    for (auto&& itor : src_public_file_name_list_) {
        LOGGER->info("src public file name:{}", itor);
        if (!FileExist(itor)) {
            LOGGER->error("{} public file:{} not exist!", __FUNCTION__, itor);
            return -1;
        }
    }
    if (src_public_file_name_list_.empty()) {
        LOGGER->error("{} none public file!", __FUNCTION__);
        return -1;
    }
    // 校验文件是否存在
    if (!FileExist(src_private_file_name_)) {
        LOGGER->error("{} private file:{} not exist!", __FUNCTION__, src_private_file_name_);
        return -1;
    }
    if (!FileExist(dest_path_.c_str())) {
        if (0 != _mkdir(dest_path_.c_str())) {
            LOGGER->error("{} mkdir:{} failed!", __FUNCTION__, dest_path_);
            return -1;
        }
    }
    if (!aes_key_) {
        LOGGER->error("{} aes key is null!", __FUNCTION__);
        return -1;
    }

    // 解密准备秘钥
    InitAesKey((uint8_t *)aes_key_);

    int encrypt_size = GLOBALCONFIG->GetEncrySize();

    // 生成目标文件的名称
    int last_pos1 = src_public_file_name_list_.front().find_last_of('/');
    int last_pos2 = src_public_file_name_list_.front().find("_pub_");
    string dest_name = dest_path_ + "/" + src_public_file_name_list_.front().substr(last_pos1 + 1, last_pos2 - last_pos1 - 1);
    LOGGER->info("{} Gererate dest file name:{} success.", __FUNCTION__, dest_name);
    FILE* dest_fp = fopen(dest_name.c_str(), "wb");
    if (!dest_fp) {
        LOGGER->error("{} open dest file:{} to write failed!", __FUNCTION__, dest_name);
        return -1;
    }

    // 估算目标文件总大小
    int64_t private_file_size = GetFileSize(src_private_file_name_);
    int64_t file_size_count = private_file_size + GetFileSize(src_public_file_name_list_.front()) * src_public_file_name_list_.size();

    GLOBALCONFIG->SetFileCount(1);
    GLOBALCONFIG->SetCurrentFileIndex(1);
    GLOBALCONFIG->SetCurrentFileName(dest_name);
    GLOBALCONFIG->SetTotalProgress(0.0f);

    // 开始循序读取数据块，并写入文件
    shared_ptr<MergeFileInfo> private_fp_ptr = make_shared<MergeFileInfo>(
        (char *)src_private_file_name_.c_str(), (uint8_t *)aes_key_
        );
        
    vector<shared_ptr<MergeFileInfo>> public_fp_vec;
    for (auto&& itor : src_public_file_name_list_) {
        public_fp_vec.emplace_back(
            make_shared<MergeFileInfo>(
            (char *)itor.c_str(), (uint8_t *)aes_key_
            )
        );
    }

    // 读取并写入私有块的前一半
    int tmp_length(0), read_length(0), read_count(0), ret(0), progress_size(0);
    uint8_t* read_buffer = (uint8_t*)malloc(block_size);
    do 
    {
        //read_length = fread((void *)&tmp_length, 1, sizeof(int), private_fp_ptr->fp);
        read_length = private_fp_ptr->ReadData((uint8_t*)&tmp_length, sizeof(int));
        if (read_length < sizeof(int)) {
            LOGGER->warn("{} line {} private file end!", __FUNCTION__, __LINE__);
            break;
        }
        read_count += tmp_length;
        if (read_count > private_file_size / 2) {
            ret = fseek(private_fp_ptr->fp, 0 - sizeof(int), SEEK_CUR);
            break;
        }

        //read_length = fread(read_buffer, 1, tmp_length, private_fp_ptr.get());
        read_length = private_fp_ptr->ReadData(read_buffer, tmp_length);
        if (read_length > 0) {
            //if (read_length >= encrypt_size) {
            //    // 解密
            //    AES_CBC_decrypt_buffer(&ctx, read_buffer, encrypt_size);
            //}
            fwrite(read_buffer, 1, read_length, dest_fp);
            progress_size += read_length;
            GLOBALCONFIG->SetChildProgress(1.0f * progress_size / file_size_count);
        }
    } while (true);

    // 循环读取公有块，并写入
    bool flag = false;
    do 
    {
        flag = false;
        for (auto&& itor : public_fp_vec) {
            //read_length = fread((void *)&tmp_length, 1, sizeof(int), itor.get());
            read_length = itor->ReadData((uint8_t*)&tmp_length, sizeof(int));
            if (read_length < sizeof(int)) {
                LOGGER->warn("{} line {} public file end!", __FUNCTION__, __LINE__);
                break;
            }
            //read_length = fread(read_buffer, 1, tmp_length, itor.get());
            read_length = itor->ReadData(read_buffer, tmp_length);
            if (read_length != tmp_length) {
                LOGGER->warn("{} length exception!", __FUNCTION__);
                break;
            }
            // 判断是否是 UUID
            if (32 == read_length) {
                char c = fgetc(itor->fp);
                if (feof(itor->fp)) {
                    // 此文件结束了，后面的 UUID 不要写入
                    continue;
                } else {
                    fseek(itor->fp, -1, SEEK_CUR);
                }
            }

            if (read_length > 0) {
                //if (read_length >= encrypt_size) {
                //    // 解密
                //    AES_CBC_decrypt_buffer(&ctx, read_buffer, encrypt_size);
                //}
                fwrite(read_buffer, 1, read_length, dest_fp);
                flag = true;
                progress_size += read_length;
                GLOBALCONFIG->SetChildProgress(1.0f * progress_size / file_size_count);
            }
        }
        if (!flag) {
            break;
        }
    } while (true);

    // 继续写入私有块的后半块
    do {
        //read_length = fread((void *)&tmp_length, 1, sizeof(int), private_fp_ptr.get());
        read_length = private_fp_ptr->ReadData((uint8_t*)&tmp_length, sizeof(int));
        if (read_length < sizeof(int)) {
            LOGGER->warn("{} line {} private file end!", __FUNCTION__, __LINE__);
            break;
        }
        //read_length = fread(read_buffer, 1, tmp_length, private_fp_ptr.get());
        read_length = private_fp_ptr->ReadData(read_buffer, tmp_length);
        if (32 == read_length) {
            char c = fgetc(private_fp_ptr->fp);
            if (feof(private_fp_ptr->fp)) {
                // 此文件结束了，后面的 UUID 不要写入
                continue;
            } else {
                fseek(private_fp_ptr->fp, -1, SEEK_CUR);
            }
        }
        if (read_length > 0) {
            //if (read_length >= encrypt_size) {
            //    // 解密
            //    AES_CBC_decrypt_buffer(&ctx, read_buffer, encrypt_size);
            //}
            fwrite(read_buffer, 1, read_length, dest_fp);
            progress_size += read_length;
            GLOBALCONFIG->SetChildProgress(1.0f * progress_size / file_size_count);
        }
    } while (true);

    fclose(dest_fp);
    free(read_buffer);

    GLOBALCONFIG->SetChildProgress(1.0f);
    GLOBALCONFIG->SetTotalProgress(1.0f);

    return 0;
}

int IBaseProcess::CloseFile()
{
    LOGGER->info("{} ", __FUNCTION__);
    return 1;
}

int IBaseProcess::GenerateChildFileName(
    const string& src_path_, const string& src_file_name_, 
    const string& dest_path_, int slice_count_, 
    const string& uuid_str_,
    __out string& private_child_file_name, __out list<string>& publish_child_file_name_list_)
{
    if (src_file_name_.empty() || slice_count_ <= 0) {
        LOGGER->warn("{} params error! file name:{}, slice_count:{}", 
            __FUNCTION__, src_file_name_, slice_count_);
        return -1;
    }
    char buffer[MAX_PATH] = { 0 };
    int len = snprintf(buffer, MAX_PATH, "%s/%s_pri_%s", 
        dest_path_.c_str(), src_file_name_.c_str(), uuid_str_.c_str());
    private_child_file_name.clear();
    private_child_file_name.append(buffer, len);
    for (int i(1); i <= slice_count_; ++i) {
        memset(buffer, 0, MAX_PATH);
        len = snprintf(buffer, MAX_PATH, "%s/%s_pub_%s_%d", 
            dest_path_.c_str(), src_file_name_.c_str(), uuid_str_.c_str(), i);
        publish_child_file_name_list_.emplace_back(string(buffer, len));
    }
    return 0;
}

void IBaseProcess::InitAesKey(const uint8_t* key_)
{
    if (!key_) {
        return;
    }
    AES_init_ctx_iv(&ctx, key_, GLOBALCONFIG->GetAES128Iv());
}
