#include "mgpch.h"
#include "DataManager.h"

#include "Components.h"
#include "Entity.h"

#include <json.hpp>
using namespace nlohmann;

namespace Mango {

	void DataManager::SerializeScene(const Ref<Scene>& scene, const std::string& filename)
	{
		json j;

		auto& reg = scene->GetRegistry();
		auto query = reg.QueryE<>();
		for (const auto& [size, entities] : query)
		{
			for (size_t i = 0; i < size; i++)
			{
				ECS::Entity ID = entities[i];
				Entity entity(ID, scene.get());

				// Tag Component
				{
					std::string tag = entity.GetComponent<TagComponent>().Tag;
					j[std::to_string(ID)]["components"]["tag"] = tag;
				}

				// Transform Component
				{
					float4x4 mat; XMStoreFloat4x4(&mat, entity.GetComponent<TransformComponent>().Transform);
					std::vector<float> matData; matData.resize(16);
					memcpy(matData.data(), ValuePtr(mat), sizeof(mat));
					j[std::to_string(ID)]["components"]["transform"] = matData;
				}

				// Camera Component
				{
					if (entity.HasComponent<CameraComponent>()) {
						auto& cam = entity.GetComponent<CameraComponent>();
						if (cam.Camera->GetType() == Camera::Type::Orthographic) {
							j[std::to_string(ID)]["components"]["camera"]["zoom"] = std::static_pointer_cast<OrthographicCamera>(cam.Camera)->GetZoom();
						}
						j[std::to_string(ID)]["components"]["camera"]["aspectRatio"] = cam.Camera->GetAspectRatio();
						j[std::to_string(ID)]["components"]["camera"]["primary"] = cam.Primary;
					}
				}

				// Sprite Renderer
				{
					if (entity.HasComponent<SpriteRendererComponent>()) {
						auto& sprite = entity.GetComponent<SpriteRendererComponent>();
						j[std::to_string(ID)]["components"]["sprite"]["color"] = { sprite.Color.x, sprite.Color.y, sprite.Color.z, sprite.Color.w};
					}
				}
			}
		}
		
		std::ofstream file(filename, std::ios::out);
		MG_CORE_ASSERT(file.is_open(), "Failed to write to file '{0}'.", filename);
		file << j.dump(4);
		file.close();
	}

	Ref<Scene> DataManager::DeserializeScene(const std::string& filename)
	{
		return Ref<Scene>();
	}

}