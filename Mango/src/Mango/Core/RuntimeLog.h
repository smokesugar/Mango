#pragma once

#include <string>
#include <stdarg.h>
#include <iostream>

namespace Mango {
	
	class RuntimeLog {
	public:
		RuntimeLog() = default;
		inline void Clear() { mBuffer.clear(); }
		inline const char* GetBuffer() const { return mBuffer.c_str(); }
		inline void AddLog(const std::string& str) { mBuffer.append(str); }
	private:
		std::string mBuffer;
	};

}