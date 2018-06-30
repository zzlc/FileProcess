#include "IBaseProcess.h"
#include <io.h>
#include <direct.h>

IBaseProcess::IBaseProcess()
{
}

IBaseProcess::~IBaseProcess()
{
    if (_ffmpeg_demux_ptr) {
        demuxer_close_file(_ffmpeg_demux_ptr);
        _ffmpeg_demux_ptr = NULL;
    }
}

int IBaseProcess::SliceFile(const string& src_path_, const string& src_file_name_,
    const string& dest_path_, int slice_count_, __out string& private_child_file_name,
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
    if (_ffmpeg_demux_ptr) {
        demuxer_close_file(_ffmpeg_demux_ptr);
        _ffmpeg_demux_ptr = nullptr;
    }
    _ffmpeg_demux_ptr = demuxer_open_file(absolute_path);
    if (!_ffmpeg_demux_ptr) {
        LOGGER->error("{} demuxer_open_file failed!", __FUNCTION__);
        return -1;
    }
    _file_name = src_file_name_;

    if (GenerateChildFileName(src_path_, src_file_name_, 
        dest_path_, slice_count_, 
        private_child_file_name, publish_child_file_name_list_) < 0) {
        LOGGER->error("{} Generate child file name failed!", __FUNCTION__);
        return -1;
    }
    // 私有块
    shared_ptr<SliceFileInfo> private_slice_ptr =
        make_shared<SliceFileInfo>((char *)private_child_file_name.c_str(), 5 * 1024 * 1024);
    // 公有块
    vector<shared_ptr<SliceFileInfo>> public_slice_vec;
    for (auto&& itor : publish_child_file_name_list_) {
        public_slice_vec.push_back(
            make_shared<SliceFileInfo>((char *)itor.c_str(), 5 * 1024 * 1024)
        );
    }

    demuxer_avpacket_t* av_pkt_ptr = nullptr;

    // 首先读取第一帧数据，然后获取 metadata
    av_pkt_ptr = demuxer_read_next_packet(_ffmpeg_demux_ptr);
    if (!av_pkt_ptr) {
        LOGGER->warn("{} none av packet!", __FUNCTION__);
        return -1;
    }
    int key_frame_encrypt_header_len = 1024; // 关键帧加密头部长度

    // 获取文件头部数据，并写入私有块
    FILE* tmp_fp = fopen(absolute_path, "rb");
    if (!tmp_fp) {
        LOGGER->error("{} {} Open file to get metadata failed!", __FUNCTION__, __LINE__);
        return -1;
    }

    string tmp_src_data;
    tmp_src_data.resize(1024 * 1024);
    int tmp_len(0), ret(0);
    uint64_t tmp_byte_count(0);
    string tmp_header((char *)av_pkt_ptr->data, 128);
    while ((tmp_len = fread((char *)tmp_src_data.data(), 1, 1024 * 1024, tmp_fp)) > 0)
    {
        // 只比较前 128 个字节即可
        ret = tmp_src_data.find(string((char *)av_pkt_ptr->data, 128));
        if (ret != tmp_src_data.npos) {
            tmp_byte_count += ret;
            private_slice_ptr->SetData((uint8_t *)tmp_src_data.data(), ret);
            break;
        }
        tmp_byte_count += tmp_len;
        private_slice_ptr->SetData((uint8_t *)tmp_src_data.data(), tmp_len);
    }
    // 文件头查找 & 写入完毕，关闭临时文件 tmp_fp
    fclose(tmp_fp);

    // 继续写入数据
    int public_index(0), current_size(0);
    const int max_buffer_size = 5 * 1024 * 1024; // 最大一次写入 5MB
    uint8_t* buffer = (uint8_t *)malloc(max_buffer_size);
    while ((av_pkt_ptr = demuxer_read_next_packet(_ffmpeg_demux_ptr)) != NULL) {
        if (av_pkt_ptr->stream_index == _ffmpeg_demux_ptr->video_stream_index
            && 1 == av_pkt_ptr->flags) {
            // 已缓冲旧数据先写入 publish 块
            public_slice_vec[public_index++]->SetData(buffer, current_size);
            current_size = 0;

            // 几个公有块轮流去写
            if (public_index >= public_slice_vec.size()) {
                public_index = 0;
            }

            // 关键帧写入私有数据块
            private_slice_ptr->SetData(av_pkt_ptr->data, av_pkt_ptr->limit_size);
        } else {
            if (current_size + av_pkt_ptr->limit_size > max_buffer_size) {
                public_slice_vec[public_index++]->SetData(buffer, current_size);
                current_size = 0;
            }
            memcpy(buffer + current_size, av_pkt_ptr->data, av_pkt_ptr->limit_size);
            current_size += av_pkt_ptr->limit_size;

            // 几个公有块轮流去写
            if (public_index >= public_slice_vec.size()) {
                public_index = 0;
            }
        }
        tmp_byte_count += av_pkt_ptr->limit_size;
    }

    demuxer_close_file(_ffmpeg_demux_ptr);
    _ffmpeg_demux_ptr = NULL;

    if (buffer) {
        free(buffer);
    }

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
    char tmp_name_buf[MAX_PATH] = { 0 };
    int file_index(0);
    int ret = sscanf(src_public_file_name_list_.front().c_str(), "%s_public_%d", tmp_name_buf, &file_index);
    if (ret <= 0) {
        LOGGER->error("{} format file name failed!", __FUNCTION__);
        return -1;
    }
    string dest_name = dest_path_ + tmp_name_buf;
    LOGGER->info("{} Gererate dest file name:{} success.", __FUNCTION__, dest_name);
    FILE* dest_fp = fopen(dest_name.c_str(), "wb");
    if (!dest_fp) {
        LOGGER->error("{} open dest file:{} to write failed!", __FUNCTION__, dest_name);
        return -1;
    }
    // 开始循序读取数据块，并写入文件
    shared_ptr<FILE> private_fp_ptr(fopen(src_private_file_name_.c_str(), "rb"), 
        [](FILE* fp) {
        if (fp) 
            fclose(fp); 
    });
    vector<shared_ptr<FILE>> public_fp_vec;
    for (auto&& itor : src_public_file_name_list_) {
        public_fp_vec.emplace_back(
            fopen(src_private_file_name_.c_str(), "rb"), [](FILE* fp) {
            if (fp)
                fclose(fp);
        }
        );
    }
    int tmp_length(0), read_length(0);
    uint8_t* read_buffer = (uint8_t*)malloc(5 * 1024 * 1024);
    do 
    {
        read_length = fread((void *)&tmp_length, 1, sizeof(int), private_fp_ptr.get());
        if (read_length < sizeof(int)) {
            LOGGER->warn("{} line {} private file end!", __FUNCTION__, __LINE__);
            break;
        }
        read_length = fread(read_buffer, 1, tmp_length, private_fp_ptr.get());
    } while (true);
}

int IBaseProcess::CloseFile()
{
    LOGGER->info("{} ", __FUNCTION__);
    if (_ffmpeg_demux_ptr) {
        demuxer_close_file(_ffmpeg_demux_ptr);
        _ffmpeg_demux_ptr = nullptr;
        return 0;
    }
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
