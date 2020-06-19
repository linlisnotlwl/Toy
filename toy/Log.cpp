#include "Log.h"
#include <string> // for std::to_string
#include <chrono>
#include <functional>

#include<Config.h>

namespace Toy
{

// xx implementation start:
// xx implementation end.-------------------------------------------<xx>

static int count = 0;

// LogLevel implementation start:
const char * LogLevel::getCStr(LogLevel::Level level)
{
	switch (level)
	{
	case Level::DEBUG:
	{
		return "DEBUG";
	}
	case Level::INFO:
	{
		return "INFO";
	}
	case Level::WARN:
	{
		return "WARN";
	}
	case Level::ERROR:
	{
		return "ERROR";
	}
	case Level::FATAL:
	{
		return "FATAL";
	}
	case Level::UNKNOW:
	{
		return "UNKNOW";
	}
	default:
		return "The input level is wrong.";
	}
}

const std::string LogLevel::getString(LogLevel::Level level)
{
	return std::string(getCStr(level));
}
// LogLevel implementation end.-------------------------------------------<LogLevel>

// LogEvent implementation start:

LogEvent::LogEvent(
	const char * file, 
	const int32_t line, 
	const uint64_t thread_id, 
	const std::string & content, 
	const uint64_t elapse_time, 
	const uint64_t timestamp, 
	const LogLevel::Level level
	) : 
	m_file(file), m_line(line), m_thread_id(thread_id), m_content(content),
	m_elapse_time(elapse_time), m_timestamp(timestamp), m_level(level)
{
	m_msg_stream << m_content;
}
LogEvent::~LogEvent()
{
}
// LogEvent implementation end.-------------------------------------------<LogEvent>

// LogEventWrap implementation start:
LogEventWrap::LogEventWrap(Logger::Ptr logger_ptr, LogEvent::Ptr event_ptr)
	: m_logger_ptr(logger_ptr), m_event_ptr(event_ptr)
{
}
LogEventWrap::~LogEventWrap()
{
	m_logger_ptr->log(m_event_ptr->getLoglevel(), m_event_ptr);
}
LogEvent::MsgStream & LogEventWrap::getStream()
{
	return m_event_ptr->getMsgStream();
}

// LogEventWrap implementation end.-------------------------------------------<LogEventWrap>

// LogFormat implementation start:
LogFormat::LogFormat(const FormatPattern &f_pattern) : m_pattern(f_pattern)
{

}

void LogFormat::init()
{

}

std::string LogFormat::generateFormatLog(LogEvent::Ptr event_ptr)
{
	// Simplify the format: give it a fixed order
	// Let the users to deal with the logs file.
	std::string temp;
#define XX(PATTERN,STR) \
	if(m_pattern & PATTERN) \
	{	\
		temp.append(STR);	\
		/*temp.append("\t");*/	\
	}
	// it is too irregular, should be improved
	XX(DATA_TIME, getFormatTime(event_ptr->getTimestamp()));
	temp.append("\t");

	//XX(ELAPSE_TIME, std::to_string(event_ptr->getTimestamp()));	// TODO : more accurate time!!!

	XX(TOY_LOG_LEVEL, event_ptr->getCStrLoglevel());
	temp.append("\t");

	XX(THREAD_ID, event_ptr->getStrThreadID());
	//std::stringstream tempstr;
	//tempstr << std::this_thread::get_id();
	//XX(THREAD_ID, tempstr.str());
	temp.append("\t");

	XX(FILENAME, event_ptr->getFile());
	temp.append(":");
	XX(LINE, std::to_string(event_ptr->getLine()));
	temp.append("\t");

	//XX(MSG, event_ptr->getContent());
	XX(MSG, event_ptr->getMsgStream().str());
#undef XX
	//...to do
	temp.append("\r\n");
	return temp;
}
// LogFormat implementation end.-------------------------------------------<LogFormat>

// LogStream implementation start:
LogStream::LogStream(const std::string & file_name) 
	: m_cur_buffer(new Buffer()), m_next_buffer(new Buffer())
{
	m_file = fopen(file_name.c_str(), "a+");
	m_write_buf = new char[WRITE_BUF_SIZE];
	if(m_write_buf == nullptr)
	{
		fprintf(stderr, "new write buffer error\n");
	}
	else
		setbuf(m_file, m_write_buf);

	m_is_running = true;
	m_thread = std::thread(std::mem_fn(&LogStream::threadFun), this);
}

LogStream::~LogStream()
{
	m_is_running = false;
	if (m_thread.joinable())
		m_thread.join();
	flush();
	fclose(m_file);
	delete[] m_write_buf;
	printf("count = %d\n", count);
}

void LogStream::append(const char * data, size_t len)
{
	assert(len < BUFFER_SIZE);
	if(!m_is_running)
		return;
	std::unique_lock<std::mutex> lock(m_mutex);
	if(len > m_cur_buffer->getFreeSize())
	{	
		swap(m_cur_buffer, m_next_buffer);
		if(!m_cur_buffer->empty())	// there are data that has not been write 
		{
			fwrite_unlocked(m_cur_buffer->getData(), sizeof(char), m_cur_buffer->getDataSize(), m_file);
			m_cur_buffer->clear();
			count++;
		}
		m_cur_buffer->append(data, len);
		m_cv_next_ready.notify_one();
	}
	else
	{
		m_cur_buffer->append(data, len);
	}
}

void LogStream::flush()
{
	//std::unique_lock<std::mutex> lock(m_mutex);
	fwrite_unlocked(m_next_buffer->getData(), sizeof(char), m_next_buffer->getDataSize(), m_file);
	m_next_buffer->clear();
	fwrite_unlocked(m_cur_buffer->getData(), sizeof(char), m_cur_buffer->getDataSize(), m_file);
	m_next_buffer->clear();
}

LogStream & LogStream::operator<<(std::stringstream & ss)
{
	std::string temp = ss.str();
	append(temp.c_str(), temp.length());
	return *this;
}

LogStream & LogStream::operator<<(std::string & str)
{
	append(str.c_str(), str.length());
	return *this;
}



void LogStream::threadFun()
{
	//using namespace std::chrono_literals; // c++14
	std::chrono::seconds wait_time(m_wait_seconds);
	//m_cur_buffer.reset(new Buffer());
	//m_next_buffer.reset(new Buffer());

	while(m_is_running)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if(m_next_buffer->empty())
			m_cv.wait_for(lock, wait_time, [&](){ return !m_next_buffer->empty(); });
		if(m_next_buffer->empty())
			swap(m_next_buffer, m_cur_buffer);
		fwrite_unlocked(m_next_buffer->getData(), sizeof(char), m_next_buffer->getDataSize(), m_file);
		m_next_buffer->clear();
	}	
	//m_cv.notify();
}
// LogStream implementation end.-------------------------------------------<LogStream>

// StdoutAppender implementation start:

void StdoutAppender::log(LogLevel::Level level, LogEvent::Ptr event_ptr)
{
	if (level >= m_level)
	{
		std::unique_lock<std::mutex> lock(m_mutex_stream);
		std::cout << m_format_ptr->generateFormatLog(event_ptr);
		//m_format_ptr->generateFormatLog(event_ptr);	//test the cost without cout
	}
}
// StdoutAppender implementation end.-------------------------------------------<StdoutAppender>

// FileAppender implementation start:
FileAppender::FileAppender(const std::string &file_path) 
	: m_file_path(file_path), m_file_stream(file_path)
{
	
}
void FileAppender::log(LogLevel::Level level, LogEvent::Ptr event_ptr)
{
	if (level >= m_level)
	{
		//std::unique_lock<std::mutex> lock(m_mutex_stream);
		std::string temp = m_format_ptr->generateFormatLog(event_ptr);
		m_file_stream << temp;
	}
}
// bool FileAppender::reopen()
// {
// 	std::unique_lock<std::mutex> lock(m_mutex_stream);
// 	if (m_file_stream.is_open())
// 	{
// 		m_file_stream.close();
// 	}
// 	m_file_stream.open(m_file_path);
// 	return m_file_stream.is_open();	
// }
// FileAppender implementation end.-------------------------------------------<FileAppender>

// Logger implementation start:
Logger::Logger(const std::string & name, LogLevel::Level level) : m_name(name), m_level(level)
{
	m_format_ptr.reset(new LogFormat()); // default format : ALL_PATTERN
}


Logger::~Logger()
{
}

void Logger::log(LogLevel::Level level, LogEvent::Ptr event_ptr)
{
	if (level >= m_level)
	{
		//std::unique_lock<std::mutex> lock(m_mutex_appenders); 
		// make sure you will not change appenders during logging, if not , use the lock
		for (auto app : m_all_appenders)
			app->log(level, event_ptr);
	}
}

void Logger::logDebug(LogEvent::Ptr event_ptr)
{
	log(LogLevel::Level::DEBUG, event_ptr);
}

void Logger::logInfo(LogEvent::Ptr event_ptr)
{
	log(LogLevel::Level::INFO, event_ptr);
}

void Logger::logWarn(LogEvent::Ptr event_ptr)
{
	log(LogLevel::Level::WARN, event_ptr);
}

void Logger::logError(LogEvent::Ptr event_ptr)
{
	log(LogLevel::Level::ERROR, event_ptr);
}

void Logger::logFatal(LogEvent::Ptr event_ptr)
{
	log(LogLevel::Level::FATAL, event_ptr);
}

void Logger::addAppender(LogAppender::Ptr app_ptr)
{
    std::unique_lock<std::mutex> lock(m_mutex_appenders);
	if (app_ptr->getLogFormat() == nullptr)
	{
		app_ptr->setLogFormat(m_format_ptr);
	}
    m_all_appenders.push_back(std::move(app_ptr));
}

void Logger::rmAppender(LogAppender::Ptr app_ptr)
{
	std::unique_lock<std::mutex> lock(m_mutex_appenders);
	for (auto it = m_all_appenders.begin(); it != m_all_appenders.end(); ++it)
	{
		if (*it == app_ptr)
		{	
			m_all_appenders.erase(it);
			return;
		}
	}
}
// Logger implementation end.-------------------------------------------<Logger>

// LogManager implementation start:
LogManager::LogManager(const std::string & config_file_name) 
	: m_logger_ptr(std::make_shared<Logger>("main_logger"))
{
	if(!Config::loadConfig(config_file_name))
	{
		//m_logger_ptr->addAppender(std::make_shared<Toy::StdoutAppender>());
		m_logger_ptr->addAppender(std::make_shared<Toy::FileAppender>("log.txt"));
	}
	else
	{
		std::string name;
		if(Config::getValOrZero<std::string>("LoggerName", name))
			m_logger_ptr->setLoggerName(name);
		
		double num = 0;
		if(Config::getValOrZero<double>("LogAppenderNum", num))
		{
			int n = static_cast<int>(num);
			for(int i = 0; i < n; ++i)
			{
				std::string temp = "LogAppender[" + std::to_string(i) + "]";
				std::string appender;
				if(Config::getValOrZero<std::string>(temp, appender))
				{
					if(appender == "StdoutAppender")
					{
						std::shared_ptr<LogAppender> ptr = std::make_shared<StdoutAppender>();
						double level = 0;
						if(Config::getValOrZero<double>("StdoutAppender.loglevel", level))
							ptr->setLevel(static_cast<LogLevel::Level>(level));
						m_logger_ptr->addAppender(ptr);
					}
					else if(appender == "FileAppender")
					{
						std::string filename = "log.txt";
						double level = 0;
						Config::getValOrZero<std::string>("FileAppender.outputfilename", filename);
						std::shared_ptr<LogAppender> ptr = std::make_shared<FileAppender>(filename);
						if(Config::getValOrZero<double>("FileAppender.loglevel", level))
							ptr->setLevel(static_cast<LogLevel::Level>(level));
						m_logger_ptr->addAppender(ptr);
					}
				}
			}
		}
	}	
}

// LogManager implementation end.-------------------------------------------<LogManager>

} // namespace Toy


