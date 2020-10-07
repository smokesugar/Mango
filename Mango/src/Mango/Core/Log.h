#pragma once

#include <memory>

#include "spdlog/spdlog.h"

namespace Mango {
	
	class Log {
	public:
		static void Init();
		inline static const std::shared_ptr<spdlog::logger>& GetCoreLogger() { return sCoreLogger; }
		inline static const std::shared_ptr<spdlog::logger>& GetClientLogger() { return sClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> sCoreLogger;
		static std::shared_ptr<spdlog::logger> sClientLogger;
	};

}

#ifdef MG_DEBUG
	#define MG_CORE_TRACE(...) ::Mango::Log::GetCoreLogger()->trace(__VA_ARGS__);
	#define MG_CORE_INFO(...)  ::Mango::Log::GetCoreLogger()->info(__VA_ARGS__);
	#define MG_CORE_WARN(...)  ::Mango::Log::GetCoreLogger()->warn(__VA_ARGS__);
	#define MG_CORE_ERROR(...) ::Mango::Log::GetCoreLogger()->error(__VA_ARGS__);
	#define MG_CORE_FATAL(...) ::Mango::Log::GetCoreLogger()->critical(__VA_ARGS__);

	#define MG_TRACE(...) ::Mango::Log::GetClientLogger()->trace(__VA_ARGS__);
	#define MG_INFO(...)  ::Mango::Log::GetClientLogger()->info(__VA_ARGS__);
	#define MG_WARN(...)  ::Mango::Log::GetClientLogger()->warn(__VA_ARGS__);
	#define MG_ERROR(...) ::Mango::Log::GetClientLogger()->error(__VA_ARGS__);
	#define MG_FATAL(...) ::Mango::Log::GetClientLogger()->critical(__VA_ARGS__);
#else
	#define MG_CORE_TRACE(...)
	#define MG_CORE_INFO(...) 
	#define MG_CORE_WARN(...) 
	#define MG_CORE_ERROR(...)
	#define MG_CORE_FATAL(...)

	#define MG_TRACE(...)
	#define MG_INFO(...) 
	#define MG_WARN(...) 
	#define MG_ERROR(...)
	#define MG_FATAL(...)
#endif