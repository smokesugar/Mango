#pragma once

#include <string>
#include <system_error>

#define NOMINMAX
#include <Windows.h>

namespace Mango {
	static std::string Translate(HRESULT hr) {
		return std::system_category().message(hr);
	}

	static std::wstring WidenString(const std::string& as)
	{
		if (as.empty())
			return std::wstring();
		size_t reqLength = ::MultiByteToWideChar(CP_UTF8, 0, as.c_str(), (int)as.length(), 0, 0);
		std::wstring ret(reqLength, L'\0');
		::MultiByteToWideChar(CP_UTF8, 0, as.c_str(), (int)as.length(), &ret[0], (int)ret.length());
		return ret;
	}
}
