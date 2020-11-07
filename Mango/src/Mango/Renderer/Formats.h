#pragma once

#include <dxgi.h>

namespace Mango {

	enum class Format {
		RGBA8_UNORM,
		RGBA8_UNORM_SRGB,
		RGBA16_FLOAT,
		RGBA32_FLOAT,
	};

	static bool IsFormatFloatingPoint(Format format) {
		switch (format) {
		case Format::RGBA8_UNORM:
		case Format::RGBA8_UNORM_SRGB:
			return false;
		case Format::RGBA16_FLOAT:
		case Format::RGBA32_FLOAT:
			return true;
		default:
			return false;
		}
	}
	
	static DXGI_FORMAT DXGIFormatFromMangoFormat(Format format) {
		switch (format) {
		case Format::RGBA8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case Format::RGBA8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case Format::RGBA16_FLOAT:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case Format::RGBA32_FLOAT:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		default:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		}
	}

	static size_t FormatSize(Format format) {
		switch (format) {
		case Format::RGBA8_UNORM:
			return 4;
		case Format::RGBA8_UNORM_SRGB:
			return 4;
		case Format::RGBA16_FLOAT:
			return 8;
		case Format::RGBA32_FLOAT:
			return 16;
		default:
			return 8;
		}
	}

}