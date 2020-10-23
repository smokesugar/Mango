#pragma once

#include "Scene.h"

namespace Mango {

	namespace Serializer {
		void SerializeScene(const Ref<Scene>& scene, const std::string& filename);
		Ref<Scene> DeserializeScene(const std::string& filename);
	}

}