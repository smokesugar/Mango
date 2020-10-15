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
		inline void SetAspectRatio(float aspect) { mAspectRatio = aspect; }

		inline ECS::Registry& GetRegistry() { return mRegistry; }
	private:
		float mAspectRatio = 1.0f;
		ECS::Registry mRegistry;
	};

}