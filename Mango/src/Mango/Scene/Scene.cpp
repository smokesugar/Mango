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
		entity.AddComponent<PreviousFrameTransformComponent>();
		entity.AddComponent<TagComponent>(name);
		return entity;
	}

	void Scene::OnUpdate(float dt, const Ref<Framebuffer>& rendertarget)
	{
		Entity activeCamera(mActiveCameraEntity, this);

		if (activeCamera.IsValid()) {
			float aspect = (float)rendertarget->GetWidth() / (float)rendertarget->GetHeight();

			auto& camera = activeCamera.GetComponent<CameraComponent>().Camera;
			auto cameraTransform = activeCamera.GetComponent<TransformComponent>().GetMatrix();

			Renderer::BeginScene(camera.GetProjectionMatrix(aspect), cameraTransform, rendertarget->GetWidth(), rendertarget->GetHeight());

			auto query = mRegistry.Query<SpriteRendererComponent, TransformComponent, PreviousFrameTransformComponent>();
			for (auto& [size, sprites, transforms, previousTransforms] : query) {
				for (size_t i = 0; i < size; i++) {
					auto& spriteComp = sprites[i];
					auto transform = transforms[i].GetMatrix();
					auto& prevTransform = previousTransforms[i].Transform;

					if (spriteComp.UsesTexture && spriteComp.Texture) {
						Renderer::DrawQuad(prevTransform, transform, spriteComp.Texture);
					} else
						Renderer::DrawQuad(prevTransform, transform, spriteComp.Color);

					prevTransform = transform;
				}
			}

			auto query1 = mRegistry.Query<MeshComponent, TransformComponent, PreviousFrameTransformComponent>();
			for (auto& [size, meshes, transforms, previousTransforms] : query1) {
				for (size_t i = 0; i < size; i++) {
					auto& meshComp = meshes[i];
					auto transform = transforms[i].GetMatrix();
					auto& prevTransform = previousTransforms[i].Transform;

					Renderer::SubmitMesh(meshComp.Mesh, prevTransform, transform);

					prevTransform = transform;
				}
			}

			Renderer::EndScene(rendertarget);
		}
	}

	void Scene::SetActiveCamera(const Entity& entity)
	{
		mActiveCameraEntity = entity.GetID();
	}

	Entity Scene::GetActiveCameraEntity()
	{
		return Entity(mActiveCameraEntity, this);
	}

}