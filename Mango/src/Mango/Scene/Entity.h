#pragma once

#include "Scene.h"

namespace Mango {
	
	class Entity {
	public:
		Entity()
			:
			mHandle(std::numeric_limits<uint32_t>::max()),
			mScene(nullptr)
		{}

		Entity(ECS::Entity handle, Scene* scene)
			: mHandle(handle), mScene(scene)
		{}

		Entity(const Entity& other)
			: mHandle(other.mHandle),
			mScene(other.mScene)
		{}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			return mScene->mRegistry.Emplace<T>(mHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		void RemoveComponent() {
			mScene->mRegistry.Remove<T>(mHandle);
		}

		template<typename T>
		T& GetComponent() {
			return mScene->mRegistry.Get<T>(mHandle);
		}

		bool IsValid() {
			return mScene ? mScene->mRegistry.Valid(mHandle) : false;
		}

		bool operator==(const Entity& other) const {
			return mScene == other.mScene && mHandle == other.mHandle;
		}

		bool operator!=(const Entity& other) const {
			return !(*this == other);
		}

	private:
		ECS::Entity mHandle;
		Scene* mScene;
	};

}