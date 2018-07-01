#include "IBaseProcess.h"
#include <io.h>
#include <direct.h>

IBaseProcess::IBaseProcess()
{
}

IBaseProcess::~IBaseProcess()
{
}

int IBaseProcess::SliceFile(const string& src_path_, const string& src_file_name_,
    const string& dest_path_, int slice_count_, char* aes_key_,
    __out string& private_child_file_name,
    __out list<string>& publish_child_file_name_list_)
{
    LOGGER->info("{} open file:{}, {}, dest_path:{}, slice count:{}", 
        __FUNCTION__, src_path_, src_file_name_, dest_path_, slice_count_);
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
        dest_path_, slice_count_, 
        private_child_file_name, publish_child_file_name_list_) < 0) {
        LOGGER->error("{} Generate child file name failed!", __FUNCTION__);
        return -1;
    }

    const int64_t file_length = GetFileSize(_file_name);
    int harf_header_size = file_length * private_file_percent;
    int key_frame_encrypt_header_len = 1024; // 加密头部长度

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

    LOGGER->info("{} open media file:{} success.", __FUNCTION__, src_file_name_);
    return 0;
}

int IBaseProcess::MegerFile(
    const string& src_private_file_name_, 
    const list<string>& src_public_file_name_list_, 
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
    // 生成目标文件的名称
    int last_pos1 = src_public_file_name_list_.front().find_last_of('/');
    int last_pos2 = src_public_file_name_list_.front().find("_public_");
    string dest_name = dest_path_ + "/" + src_public_file_name_list_.front().substr(last_pos1 + 1, last_pos2 - last_pos1 - 1);
    LOGGER->info("{} Gererate dest file name:{} success.", __FUNCTION__, dest_name);
    FILE* dest_fp = fopen(dest_name.c_str(), "wb");
    if (!dest_fp) {
        LOGGER->error("{} open dest file:{} to write failed!", __FUNCTION__, dest_name);
        return -1;
    }

    int64_t private_file_size = GetFileSize(src_private_file_name_);

    // 开始循序读取数据块，并写入文件
    shared_ptr<FILE> private_fp_ptr(fopen(src_private_file_name_.c_str(), "rb"), 
        [](FILE* fp) {
        if (fp) 
            fclose(fp); 
    });
    vector<shared_ptr<FILE>> public_fp_vec;
    for (auto&& itor : src_public_file_name_list_) {
        public_fp_vec.emplace_back(
            fopen(itor.c_str(), "rb"), [](FILE* fp) {
            if (fp)
                fclose(fp);
        }
        );
    }

    // 读取并写入私有块的前一半
    int tmp_length(0), read_length(0), read_count(0), ret(0);
    uint8_t* read_buffer = (uint8_t*)malloc(block_size);
    do 
    {
        read_length = fread((void *)&tmp_length, 1, sizeof(int), private_fp_ptr.get());
        if (read_length < sizeof(int)) {
            LOGGER->warn("{} line {} private file end!", __FUNCTION__, __LINE__);
            break;
        }
        read_count += tmp_length;
        if (read_count > private_file_size / 2) {
            ret = fseek(private_fp_ptr.get(), 0 - sizeof(int), SEEK_CUR);
            break;
        }

        read_length = fread(read_buffer, 1, tmp_length, private_fp_ptr.get());
        if (read_length > 0) {
            fwrite(read_buffer, 1, read_length, dest_fp);
        }
    } while (true);

    // 循环读取公有块，并写入
    bool flag = false;
    do 
    {
        flag = false;
        for (auto&& itor : public_fp_vec) {
            read_length = fread((void *)&tmp_length, 1, sizeof(int), itor.get());
            if (read_length < sizeof(int)) {
                LOGGER->warn("{} line {} public file end!", __FUNCTION__, __LINE__);
                break;
            }
            read_length = fread(read_buffer, 1, tmp_length, itor.get());
            if (read_length != tmp_length) {
                LOGGER->warn("{} length exception!", __FUNCTION__);
                break;
            }
            if (read_length > 0) {
                fwrite(read_buffer, 1, read_length, dest_fp);
                flag = true;
            }
        }
        if (!flag) {
            break;
        }
    } while (true);

    // 继续写入私有块的后半块
    do {
        read_length = fread((void *)&tmp_length, 1, sizeof(int), private_fp_ptr.get());
        if (read_length < sizeof(int)) {
            LOGGER->warn("{} line {} private file end!", __FUNCTION__, __LINE__);
            break;
        }
        read_count += tmp_length;
        read_length = fread(read_buffer, 1, tmp_length, private_fp_ptr.get());
        if (read_length > 0) {
            fwrite(read_buffer, 1, read_length, dest_fp);
        }
    } while (true);

    fclose(dest_fp);
    free(read_buffer);
}

int IBaseProcess::CloseFile()
{
    LOGGER->info("{} ", __FUNCTION__);
    return 1;
}

int IBaseProcess::GenerateChildFileName(
    const string& src_path_, const string& src_file_name_, 
    const string& dest_path_, int slice_count_, 
    __out string& private_child_file_name, __out list<string>& publish_child_file_name_list_)
{
    if (src_file_name_.empty() || slice_count_ <= 0) {
        LOGGER->warn("{} params error! file name:{}, slice_count:{}", 
            __FUNCTION__, src_file_name_, slice_count_);
        return -1;
    }
    char buffer[MAX_PATH] = { 0 };
    int len = snprintf(buffer, MAX_PATH, "%s/%s_private", dest_path_.c_str(), src_file_name_.c_str());
    private_child_file_name.clear();
    private_child_file_name.append(buffer, len);
    for (int i(1); i <= slice_count_; ++i) {
        memset(buffer, 0, MAX_PATH);
        len = snprintf(buffer, MAX_PATH, "%s/%s_public_%d", 
            dest_path_.c_str(), src_file_name_.c_str(), i);
        publish_child_file_name_list_.emplace_back(string(buffer, len));
    }
    return 0;
}
