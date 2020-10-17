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
					j["entities"][std::to_string(ID)]["components"]["tag"] = tag;
				}

				// Transform Component
				{
					float4x4 mat; XMStoreFloat4x4(&mat, entity.GetComponent<TransformComponent>().Transform);
					std::vector<float> matData; matData.resize(16);
					memcpy(matData.data(), ValuePtr(mat), sizeof(mat));
					j["entities"][std::to_string(ID)]["components"]["transform"] = matData;
				}

				// Camera Component
				{
					if (entity.HasComponent<CameraComponent>()) {
						auto& cam = entity.GetComponent<CameraComponent>();
						if (cam.Camera->GetType() == Camera::Type::Orthographic) {
							j["entities"][std::to_string(ID)]["components"]["camera"]["type"] = "orthographic";
							j["entities"][std::to_string(ID)]["components"]["camera"]["zoom"] = std::static_pointer_cast<OrthographicCamera>(cam.Camera)->GetZoom();
						}
						else if (cam.Camera->GetType() == Camera::Type::Perspective) {
							j["entities"][std::to_string(ID)]["components"]["camera"]["type"] = "perspective";
							j["entities"][std::to_string(ID)]["components"]["camera"]["fov"] = std::static_pointer_cast<PerspectiveCamera>(cam.Camera)->GetFOV();
							j["entities"][std::to_string(ID)]["components"]["camera"]["nearPlane"] = std::static_pointer_cast<PerspectiveCamera>(cam.Camera)->GetNearPlane();
							j["entities"][std::to_string(ID)]["components"]["camera"]["farPlane"] = std::static_pointer_cast<PerspectiveCamera>(cam.Camera)->GetFarPlane();
						}
						j["entities"][std::to_string(ID)]["components"]["camera"]["aspectRatio"] = cam.Camera->GetAspectRatio();
						j["entities"][std::to_string(ID)]["components"]["camera"]["primary"] = cam.Primary;
					}
				}

				// Sprite Renderer
				{
					if (entity.HasComponent<SpriteRendererComponent>())
					{
						auto& sprite = entity.GetComponent<SpriteRendererComponent>();
						j["entities"][std::to_string(ID)]["components"]["spriteRenderer"]["usesTexture"] = sprite.UsesTexture;
						if(sprite.UsesTexture)
							j["entities"][std::to_string(ID)]["components"]["spriteRenderer"]["texturePath"] = sprite.Texture ? sprite.Texture->GetPath() : "";
						else
							j["entities"][std::to_string(ID)]["components"]["spriteRenderer"]["color"] = { sprite.Color.x, sprite.Color.y, sprite.Color.z, sprite.Color.w };
					}
				}
			}
		}
		
		std::ofstream file(filename, std::ios::out);
		MG_CORE_ASSERT(file.is_open(), "Failed to write to scene file '{0}'.", filename);
		file << j.dump(4);
		file.close();
	}

	Ref<Scene> DataManager::DeserializeScene(const std::string& filename)
	{
		auto scene = CreateRef<Scene>();

		std::fstream file(filename);
		MG_CORE_ASSERT(file.is_open(), "Failed to read from scene file '{0}'.", filename);
		std::stringstream ss;
		ss << file.rdbuf();
		std::string scenestring = ss.str();
		json j = json::parse(scenestring);

		json& entities = j["entities"];
		for (auto& e : entities)
		{
			json& components = e["components"];
			Entity entity = scene->Create(components["tag"]);

			// Transform
			{
				float4x4 transform;
				std::vector<float> trans = components["transform"];
				memcpy(&transform, trans.data(), sizeof(transform));
				entity.GetComponent<TransformComponent>().Transform = XMLoadFloat4x4(&transform);
			}

			// Camera
			{
				if (components.find("camera") != components.end())
				{
					json& camera = components["camera"];

					if (camera["type"] == "orthographic")
					{
						float aspect = camera["aspectRatio"];
						float zoom = camera["zoom"];
						entity.AddComponent<CameraComponent>(CreateRef<OrthographicCamera>(aspect, zoom)).Primary = camera["primary"];
					}
					else if(camera["type"] == "perspective")
					{
						float aspect = camera["aspectRatio"];
						float fov = camera["fov"];
						float nearPlane = camera["nearPlane"];
						float farPlane = camera["farPlane"];

						entity.AddComponent<CameraComponent>(CreateRef<PerspectiveCamera>(fov, aspect, nearPlane, farPlane)).Primary = camera["primary"];
					}
				}
			}

			// Sprite
			{
				if (components.find("spriteRenderer") != components.end()) {
					json& sprite = components["spriteRenderer"];
					bool usesTexture = sprite["usesTexture"];
					if (usesTexture) {
						std::string path = sprite["texturePath"];
						entity.AddComponent<SpriteRendererComponent>(scene->GetTextureLibrary().Get(path));
					}
					else
					{
						std::vector<float> c = sprite["color"];
						float4 color;
						memcpy(&color, c.data(), sizeof(color));
						entity.AddComponent<SpriteRendererComponent>(color);
					}
					
				}
			}
		}

		return scene;
	}

}