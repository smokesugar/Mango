#include "mgpch.h"
#include "Scene.h"

#include "Components.h"
#include "Mango/Renderer/Render/Renderer.h"

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

	void Scene::OnUpdate(float dt, const Ref<Texture>& rendertarget)
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

	void Scene::OnUpdate(float dt, const Ref<Texture>& rendertarget, const xmmatrix& projection, const xmmatrix& cameraTransform)
	{
		Renderer::BeginScene(projection, cameraTransform, rendertarget->GetWidth(), rendertarget->GetHeight());

		auto queryL = mRegistry.Query<LightComponent, TransformComponent>();
		for (auto& [size, lights, transforms] : queryL) {
			for (size_t i = 0; i < size; i++) {
				auto& light = lights[i];
				auto& transform = transforms[i];

				xmvector colxm = XMLoadFloat3(&light.Color);
				colxm *= light.Intensity;
				float3 color;
				XMStoreFloat3(&color, colxm);

				if (light.Type == LightType::Point)
					Renderer::SubmitPointLight(transform.Translation, color);
				else {
					xmvector xmdir = {0.0f, 1.0f, 0.0f, 0.0f};
					xmdir = XMVector4Transform(xmdir, transform.GetMatrix());
					float3 dir;
					XMStoreFloat3(&dir, xmdir);
					Renderer::SubmitDirectionalLight(dir, color);
				}
			}
		}

		auto query = mRegistry.Query<SpriteRendererComponent, TransformComponent, PreviousFrameTransformComponent>();
		for (auto& [size, sprites, transforms, previousTransforms] : query) {
			for (size_t i = 0; i < size; i++) {
				auto& spriteComp = sprites[i];
				auto transform = transforms[i].GetMatrix();
				auto& prevTransform = previousTransforms[i].Transform;

				Renderer::SubmitQuad(prevTransform, transform, spriteComp.UsesTexture && spriteComp.Texture? spriteComp.Texture : Renderer::GetWhiteTexture(), spriteComp.Color);

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