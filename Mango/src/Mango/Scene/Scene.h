#pragma once

#include "Mango/Core/ECS.h"
#include "Mango/Renderer/Texture.h"
#include "Mango/Renderer/Mesh.h"
#include "Mango/Renderer/RenderTarget.h"

namespace Mango {

	class Scene {
		friend class Entity;
	public:
		Scene();
		~Scene();

		ECS::Entity Create(const std::string& name = "Unnamed Entity");

		void OnUpdate(float dt, const Ref<Texture>& rendertarget);
		void OnUpdate(float dt, const Ref<Texture>& rendertarget, const xmmatrix& projection, const xmmatrix& cameraTransform);

		void SetActiveCamera(ECS::Entity entity);
		ECS::Entity GetActiveCameraEntity();
		
		inline ECS::Registry& GetRegistry() { return mRegistry; }
		inline TextureLibrary& GetTextureLibrary() { return mTextureLibrary; }
		inline MeshLibrary& GetMeshLibrary() { return mMeshLibrary; }
	private:
		ECS::Registry mRegistry;
		ECS::Entity mActiveCameraEntity = ECS::Null;
		TextureLibrary mTextureLibrary;
		MeshLibrary mMeshLibrary;
	};

}