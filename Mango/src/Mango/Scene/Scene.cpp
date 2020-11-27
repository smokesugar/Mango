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
		if (mPlaying)
			Stop();
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
		InternalUpdate();

		if (mRegistry.Valid(mActiveCameraEntity) && !mRegistry.Has<CameraComponent>(mActiveCameraEntity))
			mActiveCameraEntity = ECS::Null;

		if (mRegistry.Valid(mActiveCameraEntity))
		{
			auto transform = mRegistry.Get<TransformComponent>(mActiveCameraEntity).GetMatrix();
			auto& camera = mRegistry.Get<CameraComponent>(mActiveCameraEntity).Camera;
			float aspect = (float)rendertarget->GetWidth() / (float)rendertarget->GetHeight();
			Render(rendertarget, camera.GetProjectionMatrix(aspect), transform);
		}
	}

	void Scene::OnUpdate(float dt, const Ref<Texture>& rendertarget, const xmmatrix& projection, const xmmatrix& cameraTransform)
	{
		InternalUpdate();
		Render(rendertarget, projection, cameraTransform);
	}

	void Scene::InternalUpdate()
	{
		if (mPlaying) {
			auto scriptQuery = mRegistry.Query<LuaScriptComponent>();
			bool shouldStop = false;
			for (auto& [size, scripts] : scriptQuery) {
				for (size_t i = 0; i < size; i++) {
					auto L = scripts[i].L;
					lua_getglobal(L, "update");
					if (lua_isfunction(L, -1)) {
						if (!LUA_FN_CALL(L, lua_pcall(L, 0, 0, 0)))
							shouldStop = true;
					}
					else {
						lua_pop(L, 1); // We need to call pop here in the case where we can't call "lua_pcall", which pops for us.
					}
				}
			}
			if (shouldStop) Stop();
		}
	}

	void Scene::Render(const Ref<Texture>& rendertarget, const xmmatrix& projection, const xmmatrix& cameraTransform)
	{
		Renderer::BeginScene(projection, cameraTransform, rendertarget->GetWidth(), rendertarget->GetHeight());

		auto lightQuery = mRegistry.Query<LightComponent, TransformComponent>();
		for (auto& [size, lights, transforms] : lightQuery) {
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
					xmvector xmdir = { 0.0f, 1.0f, 0.0f, 0.0f };
					xmdir = XMVector4Transform(xmdir, transform.GetMatrix());
					float3 dir;
					XMStoreFloat3(&dir, xmdir);
					Renderer::SubmitDirectionalLight(dir, color);
				}
			}
		}

		auto spriteQuery = mRegistry.Query<SpriteRendererComponent, TransformComponent, PreviousFrameTransformComponent>();
		for (auto& [size, sprites, transforms, previousTransforms] : spriteQuery) {
			for (size_t i = 0; i < size; i++) {
				auto& spriteComp = sprites[i];
				auto transform = transforms[i].GetMatrix();
				auto& prevTransform = previousTransforms[i].Transform;

				Renderer::SubmitQuad(prevTransform, transform, spriteComp.UsesTexture && spriteComp.Texture ? spriteComp.Texture : Renderer::GetWhiteTexture(), spriteComp.Color);

				prevTransform = transform;
			}
		}

		auto meshQuery = mRegistry.Query<MeshComponent, TransformComponent, PreviousFrameTransformComponent>();
		for (auto& [size, meshes, transforms, previousTransforms] : meshQuery) {
			for (size_t i = 0; i < size; i++) {
				auto& meshComp = meshes[i];
				auto transform = transforms[i].GetMatrix();
				auto& prevTransform = previousTransforms[i].Transform;

				if (meshComp.MeshIndex != -1)
					Renderer::SubmitMesh(*mMeshLibrary[meshComp.MeshIndex].second, prevTransform, transform);

				prevTransform = transform;
			}
		}

		Renderer::EndScene(rendertarget);
	}

	// Define custom log function
	static int lua_Log(lua_State* L) {
		const char* str = lua_tostring(L, 1);
		MG_CORE_TRACE("[Script] {0}", str);
		Application::Get().GetRuntimeLog().AddLog(std::string("[Script] ") + str + "\n");
		return 0;
	}

	void Scene::Start()
	{
		mPlaying = true;

		auto query = mRegistry.Query<LuaScriptComponent>();
		for (auto& [size, scriptComps] : query) {
			for (int i = 0; i < size; i++)
			{
				auto& comp = scriptComps[i];
				auto& L = comp.L;

				// Initialize
				L = luaL_newstate();
				luaL_openlibs(L);

				// Register Custom Functions
				lua_register(L, "Log", lua_Log);

				// Run the actual file
				LUA_FN_CALL(L, luaL_dofile(L, comp.Path.c_str()));
			}
		}
	}

	void Scene::Stop()
	{
		mPlaying = false;

		auto query = mRegistry.Query<LuaScriptComponent>();
		for (auto& [size, scriptComps] : query) {
			for (int i = 0; i < size; i++) {
				auto& L = scriptComps[i].L;
				lua_close(L);
				L = nullptr;
			}
		}
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