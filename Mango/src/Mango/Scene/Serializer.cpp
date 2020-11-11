#include "mgpch.h"
#include "Serializer.h"

#include "Components.h"
#include "Mango/Renderer/Render/Renderer.h"

#include <json.hpp>
using namespace nlohmann;

namespace Mango {

	static void DumpMaterial(json& j, const Ref<Material>& mat) {
		std::string path = mat->AlbedoTexture->GetPath();
		if (!path.empty())
			j["albedoTexture"] = path;

		if (mat->NormalTexture)
			j["normalTexture"] = mat->NormalTexture->GetPath();

		path = mat->RoughnessTexture->GetPath();
		if (!path.empty())
			j["roughnessTexture"] = path;
		
		j["albedoColor"] = { mat->AlbedoColor.x, mat->AlbedoColor.y, mat->AlbedoColor.z };
		j["roughnessValue"] = mat->RoughnessValue;
		j["metalness"] = mat->Metalness;
	}

	static Ref<Material> LoadMaterial(json& j, TextureLibrary& library) {
		Ref<Texture> albedoTexture;
		if (j.find("albedoTexture") != j.end())
			albedoTexture = library.Get(j["albedoTexture"], Format::RGBA8_UNORM_SRGB, Texture_Trilinear);
		else
			albedoTexture = Renderer::GetWhiteTexture();

		Ref<Texture> normalTexture;
		if (j.find("normalTexture") != j.end())
			normalTexture = library.Get(j["normalTexture"], Format::RGBA8_UNORM, Texture_Trilinear);

		Ref<Texture> roughnessTexture;
		if (j.find("roughnessTexture") != j.end())
			roughnessTexture = library.Get(j["roughnessTexture"], Format::RGBA8_UNORM, Texture_Trilinear);
		else
			roughnessTexture = Renderer::GetWhiteTexture();

		std::vector<float> albedoColorV = j["albedoColor"];
		float3 albedoColor = {albedoColorV[0], albedoColorV[1], albedoColorV[2] };
		float roughnessValue = j["roughnessValue"];
		float metalness = j["metalness"];

		return CreateRef<Material>(albedoTexture, normalTexture, roughnessTexture, albedoColor, roughnessValue, metalness);
	}

	void Serializer::SerializeScene(const Ref<Scene>& scene, const std::string& filename)
	{
		json j;

		auto& reg = scene->GetRegistry();
		auto query = reg.QueryE<>();

		j["activeCamera"] = (uint32_t)scene->GetActiveCameraEntity();
		if (Renderer::GetSkybox())
			j["skybox"] = Renderer::GetSkybox()->GetPath();
		j["envStrength"] = Renderer::EnvironmentStrength();

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
							DumpMaterial(j["entities"][std::to_string(entity)]["components"]["mesh"]["material"], mesh.Materials[0]);
						}
						else if (comp.Type == MeshType::Sphere) {
							j["entities"][std::to_string(entity)]["components"]["mesh"]["type"] = "sphere";
							DumpMaterial(j["entities"][std::to_string(entity)]["components"]["mesh"]["material"], mesh.Materials[0]);
						}
						else if (comp.Type == MeshType::Capsule) {
							j["entities"][std::to_string(entity)]["components"]["mesh"]["type"] = "capsule";
							DumpMaterial(j["entities"][std::to_string(entity)]["components"]["mesh"]["material"], mesh.Materials[0]);
						}
						else if (comp.Type == MeshType::Model) {
							j["entities"][std::to_string(entity)]["components"]["mesh"]["type"] = "model";
							j["entities"][std::to_string(entity)]["components"]["mesh"]["path"] = comp.Path;
							for (auto& mat : mesh.Materials) {
								json jmat;
								DumpMaterial(jmat, mat);
								j["entities"][std::to_string(entity)]["components"]["mesh"]["materials"].push_back(jmat);
							}
						}
					}
				}

				// Light
				{
					if (reg.Has<LightComponent>(entity)) {
						auto& comp = reg.Get<LightComponent>(entity);
						j["entities"][std::to_string(entity)]["components"]["light"]["type"] = comp.Type == LightType::Point ? "point" : "directional";
						j["entities"][std::to_string(entity)]["components"]["light"]["color"] = { comp.Color.x, comp.Color.y, comp.Color.z };
						j["entities"][std::to_string(entity)]["components"]["light"]["intensity"] = comp.Intensity;
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

		Renderer::EnvironmentStrength() = j["envStrength"];
		if (j.find("skybox") != j.end()) {
			std::string path = j["skybox"];
			Ref<Cubemap> newHdri = Ref<Cubemap>(Cubemap::Create(path, SKYBOX_RESOLUTION));
			Renderer::InitializeCubemap(newHdri);
			Renderer::SetSkybox(newHdri);
		}

		json& entities = j["entities"];
		for (auto& [id, e] : entities.items())
		{
			json& components = e["components"];
			ECS::Entity entity = scene->Create(components["tag"]);

			if (std::stoi(id) == activeCamera)
				scene->SetActiveCamera(entity);

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
						reg.Emplace<SpriteRendererComponent>(entity, scene->GetTextureLibrary().Get(path, Format::RGBA8_UNORM, Texture_Trilinear));
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
						Ref<Material> mat = LoadMaterial(mesh["material"], scene->GetTextureLibrary());
						reg.Emplace<MeshComponent>(entity, Mesh::CreateCube(mat), MeshType::Cube);
					}
					else if (mesh["type"] == "sphere") {
						Ref<Material> mat = LoadMaterial(mesh["material"], scene->GetTextureLibrary());
						reg.Emplace<MeshComponent>(entity, Mesh::CreateSphere(mat), MeshType::Sphere);
					}
					else if (mesh["type"] == "capsule") {
						Ref<Material> mat = LoadMaterial(mesh["material"], scene->GetTextureLibrary());
						reg.Emplace<MeshComponent>(entity, Mesh::CreateCapsule(mat), MeshType::Capsule);
					}
					else if (mesh["type"] == "model") {
						std::string path = mesh["path"];
						std::vector<Ref<Material>> mats;
						for (auto& j : mesh["materials"])
							mats.push_back(LoadMaterial(j, scene->GetTextureLibrary()));
						reg.Emplace<MeshComponent>(entity, path.empty() ? Mesh() : Mesh::CreateModel(mats, scene->GetTextureLibrary(), path), MeshType::Model).Path = path;
					}
				}
			}

			// Light
			{
				if (components.find("light") != components.end()) {
					auto& light = components["light"];
					std::vector<float> col = light["color"];
					float intensity = light["intensity"];
					std::string type = light["type"];
					reg.Emplace<LightComponent>(entity, *(float3*)col.data(), intensity, type == "point" ? LightType::Point : LightType::Directional);
				}
			}
		}

		return scene;
	}

}