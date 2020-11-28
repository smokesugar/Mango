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
		inline void AddLog(const std::string& str) { mBuffer.append(str); mScroll = true; }
		inline bool HandleScroll() { bool save = mScroll; mScroll = false; return save; }
	private:
		std::string mBuffer;
		bool mScroll = false;
	};

}