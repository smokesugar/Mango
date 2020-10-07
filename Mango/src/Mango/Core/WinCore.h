#pragma once

#include <string>
#include <system_error>

#define NOMINMAX
#include <Windows.h>

namespace Mango {
	static std::string Translate(HRESULT hr) {
		return std::system_category().message(hr);
	}
}