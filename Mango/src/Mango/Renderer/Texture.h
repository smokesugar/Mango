#pragma once

#include <cinttypes>
#include <string>

#include "Mango/Core/Base.h"

namespace Mango {

	class Texture {
	public:
		virtual ~Texture() {}

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void Bind(size_t slot) const = 0;
		static void Unbind(size_t slot);

	};

	class Texture2D : public Texture {
	public:
		virtual ~Texture2D() {}

		virtual const std::string& GetPath() const = 0;

		static Texture2D* Create(const std::string& filePath, bool sRGB);
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

	class TextureLibrary {
	public:
		TextureLibrary() = default;
		void Load(const std::string& name, bool sRGB);
		const Ref<Texture2D>& Get(const std::string& name, bool sRGB);
		bool IsLoaded(const std::string& name);
		void ClearUnused();
	private:
		std::unordered_map<std::string, Ref<Texture2D>> mTextures;
	};

}