#pragma once
/**
 * @brief 日式系统：用于输出日志
 * 
 */

#include <memory>
#include <mutex>
#include <list>
#include <iostream> // for std::cout
#include <fstream>	// for ofstream
#include <sstream>	// for stringstream ostringstream
#include <ctime>	// for struct tm, time()
#include <thread>
#include <vector>
#include <condition_variable>


#include "Singleton.h"
#include "Buffer.h"

#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
#endif

//uint64_t getCurMicroSecond() { return 0; }	// TODO



namespace Toy
{

class LogLevel
{
public:
	enum Level
	{
		UNKNOW = 0,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL,
		LEVEL_NUM
	};
	static const char * getCStr(LogLevel::Level level);
	const std::string getString(LogLevel::Level level);
private:
};

class Logger;

/**
 * @brief 日志事件
 * 每一条日志都作为一个日志事件来处理
 */
class LogEvent
{
public:
	typedef std::shared_ptr<LogEvent> Ptr;
	typedef std::stringstream MsgStream;
	typedef time_t TimeType;
	LogEvent(
		const char * file, 
		const int32_t line, 
		const uint64_t thread_id, 
		const std::string & content,
		const uint64_t elapse_time,
		const uint64_t timestamp,
		const LogLevel::Level level
		);
	~LogEvent();
	const char * getFile() const { return m_file; }
	int32_t getLine() const { return m_line; }
	uint64_t getThreadID() const { return m_thread_id; }
	std::string getContent() const { return m_content; }
	uint64_t getElapseTime() const { return m_elapse_time; }
	TimeType getTimestamp() const { return m_timestamp; }
	LogLevel::Level getLoglevel() const { return m_level; }
	const char * getCStrLoglevel() const { return LogLevel::getCStr(m_level); }

	const char * getCStrTime() const { return ctime(&m_timestamp); }

	MsgStream & getMsgStream() { return m_msg_stream; }

	std::string getStrThreadID() {
		std::stringstream tempstr;
		tempstr << std::this_thread::get_id();
		return tempstr.str();
	}

private:
	///	文件名
	const char * m_file = nullptr;
	/// 行号
	int32_t m_line = 0;
	/// thread id
	uint64_t m_thread_id = 0;
	///	log content
	std::string m_content;
	/// elapse time from the start of program to now
	uint64_t m_elapse_time;
	/// timestamp
	TimeType m_timestamp;
	/// log level
	LogLevel::Level m_level;
	/// logger pointer
	//std::shared_ptr<Logger> m_logger_ptr;
	/// stream of log msg
	MsgStream m_msg_stream;

};

class LogFormat
{
public:
	enum FormatPattern
	{
		DATA_TIME = 1,
		TOY_LOG_LEVEL = (1 << 2),
		THREAD_ID = (1 << 3),
		FILENAME = (1 << 4),
		LINE = (1 << 5),
		MSG = (1 << 6),
		ELAPSE_TIME = (1 << 7),
		ALL_ITEM = DATA_TIME | FILENAME | THREAD_ID | TOY_LOG_LEVEL | LINE | MSG | ELAPSE_TIME
	};
	typedef std::shared_ptr<LogFormat> Ptr;
	LogFormat(const FormatPattern &f_pattern = ALL_ITEM);
	void init();
	//Toy::LogFormat::generateFormatLog consumes a lot of CPU time 
	// especially the Time format and stream
	// TODO : improve generateFormatLog
	// Maybe generate format logs in another background threads.
	// or generate time once per second
	// or 
	std::string generateFormatLog(LogEvent::Ptr);
private:
	char * getFormatTime(LogEvent::TimeType raw_time) 
	{ 
		m_info = localtime(&raw_time);
		strftime(m_time_buf, sizeof(m_time_buf), m_time_format, m_info);
		return m_time_buf;
	}
	FormatPattern m_pattern;
	char m_time_buf[64];
	const char *m_time_format = "%Y-%m-%d %H:%M:%S";
	struct tm *m_info;
};

class LogStream
{
public:
	static constexpr size_t BUFFER_SIZE = 64 * 1024;
	static constexpr size_t WRITE_BUF_SIZE = 128 * 1024;
	typedef FixBuffer<BUFFER_SIZE> Buffer;
	typedef std::shared_ptr<Buffer> BufferPtr;
	LogStream(const std::string & file_name);
	~LogStream();
	
	void append(const char *, size_t len);
	LogStream & operator<<(std::stringstream &);
	LogStream & operator<<(std::string &);
	//LogStream & operator<<(const char *);
private:
	void threadFun();
	void flush();
	BufferPtr m_cur_buffer;
	BufferPtr m_next_buffer;

	std::condition_variable m_cv;
	std::condition_variable m_cv_next_ready;
	std::mutex m_mutex;

	FILE * m_file;
	char * m_write_buf;

	std::atomic<bool> m_is_running;
	//int m_wait_seconds;

	std::thread m_thread;
};

class LogAppender
{
public:
	typedef std::shared_ptr<LogAppender> Ptr;
	virtual ~LogAppender() {}
	/// to output a log
	virtual void log(LogLevel::Level, LogEvent::Ptr) = 0;
	void setLogFormat(LogFormat::Ptr f_ptr) { m_format_ptr = f_ptr; }
	LogFormat::Ptr getLogFormat() const { return m_format_ptr; }
	void setLevel(LogLevel::Level level) { m_level = level; }
protected:
#ifdef NDEBUG
	LogLevel::Level m_level = LogLevel::Level::INFO;
#else
	LogLevel::Level m_level = LogLevel::Level::DEBUG;
#endif
	LogFormat::Ptr m_format_ptr = nullptr;	// it should be init, otherwise it would be empty(Windbg status)
											// (could not use, but not pointer to nullptr, look like it works)
	std::mutex m_mutex_stream;
	
};

class StdoutAppender : public LogAppender
{
public:
	typedef std::shared_ptr<StdoutAppender> Ptr;
	void log(LogLevel::Level, LogEvent::Ptr) override;
private:
};

class FileAppender : public LogAppender
{
public:
	typedef std::shared_ptr<FileAppender> Ptr;
	typedef LogStream FileOutStream;	
	FileAppender(const std::string & file_path = "log.txt");
	void log(LogLevel::Level, LogEvent::Ptr) override;
	void setFile(const std::string & file_path) { m_file_path = file_path; }
	//  /**
	//   * @brief reopen a file
	//   * 	if it is open, close it and then open again.
	//   * 	if not, just open it
	//   * @return true 
	//   * 	reopen success
	//   * @return false 
	//   * 	reopen fail
	//   */
	//  bool reopen();
private:
	/// where to store the logs
	std::string m_file_path;
	FileOutStream m_file_stream;
};

class Logger
{
public:
	typedef std::shared_ptr<Logger> Ptr;
#ifdef NDEBUG
	Logger(const std::string & name = "root", LogLevel::Level level = LogLevel::Level::INFO);
#else
	Logger(const std::string & name = "root", LogLevel::Level level = LogLevel::Level::DEBUG);
#endif
	
	~Logger();
	void log(LogLevel::Level, LogEvent::Ptr);
	void logDebug(LogEvent::Ptr);
	void logInfo(LogEvent::Ptr);
	void logWarn(LogEvent::Ptr);
	void logError(LogEvent::Ptr);
	void logFatal(LogEvent::Ptr);
	void addAppender(LogAppender::Ptr);
	void rmAppender(LogAppender::Ptr);
	void setLevel(const LogLevel::Level & level) { m_level = level; } 
	LogLevel::Level getLevel() const { return m_level; } 
	void setLoggerName(const std::string & name) { m_name = name; }
	std::string getLoggerName() const { return m_name; }
private:
	/// all LogAppenders
	std::list<LogAppender::Ptr> m_all_appenders;
	/// name of logger
	std::string m_name;
	/// level of Logger itself : it can log those (level >= m_level)
	LogLevel::Level m_level;
	/// protect list of appenders
	std::mutex m_mutex_appenders;
	/// 
	LogFormat::Ptr m_format_ptr;

};

class LogEventWrap
{
public:
	//std::shared_ptr<Logger>
	LogEventWrap(Logger::Ptr, LogEvent::Ptr);
	~LogEventWrap();
	LogEvent::MsgStream & getStream();
private:
	Logger::Ptr m_logger_ptr;
	LogEvent::Ptr m_event_ptr;
};

class LogManager
{
public:
	LogManager(const std::string & config_file_name = "log.config.json");
	~LogManager() = default;
	Logger::Ptr getLogger() { return m_logger_ptr; }
private:
	Logger::Ptr m_logger_ptr;
	//LogAppender::Ptr m_stdappender_ptr;
	//LogAppender::Ptr m_fileappender_ptr;
};

}// namespace Toy

// Log Macro start:--------------------------------------|
#define TOY_LOG(logger_ptr,level) \
	if(level >= logger_ptr->getLevel())	\
		Toy::LogEventWrap(logger_ptr, std::make_shared<Toy::LogEvent>(__FILE__, __LINE__, \
			0, "", 0, time(nullptr), level)).getStream() \
// FIXME:what if level < logger_ptr->getLevel()
#define _TOY_LOG_DEBUG(logger_ptr) TOY_LOG(logger_ptr, Toy::LogLevel::DEBUG)
#define _TOY_LOG_INFO(logger_ptr) TOY_LOG(logger_ptr, Toy::LogLevel::INFO)
#define _TOY_LOG_WARN(logger_ptr) TOY_LOG(logger_ptr, Toy::LogLevel::WARN)
#define _TOY_LOG_ERROR(logger_ptr) TOY_LOG(logger_ptr, Toy::LogLevel::ERROR)
#define _TOY_LOG_FATAL(logger_ptr) TOY_LOG(logger_ptr, Toy::LogLevel::FATAL)

#define	TOY_GLOBAL_LOG_MAMAGER Toy::Singleton<Toy::LogManager>::getInstance()
#define	TOY_GLOBAL_LOGGER TOY_GLOBAL_LOG_MAMAGER.getLogger()
#define TOY_LOG_DEBUG _TOY_LOG_DEBUG(TOY_GLOBAL_LOGGER)
#define TOY_LOG_INFO _TOY_LOG_INFO(TOY_GLOBAL_LOGGER)
#define TOY_LOG_WARN _TOY_LOG_WARN(TOY_GLOBAL_LOGGER)
#define TOY_LOG_ERROR _TOY_LOG_ERROR(TOY_GLOBAL_LOGGER)
#define TOY_LOG_FATAL _TOY_LOG_FATAL(TOY_GLOBAL_LOGGER)
// Log Macro end:----------------------------------------|





