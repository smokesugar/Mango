#pragma once

#include "Mango/Core/ECS.h"
#include "Mango/Renderer/Texture.h"
#include "Mango/Renderer/Framebuffer.h"

namespace Mango {

	class Scene {
		friend class Entity;
	public:
		Scene();
		~Scene();

		class Entity Create(const std::string& name = "Unnamed Entity");

		void OnUpdate(float dt, const Ref<Framebuffer>& rendertarget);
		
		inline ECS::Registry& GetRegistry() { return mRegistry; }
		inline TextureLibrary& GetTextureLibrary() { return mTextureLibrary; }

		ECS::Registry mRegistry;

		class TextureLibrary mTextureLibrary;
	};

}