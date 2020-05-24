#include "Log.h"
#include <string> // for std::to_string



namespace Toy
{

// xx implementation start:
// xx implementation end.-------------------------------------------<xx>



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
FileAppender::FileAppender(const std::string &file_path) : m_file_path(file_path)
{
	m_file_stream.open(file_path.c_str(), std::ios_base::out | std::ios_base::ate);
}
void FileAppender::log(LogLevel::Level level, LogEvent::Ptr event_ptr)
{
	if (level >= m_level)
	{
		std::unique_lock<std::mutex> lock(m_mutex_stream);
		if(m_file_stream.is_open())
			m_file_stream << m_format_ptr->generateFormatLog(event_ptr);
	}
}
bool FileAppender::reopen()
{
	std::unique_lock<std::mutex> lock(m_mutex_stream);
	if (m_file_stream.is_open())
	{
		m_file_stream.close();
	}
	m_file_stream.open(m_file_path);
	return m_file_stream.is_open();	
}
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
		std::unique_lock<std::mutex> lock(m_mutex_appenders);
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
LogManager::LogManager() : m_logger_ptr(std::make_shared<Logger>("main_logger")),
	m_stdappender_ptr(std::make_shared<Toy::StdoutAppender>()),
	m_fileappender_ptr(std::make_shared<Toy::FileAppender>("log.txt"))
{
	//m_logger_ptr->addAppender(m_stdappender_ptr);
	m_logger_ptr->addAppender(m_fileappender_ptr);
}
// LogManager implementation end.-------------------------------------------<LogManager>

} // namespace Toy


