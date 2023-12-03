#include "Core/Logger.hpp"


#include <filesystem>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace klux
{
	KLUX_API RawPtr<Logger> Logger::s_Instance = nullptr;
	
	void Logger::Init() 
	{
		if (!s_Instance) s_Instance = CreateRawPtr<Logger>(); 
	}
	
	Logger* Logger::Get()
	{
		if (!s_Instance) s_Instance = CreateRawPtr<Logger>(); return s_Instance;
	}

	void Logger::Shutdown()
	{
		delete s_Instance; 
	}		

	Logger::Logger()
	{
#if defined(NDEBUG)
		m_LogLevelFilter = LogLevel::Info | LogLevel::Warning | LogLevel::Error | LogLevel::Fatal;
#else
		m_LogLevelFilter = LogLevel::All;
#endif
	}

	bool Logger::AttachFile(const String& filename, LogLevel level)
	{
		std::filesystem::path path(filename);
		/*if (std::filesystem::exists(path))
		{
			std::filesystem::remove(path);
		}*/

		std::filesystem::create_directories(path.parent_path());

		auto fullpath = path.string();

		for (const auto& file : m_AttachedFiles)
		{
			if (file.first == fullpath)
			{
				return false;
			}
		}

		m_AttachedFiles.emplace_back(fullpath, level);
		LogToFile(fullpath, "--------------- NEW LOG SESSION ---------------------\n\n");
		Log(LogLevel::Info, "Attached file " + fullpath + " to logger.");
		return true;
	}

	Bool Logger::DetachFile(const String& filename)
	{
		const auto fullath = std::filesystem::path(filename).string();
		auto it = std::find_if(m_AttachedFiles.begin(), m_AttachedFiles.end(), [&](const auto& file) { return file.first == fullath; });
		if (it != m_AttachedFiles.end())
		{
			m_AttachedFiles.erase(it);
			return true;
		}
		return false;
	}

	void Logger::Log(LogLevel level, const String& message)
	{
		if (!static_cast<Bool>(level & m_LogLevelFilter)) return;

		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		std::stringstream timeStr;
		timeStr << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");

		const auto logLevelString = GetLogLevelString(level);
		const auto logMessage = "[" + logLevelString + "]" + std::string(8 - logLevelString.size(), ' ') + "[" + timeStr.str() + "]\t: " + message + "\n";

		if (m_EnableConsole)
		{
			LogToConsole(level, logMessage);
		}

		for (const auto& file : m_AttachedFiles)
		{
			if (static_cast<Bool>(level & file.second))
			{
				LogToFile(file.first, logMessage);
			}
		}
	}

	String Logger::GetLogLevelString(LogLevel level) const
	{
		switch (level)
		{
		case klux::LogLevel::Trace:		return "TRACE";
		case klux::LogLevel::Debug:		return "DEBUG";
		case klux::LogLevel::Info:		return "INFO";
		case klux::LogLevel::Warning:	return "WARNING";
		case klux::LogLevel::Error:		return "ERROR";
		case klux::LogLevel::Fatal:		return "FATAL";
		case klux::LogLevel::All:		return "ALL";
		case klux::LogLevel::None:		return "NONE";
		default:						return "UNKNOWN";	
		}
	}

	void Logger::LogToFile(const String& file, const String& message)
	{
		std::ofstream stream(file, std::ios::app);
		stream << message;
		stream.close();
	}

	void Logger::LogToConsole(LogLevel level, const String& message)
	{

#if defined(_WIN32)
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
		WORD saved_attributes = consoleInfo.wAttributes;

		switch (level)
		{
		case klux::LogLevel::Trace:
			SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			break;
		case klux::LogLevel::Debug:
			SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
			break;
		case klux::LogLevel::Info:
			SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
			break;
		case klux::LogLevel::Warning:
			SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
			break;
		case klux::LogLevel::Error:
			SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED);
			break;
		case klux::LogLevel::Fatal:
			SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE);
			break;
		default:
			SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			break;
		}

		std::cout << message;

		SetConsoleTextAttribute(hConsole, saved_attributes);
#else

		switch (level)
		{
		case klux::LogLevel::Trace:
			std::cout << "\033[0;37m" << message << "\033[0m";
			break;
		case klux::LogLevel::Debug:
			std::cout << "\033[0;32m" << message << "\033[0m";
			break;
		case klux::LogLevel::Info:
			std::cout << "\033[0;36m" << message << "\033[0m";
			break;
		case klux::LogLevel::Warning:
			std::cout << "\033[0;33m" << message << "\033[0m";
			break;
		case klux::LogLevel::Error:
			std::cout << "\033[0;31m" << message << "\033[0m";
			break;
		case klux::LogLevel::Fatal:
			std::cout << "\033[0;35m" << message << "\033[0m";
			break;
		default:	
			std::cout << "\033[0;37m" << message << "\033[0m";
			break;
		}
#endif

	}

}