#pragma once

#include <cinttypes>
#include <string>

#include "Mango/Core/Base.h"
#include "Formats.h"

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

		static Texture2D* Create(const std::string& filePath, Format format, bool mipMap = true);
		static Texture2D* Create(void* data, uint32_t width, uint32_t height);
	};

	class Cubemap : public Texture {
	public:
		virtual ~Cubemap() {}

		virtual void BindAsRenderTarget(size_t mip = 0) const = 0;
		virtual const std::string& GetPath() const = 0;

		virtual void GenerateMips() = 0;
		virtual uint32_t GetMipLevels() const = 0;

		static Cubemap* Create(const std::string& filePath, uint32_t size);
	};

	class SamplerState {
	public:
		enum class Filter {
			Linear,
			Point
		};
		enum class Address {
			Clamp,
			Wrap,
			Border
		};
	public:
		virtual ~SamplerState() {}

		virtual void Bind(size_t slot) const = 0;

		static SamplerState* Create(Filter mode = Filter::Linear, Address address = Address::Wrap, bool comparison = false);
	};

	class TextureLibrary {
	public:
		TextureLibrary() = default;
		void Load(const std::string& name, Format format);
		const Ref<Texture2D>& Get(const std::string& name, Format format);
		bool IsLoaded(const std::string& name);
		void ClearUnused();
	private:
		std::unordered_map<std::string, Ref<Texture2D>> mTextures;
	};

}