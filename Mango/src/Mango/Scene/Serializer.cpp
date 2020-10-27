#include "mgpch.h"
#include "Serializer.h"

#include "Components.h"

#include <json.hpp>
using namespace nlohmann;

namespace Mango {

	void Serializer::SerializeScene(const Ref<Scene>& scene, const std::string& filename)
	{
		json j;

		auto& reg = scene->GetRegistry();
		auto query = reg.QueryE<>();

		j["activeCamera"] = (uint32_t)scene->GetActiveCameraEntity();

		for (const auto& [size, entities] : query)
		{
			for (size_t i = 0; i < size; i++)
			{
				ECS::Entity entity = entities[i];

				// Tag Component
				{
					std::string tag = reg.Get<TagComponent>(entity).Tag;
					j["entities"][std::to_string(entity)]["components"]["tag"] = tag;
				}

				// Transform Component
				{
					auto& transform = reg.Get<TransformComponent>(entity);
					j["entities"][std::to_string(entity)]["components"]["transform"]["translation"] = { transform.Translation.x, transform.Translation.y, transform.Translation.z };
					j["entities"][std::to_string(entity)]["components"]["transform"]["rotation"] = { transform.Rotation.x, transform.Rotation.y, transform.Rotation.z };
					j["entities"][std::to_string(entity)]["components"]["transform"]["scale"] = { transform.Scale.x, transform.Scale.y, transform.Scale.z };
				}

				// Camera Component
				{
					if (reg.Has<CameraComponent>(entity)) {
						auto& cam = reg.Get<CameraComponent>(entity);
						if (cam.Camera.GetType() == Camera::Type::Orthographic) {
							j["entities"][std::to_string(entity)]["components"]["camera"]["type"] = "orthographic";
							j["entities"][std::to_string(entity)]["components"]["camera"]["size"] = cam.Camera.GetOSize();
						}
						else if (cam.Camera.GetType() == Camera::Type::Perspective) {
							j["entities"][std::to_string(entity)]["components"]["camera"]["type"] = "perspective";
							j["entities"][std::to_string(entity)]["components"]["camera"]["fov"] = cam.Camera.GetPFOV();
							j["entities"][std::to_string(entity)]["components"]["camera"]["nearPlane"] = cam.Camera.GetPNear();
							j["entities"][std::to_string(entity)]["components"]["camera"]["farPlane"] = cam.Camera.GetPFar();
						}
					}
				}

				// Sprite Renderer
				{
					if (reg.Has<SpriteRendererComponent>(entity))
					{
						auto& sprite = reg.Get<SpriteRendererComponent>(entity);
						j["entities"][std::to_string(entity)]["components"]["spriteRenderer"]["usesTexture"] = sprite.UsesTexture;
						if(sprite.UsesTexture)
							j["entities"][std::to_string(entity)]["components"]["spriteRenderer"]["texturePath"] = sprite.Texture ? sprite.Texture->GetPath() : "";
						else
							j["entities"][std::to_string(entity)]["components"]["spriteRenderer"]["color"] = { sprite.Color.x, sprite.Color.y, sprite.Color.z, sprite.Color.w };
					}
				}

				// Mesh
				{
					if (reg.Has<MeshComponent>(entity))
					{
						auto& comp = reg.Get<MeshComponent>(entity);
						auto& mesh = comp.Mesh;
						if (comp.Type == MeshType::Empty) {
							j["entities"][std::to_string(entity)]["components"]["mesh"]["type"] = "empty";
						}
						else if (comp.Type == MeshType::Cube) {
							j["entities"][std::to_string(entity)]["components"]["mesh"]["type"] = "cube";
						}
						else if (comp.Type == MeshType::Sphere) {
							j["entities"][std::to_string(entity)]["components"]["mesh"]["type"] = "sphere";
						}
						else if (comp.Type == MeshType::Capsule) {
							j["entities"][std::to_string(entity)]["components"]["mesh"]["type"] = "capsule";
						}
						else if (comp.Type == MeshType::Model) {
							j["entities"][std::to_string(entity)]["components"]["mesh"]["type"] = "model";
							j["entities"][std::to_string(entity)]["components"]["mesh"]["path"] = comp.Path;
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

	Ref<Scene> Serializer::DeserializeScene(const std::string& filename)
	{
		auto scene = CreateRef<Scene>();
		auto& reg = scene->GetRegistry();

		std::fstream file(filename);
		MG_CORE_ASSERT(file.is_open(), "Failed to read from scene file '{0}'.", filename);
		std::stringstream ss;
		ss << file.rdbuf();
		std::string scenestring = ss.str();
		json j = json::parse(scenestring);

		ECS::Entity activeCamera = j["activeCamera"];

		json& entities = j["entities"];
		for (auto& [id, e] : entities.items())
		{
			json& components = e["components"];
			ECS::Entity entity = scene->Create(components["tag"]);

			// Transform
			{
				std::vector<float> translationArray = components["transform"]["translation"];
				float3 translation = *(float3*)(translationArray.data());
				std::vector<float> rotationArray = components["transform"]["rotation"];
				float3 rotation = *(float3*)(rotationArray.data());
				std::vector<float> scaleArray = components["transform"]["scale"];
				float3 scale = *(float3*)(scaleArray.data());

				reg.Get<TransformComponent>(entity) = TransformComponent(translation, rotation, scale);
			}

			// Camera
			{
				if (components.find("camera") != components.end())
				{
					json& camera = components["camera"];

					if (std::stoi(id) == activeCamera)
						scene->SetActiveCamera(entity);


					if (camera["type"] == "orthographic")
					{
						float size = camera["size"];
						reg.Emplace<CameraComponent>(entity, Camera::CreateOrthographic(size));
					}
					else if(camera["type"] == "perspective")
					{
						float fov = camera["fov"];
						float nearPlane = camera["nearPlane"];
						float farPlane = camera["farPlane"];

						reg.Emplace<CameraComponent>(entity, Camera::CreatePerspective(fov, nearPlane, farPlane));
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
						reg.Emplace<SpriteRendererComponent>(entity, scene->GetTextureLibrary().Get(path));
					}
					else
					{
						std::vector<float> c = sprite["color"];
						float4 color;
						memcpy(&color, c.data(), sizeof(color));
						reg.Emplace<SpriteRendererComponent>(entity, color);
					}
					
				}
			}

			// Mesh
			{
				if (components.find("mesh") != components.end()) {
					json& mesh = components["mesh"];
					if (mesh["type"] == "empty") {
						reg.Emplace<MeshComponent>(entity);
					}
					else if (mesh["type"] == "cube") {
						reg.Emplace<MeshComponent>(entity, Mesh::CreateCube(), MeshType::Cube);
					}
					else if (mesh["type"] == "sphere") {
						reg.Emplace<MeshComponent>(entity, Mesh::CreateSphere(), MeshType::Sphere);
					}
					else if (mesh["type"] == "capsule") {
						reg.Emplace<MeshComponent>(entity, Mesh::CreateCapsule(), MeshType::Capsule);
					}
					else if (mesh["type"] == "model") {
						std::string path = mesh["path"];
						reg.Emplace<MeshComponent>(entity, path.empty() ? Mesh() : Mesh::CreateModel(path), MeshType::Model).Path = path;
					}
				}
			}
		}

		return scene;
	}

}