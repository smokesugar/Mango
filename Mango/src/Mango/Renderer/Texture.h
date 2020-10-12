#pragma once

#include <cinttypes>
#include <string>

namespace Mango {

	class Texture {
	public:
		virtual ~Texture() {}

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void Bind(size_t slot) const = 0;
	};

	class Texture2D : public Texture {
	public:
		virtual ~Texture2D() {}

		static Texture2D* Create(const std::string& filePath);
		static Texture2D* Create(void* data, uint32_t width, uint32_t height);
	};

	class SamplerState {
	public:
		enum class Mode {
			Linear,
			Point
		};
	public:
		virtual ~SamplerState() {}

		virtual void Bind(size_t slot) const = 0;

		static SamplerState* Create(Mode mode = Mode::Linear);
	};

}