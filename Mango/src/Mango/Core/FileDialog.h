#pragma once

#include <string>

namespace Mango {
	
	namespace FileDialog {
		bool Open(std::string& path, const wchar_t* filter = L"All\0 * .*\0");
		bool Save(std::string& path, const wchar_t* filter = L"All\0 * .*\0");
	}

}