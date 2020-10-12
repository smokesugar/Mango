#include "mgpch.h"
#include "Scene.h"

#include "Entity.h"
#include "Components.h"

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
		auto query = mRegistry.Query<TagComponent>();
		for (auto& tuple : query) {
			for (int i = 0; i < ECS_SIZE(tuple); i++) {
				auto& tag = std::get<1>(tuple)[i].Tag;
				MG_CORE_INFO("Tag: {0}", tag);
			}
		}
	}

}