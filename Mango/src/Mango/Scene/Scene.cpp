#include "mgpch.h"
#include "Scene.h"

#include "Entity.h"
#include "Components.h"
#include "Mango/Renderer/Renderer.h"

namespace Mango {

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{

	}

	Entity Scene::Create(const std::string& name)
	{
		Entity entity(mRegistry.Create(), this);
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name);
		return entity;
	}

	void Scene::OnUpdate(float dt)
	{
		Camera* currentCamera = nullptr;
		xmmatrix cameraTransform;
		auto camQuery = mRegistry.Query<CameraComponent, TransformComponent>();
		for (auto& [size, cameras, transforms] : camQuery) {
			for (size_t i = 0; i < size; i++) {
				if (currentCamera) continue;
				auto& camComp = cameras[i];
				if (camComp.Primary) {
					auto& transComp = transforms[i];
					currentCamera = &camComp.Camera;
					cameraTransform = transComp.GetTransform();
				}
			}
		}

		if (currentCamera) {
			Renderer::BeginScene(currentCamera->GetProjectionMatrix(mAspectRatio), cameraTransform);

			auto query = mRegistry.Query<SpriteRendererComponent, TransformComponent>();
			for (auto& [size, sprites, transforms] : query) {
				for (size_t i = 0; i < size; i++) {
					auto& spriteComp = sprites[i];
					auto& transformComp = transforms[i];

					if (spriteComp.UsesTexture && spriteComp.Texture) {
						Renderer::DrawQuad(transformComp.GetTransform(), spriteComp.Texture);
					} else
						Renderer::DrawQuad(transformComp.GetTransform(), spriteComp.Color);
				}
			}

			auto query1 = mRegistry.Query<MeshComponent, TransformComponent>();
			for (auto& [size, meshes, transforms] : query1) {
				for (size_t i = 0; i < size; i++) {
					auto& meshComp = meshes[i];
					auto& transform = transforms[i];

					Renderer::SubmitMesh(meshComp.Mesh, transform.GetTransform());
				}
			}

			Renderer::EndScene();
		}
	}

}