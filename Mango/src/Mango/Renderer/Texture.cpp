#include "mgpch.h"
#include "Texture.h"

namespace Mango {

	void TextureLibrary::Load(const std::string& name)
	{
		MG_CORE_ASSERT(!IsLoaded(name), "Texture '{0}' is already loaded.");
		mTextures[name] = Ref<Texture2D>(Texture2D::Create(name));
	}

	const Ref<Texture2D>& TextureLibrary::Get(const std::string& name)
	{
		if (!IsLoaded(name))
			Load(name);

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