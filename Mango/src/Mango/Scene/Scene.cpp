#include "mgpch.h"
#include "Scene.h"

#include "Components.h"
#include "Mango/Renderer/Renderer.h"

namespace Mango {

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{

	}

	ECS::Entity Scene::Create(const std::string& name)
	{
		auto entity = mRegistry.Create();
		mRegistry.Emplace<TransformComponent>(entity);
		mRegistry.Emplace<PreviousFrameTransformComponent>(entity);
		mRegistry.Emplace<TagComponent>(entity, name);;
		return entity;
	}

	void Scene::OnUpdate(float dt, const Ref<Framebuffer>& rendertarget)
	{
		if (mRegistry.Valid(mActiveCameraEntity) && !mRegistry.Has<CameraComponent>(mActiveCameraEntity))
			mActiveCameraEntity = ECS::Null;

		if (mRegistry.Valid(mActiveCameraEntity))
		{
			auto transform = mRegistry.Get<TransformComponent>(mActiveCameraEntity).GetMatrix();
			auto& camera = mRegistry.Get<CameraComponent>(mActiveCameraEntity).Camera;
			float aspect = (float)rendertarget->GetWidth() / (float)rendertarget->GetHeight();
			OnUpdate(dt, rendertarget, camera.GetProjectionMatrix(((float)rendertarget->GetWidth() / (float)rendertarget->GetHeight())), transform);
		}
	}

	void Scene::OnUpdate(float dt, const Ref<Framebuffer>& rendertarget, const xmmatrix& projection, const xmmatrix& cameraTransform)
	{
		Renderer::BeginScene(projection, cameraTransform, rendertarget->GetWidth(), rendertarget->GetHeight());

		auto query = mRegistry.Query<SpriteRendererComponent, TransformComponent, PreviousFrameTransformComponent>();
		for (auto& [size, sprites, transforms, previousTransforms] : query) {
			for (size_t i = 0; i < size; i++) {
				auto& spriteComp = sprites[i];
				auto transform = transforms[i].GetMatrix();
				auto& prevTransform = previousTransforms[i].Transform;

				if (spriteComp.UsesTexture && spriteComp.Texture) {
					Renderer::DrawQuad(prevTransform, transform, spriteComp.Texture);
				}
				else
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

	void Scene::SetActiveCamera(ECS::Entity entity)
	{
		mActiveCameraEntity = entity;
	}

	ECS::Entity Scene::GetActiveCameraEntity()
	{
		return mActiveCameraEntity;
	}

}