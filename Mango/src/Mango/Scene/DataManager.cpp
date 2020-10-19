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
					auto& transform = entity.GetComponent<TransformComponent>();
					j["entities"][std::to_string(ID)]["components"]["transform"]["translation"] = { transform.Translation.x, transform.Translation.y, transform.Translation.z };
					j["entities"][std::to_string(ID)]["components"]["transform"]["rotation"] = { transform.Rotation.x, transform.Rotation.y, transform.Rotation.z };
					j["entities"][std::to_string(ID)]["components"]["transform"]["scale"] = { transform.Scale.x, transform.Scale.y, transform.Scale.z };
				}

				// Camera Component
				{
					if (entity.HasComponent<CameraComponent>()) {
						auto& cam = entity.GetComponent<CameraComponent>();
						if (cam.Camera.GetType() == Camera::Type::Orthographic) {
							j["entities"][std::to_string(ID)]["components"]["camera"]["type"] = "orthographic";
							j["entities"][std::to_string(ID)]["components"]["camera"]["size"] = cam.Camera.GetOSize();
						}
						else if (cam.Camera.GetType() == Camera::Type::Perspective) {
							j["entities"][std::to_string(ID)]["components"]["camera"]["type"] = "perspective";
							j["entities"][std::to_string(ID)]["components"]["camera"]["fov"] = cam.Camera.GetPFOV();
							j["entities"][std::to_string(ID)]["components"]["camera"]["nearPlane"] = cam.Camera.GetPNear();
							j["entities"][std::to_string(ID)]["components"]["camera"]["farPlane"] = cam.Camera.GetPFar();
						}
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

				// Mesh
				{
					if (entity.HasComponent<MeshComponent>())
					{
						auto& comp = entity.GetComponent<MeshComponent>();
						auto& mesh = comp.Mesh;
						if (comp.Type == MeshType::Empty) {
							j["entities"][std::to_string(ID)]["components"]["mesh"]["type"] = "empty";
						}
						else if (comp.Type == MeshType::Cube) {
							j["entities"][std::to_string(ID)]["components"]["mesh"]["type"] = "cube";
						}
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
				std::vector<float> translationArray = components["transform"]["translation"];
				float3 translation = *(float3*)(translationArray.data());
				std::vector<float> rotationArray = components["transform"]["rotation"];
				float3 rotation = *(float3*)(rotationArray.data());
				std::vector<float> scaleArray = components["transform"]["scale"];
				float3 scale = *(float3*)(scaleArray.data());

				entity.GetComponent<TransformComponent>() = TransformComponent(translation, rotation, scale);
			}

			// Camera
			{
				if (components.find("camera") != components.end())
				{
					json& camera = components["camera"];

					if (camera["type"] == "orthographic")
					{
						float size = camera["size"];
						entity.AddComponent<CameraComponent>(Camera::CreateOrthographic(size)).Primary = camera["primary"];
					}
					else if(camera["type"] == "perspective")
					{
						float fov = camera["fov"];
						float nearPlane = camera["nearPlane"];
						float farPlane = camera["farPlane"];

						entity.AddComponent<CameraComponent>(Camera::CreatePerspective(fov, nearPlane, farPlane)).Primary = camera["primary"];
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

			// Mesh
			{
				if (components.find("mesh") != components.end()) {
					json& mesh = components["mesh"];
					if (mesh["type"] == "empty") {
						entity.AddComponent<MeshComponent>();
					}
					else if (mesh["type"] == "cube") {
						entity.AddComponent<MeshComponent>(Mesh::CreateCube(), MeshType::Cube);
					}
				}
			}
		}

		return scene;
	}

}