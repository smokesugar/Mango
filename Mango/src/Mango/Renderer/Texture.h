#pragma once

#include <cinttypes>
#include <string>

#include "Mango/Core/Base.h"
#include "Mango/Core/Math.h"
#include "Formats.h"

namespace Mango {

	using TextureFlags = int;
	enum _TextureFlags {
		Texture_Trilinear = BIT(0),
		Texture_RenderTarget = BIT(1),
		Texture_CPU = BIT(2)
	};

	class Texture {
	public:
		virtual ~Texture() {}

		virtual void Bind(size_t slot) const = 0;

		virtual const std::string& GetPath() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual Format GetFormat() const = 0;
		virtual void* GetNativeTexture() const = 0;

		virtual void EnsureSize(uint32_t width, uint32_t height) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void GetData(uint32_t x, uint32_t y, const Ref<Texture>& stagingTexture, void* buffer, size_t size) = 0;

		virtual void Clear(float4 color) = 0;

		static Texture* Create(const std::string& filePath, Format format, TextureFlags flags);
		static Texture* Create(void* data, uint32_t width, uint32_t height, Format format, TextureFlags flags);

		static void Unbind(size_t slot);
	};

	void BlitTexture(const Ref<Texture>& dest, const Ref<Texture>& src);

	class Cubemap {
	public:
		virtual ~Cubemap() {}

		virtual void BindAsShaderResource(size_t slot) const = 0;
		virtual void BindAsRenderTarget(size_t mip = 0) const = 0;

		virtual const std::string& GetPath() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

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
		void Load(const std::string& name, Format format, TextureFlags flags);
		const Ref<Texture>& Get(const std::string& name, Format format, TextureFlags flags);
		bool IsLoaded(const std::string& name);
		void ClearUnused();
	private:
		std::unordered_map<std::string, Ref<Texture>> mTextures;
	};

}