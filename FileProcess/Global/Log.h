#pragma once
#include "3rd/spdlog/spdlog.h"

#ifdef _GATEWAY_DEVELOP
#include "Global.h"
#endif // _GATEWAY_DEVELOP

using namespace std;
using namespace spdlog;

// class log
class CSpdLog
{
public:
    static CSpdLog* Instance()
    {
        if (!_spd_logger_ptr) {
            _spd_logger_ptr = new CSpdLog;
        }
        return _spd_logger_ptr;
    }

    void SetLogFileName(const string& file_name_)
    {
        if (!file_name_.empty()) {
            _file_name = file_name_;
        }
    }

    std::shared_ptr<spdlog::logger> GetLogger()
    {
        return _file_logger_ptr;
    }

    void InitLog()
    {
        //init log
        spdlog::set_async_mode(4096);
    #ifdef DEBUG
        spdlog::set_level(spdlog::level::debug);
    #else
        spdlog::set_level(spdlog::level::info);
    #endif //endif DEBUG
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][%t][%l] %v");
        auto console_log_ptr = spdlog::stdout_logger_mt("console");
        auto file_log_ptr = spdlog::rotating_logger_mt("", _file_name, 100 * 1024 * 1024, 2);
        spdlog::sinks_init_list sink_list = { console_log_ptr->sinks().front(), file_log_ptr->sinks().front() };
        _file_logger_ptr = spdlog::create("loggername", sink_list);

        _file_logger_ptr->info("Version Build Time:{} {}", __DATE__, __TIME__);
        _file_logger_ptr->flush();
    }

protected:
    CSpdLog()
    {
        InitLog();
    };
    CSpdLog(const CSpdLog& ) {};
    CSpdLog operator = (const CSpdLog& obj_){}   
    ~CSpdLog() 
    {
        if (_file_logger_ptr) {
            _file_logger_ptr->flush();
        }
    };

private:
    std::shared_ptr<spdlog::logger> _file_logger_ptr;
    spdlog::level::level_enum       _log_level = spdlog::level::info;
    string                          _file_name = "fileprocess.log";
protected:
    static  CSpdLog*                _spd_logger_ptr;
};

#define LOGGER CSpdLog::Instance()->GetLogger()