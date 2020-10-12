#pragma once

#include "Mango/Core/ECS.h"

namespace Mango {

	class Scene {
		friend class Entity;
	public:
		Scene();
		~Scene();

		class Entity Create(const std::string& name = "Unnamed Entity");

		void OnUpdate(float dt);
	private:
		ECS::Registry mRegistry;
	};

}