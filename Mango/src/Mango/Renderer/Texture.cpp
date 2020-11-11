#include "mgpch.h"
#include "Texture.h"

namespace Mango {

	void TextureLibrary::Load(const std::string& name, Format format, TextureFlags flags)
	{
		MG_CORE_ASSERT(!IsLoaded(name), "Texture '{0}' is already loaded.");
		mTextures[name] = Ref<Texture>(Texture::Create(name, format, flags));
	}

	const Ref<Texture>& TextureLibrary::Get(const std::string& name, Format format, TextureFlags flags)
	{
		if (!IsLoaded(name))
			Load(name, format, flags);

		return mTextures[name];
	}

	bool TextureLibrary::IsLoaded(const std::string& name)
	{
		return mTextures.find(name) != mTextures.end();
	}

	void TextureLibrary::ClearUnused()
	{
		for (auto it = mTextures.begin(); it != mTextures.end(); it++)
			if (it->second.use_count() == 1) mTextures.erase(it);
	}

}