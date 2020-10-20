#pragma once

#include "Mango/Core/ECS.h"
#include "Mango/Renderer/Texture.h"

namespace Mango {

	class Scene {
		friend class Entity;
	public:
		Scene();
		~Scene();

		class Entity Create(const std::string& name = "Unnamed Entity");

		void OnUpdate(float dt);
		inline void SetScreenDimensions(uint32_t width, uint32_t height) { mScreenWidth = width; mScreenHeight = height; }

		inline ECS::Registry& GetRegistry() { return mRegistry; }
		inline TextureLibrary& GetTextureLibrary() { return mTextureLibrary; }
	private:
		uint32_t mScreenWidth;
		uint32_t mScreenHeight;
		ECS::Registry mRegistry;

		class TextureLibrary mTextureLibrary;
	};

}