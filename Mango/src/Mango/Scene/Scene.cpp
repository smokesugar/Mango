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
		entity.AddComponent<TransformComponent>(XMMatrixIdentity());
		entity.AddComponent<TagComponent>(name);
		return entity;
	}

	void Scene::OnUpdate(float dt)
	{
		auto query = mRegistry.Query<SpriteRendererComponent, TransformComponent>();
		for (auto& tuple : query) {
			for (size_t i = 0; i < ECS_SIZE(tuple); i++) {
				auto& spriteComp = std::get<1>(tuple)[i];
				auto& transformComp = std::get<2>(tuple)[i];
				Renderer::DrawQuad(transformComp.Transform, spriteComp.Color);
			}
		}
	}

}