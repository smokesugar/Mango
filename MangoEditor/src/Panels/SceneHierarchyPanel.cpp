#include "SceneHierarchyPanel.h"

#include <queue>

#include "Mango/Core/FileDialog.h"
#include "Mango/Scene/Components.h"
#include "Mango/Renderer/Render/Renderer.h"

#include <imgui.h>

namespace Mango {
	
	static bool OpenTextureFileDialog(std::string& path) {
		return FileDialog::Open(path, L"Image File\0*.jpg;*.png;*.bmp\0All\0*.*\0");
	}

	SceneHierarchyPanel::SceneHierarchyPanel()
		: mScene(nullptr)
	{
	}

	template<typename T, typename UIFunction>
	static void  DrawComponent(const std::string& name, ECS::Entity entity, ECS::Registry& reg, UIFunction uifunction) {
		if (reg.Has<T>(entity))
		{
			bool open = ImGui::TreeNodeEx(typeid(T).name(), 0, name.c_str());

			bool remove = false;
			if (ImGui::BeginPopupContextItem()) {
				if (ImGui::MenuItem("Remove"))
					remove = true;
				ImGui::EndPopup();
			}

			if (open)
			{
				uifunction();
				ImGui::TreePop();
			}
			ImGui::Separator();

			if (remove)
				reg.Remove<T>(entity);
		}
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		// Scene Hierarchy -------------------------------------------------------------------------------------------------------------

		auto& reg = mScene->GetRegistry();

		std::queue<ECS::Entity> deletionQueue;

		ImGui::Begin("Scene Hierarchy");
		auto query = reg.QueryE<TagComponent>();
		for (auto& [size, entities, tags] : query)
		{
			for (size_t i = 0; i < size; i++)
			{
				auto id = entities[i];
				auto& tagComponent = tags[i];
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | (mSelectedEntity == id ? ImGuiTreeNodeFlags_Selected : 0);
				bool open = ImGui::TreeNodeEx((void*)(uint64_t)id, flags, tagComponent.Tag.c_str());

				if (ImGui::IsItemClicked())
					mSelectedEntity = id;

				bool remove = false;
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Delete Entity"))
						remove = true;
					ImGui::EndPopup();
				}

				if (open)
				{
					ImGui::TreePop();
				}

				if (remove)
					deletionQueue.push(id);
			}
		}

		while (!deletionQueue.empty()) {
			reg.Destroy(deletionQueue.front());
			deletionQueue.pop();
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0))
			mSelectedEntity = ECS::Null;

		if (ImGui::BeginPopupContextWindow(0, 1, false)) {
			if (ImGui::MenuItem("Create Empty")) {
				mSelectedEntity = mScene->Create();
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		// Material Panel ------------------------------------------------------------------------------------------------------------

		ImGui::Begin("Materials");
		if (reg.Has<MeshComponent>(mSelectedEntity)) {
			auto& comp = reg.Get<MeshComponent>(mSelectedEntity);
			if (comp.MeshIndex != -1) {
				auto& mesh = mScene->GetMeshLibrary()[reg.Get<MeshComponent>(mSelectedEntity).MeshIndex].second;
				size_t counter = 0;
				for (auto& material : mesh->Materials) {
					if (ImGui::TreeNodeEx((void*)material.get(), 0, ("Material" + std::to_string(counter++)).c_str())) {
						ImGui::Columns(2);

						bool color = material->AlbedoTexture == Renderer::GetWhiteTexture();
						bool roughnessVal = material->RoughnessTexture == Renderer::GetWhiteTexture();

						ImGui::AlignTextToFramePadding();
						ImGui::Text("Albedo");
						ImGui::AlignTextToFramePadding();
						ImGui::Text("");
						ImGui::AlignTextToFramePadding();
						ImGui::Text("Roughness");
						ImGui::AlignTextToFramePadding();
						ImGui::Text("");
						if (!roughnessVal) {
							ImGui::AlignTextToFramePadding();
							ImGui::Text("");
						}
						ImGui::AlignTextToFramePadding();
						ImGui::Text("Metalness");
						ImGui::AlignTextToFramePadding();
						ImGui::Text("Normal");

						ImGui::NextColumn();

						if (ImGui::BeginCombo("##combo_albedo", color ? "Color" : "Texture")) {
							if (ImGui::Selectable("Color") && !color) {
								material->AlbedoTexture = Renderer::GetWhiteTexture();
							}
							if (ImGui::Selectable("Texture") && color) {
								material->AlbedoTexture = Renderer::GetBlackTexture();
								material->AlbedoColor = float3(1.0f, 1.0f, 1.0f);
							}
							ImGui::EndCombo();
						}

						if (color) {
							ImGui::ColorEdit3("##material_albedocolor", ValuePtr(material->AlbedoColor));
						}
						else {
							char buf[64];
							memset(buf, 0, sizeof(buf));
							std::string path = material->AlbedoTexture->GetPath();
							memcpy(buf, path.c_str(), Min(path.size(), sizeof(buf)));
							ImGui::InputText("##material_albedotex", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
							ImGui::SameLine();
							if (ImGui::Button("...##0")) {
								std::string newPath;
								if (OpenTextureFileDialog(newPath))
									material->AlbedoTexture = mScene->GetTextureLibrary().Get(newPath, Format::RGBA8_UNORM_SRGB, Texture_Trilinear);
							}
						}

						if (ImGui::BeginCombo("##combo_roughness", roughnessVal ? "Float" : "Texture")) {
							if (ImGui::Selectable("Float") && !roughnessVal) {
								material->RoughnessTexture = Renderer::GetWhiteTexture();
								material->RoughnessValue = 0.5f;
							}
							if (ImGui::Selectable("Texture") && roughnessVal) {
								material->RoughnessTexture = Renderer::GetBlackTexture();
								material->RoughnessValue = 1.0f;
							}
							ImGui::EndCombo();
						}

						if (!roughnessVal) {
							char buf[64];
							memset(buf, 0, sizeof(buf));
							std::string path = material->RoughnessTexture->GetPath();
							memcpy(buf, path.c_str(), Min(path.size(), sizeof(buf)));
							ImGui::InputText("##material_roughnesstex", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
							ImGui::SameLine();
							if (ImGui::Button("...##1")) {
								std::string newPath;
								if (OpenTextureFileDialog(newPath))
									material->RoughnessTexture = mScene->GetTextureLibrary().Get(newPath, Format::RGBA8_UNORM, Texture_Trilinear);
							}
						}
						ImGui::DragFloat("##material_roughnessValue", &material->RoughnessValue, 0.01f, 0.0f, 1.0f);

						ImGui::DragFloat("##material_metalness", &material->Metalness, 0.01f, 0.0f, 1.0f);

						char buf[64];
						memset(buf, 0, sizeof(buf));
						if (material->NormalTexture) {
							std::string path = material->NormalTexture->GetPath();
							memcpy(buf, path.c_str(), Min(path.size(), sizeof(buf)));
						}
						ImGui::InputText("##material_normaltex", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
						ImGui::SameLine();
						if (ImGui::Button("Clear"))
							material->NormalTexture.reset();
						ImGui::SameLine();
						if (ImGui::Button("...##2")) {
							std::string newPath;
							if (OpenTextureFileDialog(newPath))
								material->NormalTexture = mScene->GetTextureLibrary().Get(newPath, Format::RGBA8_UNORM, Texture_Trilinear);
						}

						ImGui::Columns(1);
						ImGui::TreePop();
					}
					ImGui::Separator();
				}
			}
		}
		ImGui::End();

		// Properties ----------------------------------------------------------------------------------------------------------------

		ImGui::Begin("Properties");
		if (reg.Valid(mSelectedEntity))
		{
			// Tag Component
			{
				std::string& tag = reg.Get<TagComponent>(mSelectedEntity).Tag;
				char buf[64];
				memset(buf, 0, sizeof(buf));
				memcpy(buf, tag.c_str(), tag.size());
				if (ImGui::InputText("##entity_tag", buf, sizeof(buf)))
					tag.assign(buf);
				ImGui::Separator();
			}

			// Transform Component
			{
				if (ImGui::TreeNodeEx(typeid(TransformComponent).name(), 0, "Transform")) {
					ImGui::Columns(2);
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Position");
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Rotation");
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Scale");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1.0f);
					auto& transform = reg.Get<TransformComponent>(mSelectedEntity);
					ImGui::DragFloat3("##transform_position", ValuePtr(transform.Translation), 0.01f);
					ImGui::DragFloat3("##transform_rotation", ValuePtr(transform.Rotation), 0.1f);
					ImGui::DragFloat3("##transform_scale", ValuePtr(transform.Scale), 0.01f, 0.001f, INFINITY);
					transform.Scale = Max(float3(0.001f, 0.001f, 0.001f), transform.Scale);
					ImGui::PopItemWidth();
					ImGui::Columns(1);
					ImGui::TreePop();
				}
				ImGui::Separator();
			}

			// Camera Component
			DrawComponent<CameraComponent>("Camera", mSelectedEntity, reg, [&]() {
				auto& cameraComp = reg.Get<CameraComponent>(mSelectedEntity);

				bool orthographic = cameraComp.Camera.GetType() == Camera::Type::Orthographic;

				ImGui::Columns(2);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Type");

				if (!orthographic) {
					ImGui::AlignTextToFramePadding();
					ImGui::Text("FOV");
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Near Plane");
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Far Plane");
				}
				if (orthographic) {
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Size");
				}

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Enabled");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1.0f);

				const char* wantedType = orthographic ? "o" : "p";
				if (ImGui::BeginCombo("##camera_type", orthographic ? "Orthographic" : "Perspective"))
				{
					if (ImGui::Selectable("Orthographic", orthographic) && !orthographic) {
						cameraComp.Camera = Camera::CreateOrthographic(2.5f);
						orthographic = true;
					}
					if (ImGui::Selectable("Perspective", !orthographic) && orthographic) {
						cameraComp.Camera = Camera::CreatePerspective(ToRadians(45.0f), 0.1f, 1000.0f);
						orthographic = false;
					}

					ImGui::EndCombo();
				}

				if (!orthographic) {
					auto& cam = cameraComp.Camera;

					float fov = ToDegrees(cam.GetPFOV());
					ImGui::DragFloat("##camera_fov", &fov, 1.0f, 10.0f, 120.0f);
					cam.SetPFOV(ToRadians(fov));

					float nearPlane = cam.GetPNear();
					ImGui::InputFloat("##camera_near", &nearPlane);
					cam.SetPNear(nearPlane);

					float farPlane = cam.GetPFar();
					ImGui::InputFloat("##camera_far", &farPlane);
					cam.SetPFar(farPlane);
				}
				if (orthographic) {
					auto& cam = cameraComp.Camera;
					float size = cam.GetOSize();
					ImGui::DragFloat("##camera_size", &size, 0.1f, 0.001f, INFINITY);
					cam.SetOSize(Max(size, 0.001f));
				}

				bool enabled = mSelectedEntity == mScene->GetActiveCameraEntity();
				if (ImGui::Checkbox("##camera_enabled", &enabled)) {
					if (enabled)
						mScene->SetActiveCamera(mSelectedEntity);
					else
						mScene->SetActiveCamera(ECS::Null);
				}
				ImGui::PopItemWidth();
				ImGui::Columns(1);
			});

			// Sprite Renderer Component
			DrawComponent<SpriteRendererComponent>("Sprite Renderer", mSelectedEntity, reg, [&]() {
				auto& sprite = reg.Get<SpriteRendererComponent>(mSelectedEntity);
				ImGui::Columns(2);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Type");
				if (sprite.UsesTexture) {
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Texture");
				}
				else {
					ImGui::AlignTextToFramePadding();
					ImGui::Text("Color");
				}

				ImGui::NextColumn();
				ImGui::PushItemWidth(-1.0f);

				if (ImGui::BeginCombo("##sprite_type", sprite.UsesTexture ? "Texture" : "Color")) {
					if (ImGui::Selectable("Texture", sprite.UsesTexture) && !sprite.UsesTexture)
						sprite = SpriteRendererComponent(Ref<Texture>());
					if (ImGui::Selectable("Color", !sprite.UsesTexture) && sprite.UsesTexture)
						sprite = SpriteRendererComponent(float4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::EndCombo();
				}

				if (sprite.UsesTexture) {
					char path[64];
					memset(path, 0, sizeof(path));
					if (sprite.Texture)
						memcpy(path, sprite.Texture->GetPath().c_str(), Min(sizeof(path), sprite.Texture->GetPath().size()));
					ImGui::PopItemWidth();
					ImGui::InputText("##sprite_texture_path", path, sizeof(path), ImGuiInputTextFlags_ReadOnly);
					ImGui::SameLine();
					if (ImGui::Button("..."))
					{
						std::string path;
						if (OpenTextureFileDialog(path)) {
							sprite.Texture = mScene->GetTextureLibrary().Get(path, Format::RGBA8_UNORM, Texture_Trilinear);
						}
					}
					ImGui::PushItemWidth(-1.0f);
				}
				else
					ImGui::ColorEdit4("##sprite_color", ValuePtr(sprite.Color));

				ImGui::PopItemWidth();
				ImGui::Columns(1);
			});

			DrawComponent<MeshComponent>("Mesh Renderer", mSelectedEntity, reg, [&]() {
				auto& comp = reg.Get<MeshComponent>(mSelectedEntity);

				ImGui::PushItemWidth(-1.0f);

				char buf[64];
				memset(buf, 0, sizeof(buf));
				if (comp.MeshIndex != -1) {
					std::string* meshname = &mScene->GetMeshLibrary()[comp.MeshIndex].first;
					memcpy(buf, meshname->c_str(), Min(sizeof(buf), meshname->size()));
				}
				ImGui::InputText("##model_name", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

				if (ImGui::BeginDragDropTarget()) {
					auto payload = ImGui::AcceptDragDropPayload("payload_meshindex");
					if (payload) {
						MG_ASSERT(payload->DataSize == sizeof(int), "Incorrect ImGui payload size.");
						comp.MeshIndex = *(int*)payload->Data;
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::PopItemWidth();
			});

			// Light
			DrawComponent<LightComponent>("Light", mSelectedEntity, reg, [&]() {
				auto& light = reg.Get<LightComponent>(mSelectedEntity);
				ImGui::Columns(2);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Type");
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Color");
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Intensity");

				ImGui::NextColumn();
				ImGui::PushItemWidth(-1.0f);

				bool typePoint = light.Type == LightType::Point;

				if (ImGui::BeginCombo("##light_type", typePoint ? "Point" : "Directional")) {
					if (ImGui::Selectable("Point", typePoint) && !typePoint)
						light = LightComponent(float3(1.0f, 1.0f, 1.0f), 1.0f, LightType::Point);
					if (ImGui::Selectable("Directional", !typePoint) && typePoint)
						light = LightComponent(float3(1.0f, 1.0f, 1.0f), 1.0f, LightType::Directional);
					ImGui::EndCombo();
				}

				ImGui::ColorEdit3("##light_color", ValuePtr(light.Color));
				ImGui::DragFloat("##light_intensity", &light.Intensity, 1.0f, 0.0f, INFINITY);

				ImGui::PopItemWidth();
				ImGui::Columns(1);
			});

			DrawComponent<LuaScriptComponent>("Lua Script", mSelectedEntity, reg, [&]() {
				auto& comp = reg.Get<LuaScriptComponent>(mSelectedEntity);

				char buf[64];
				memset(buf, 0, sizeof(buf));
				memcpy(buf, comp.Path.c_str(), Min(sizeof(buf), comp.Path.size()));

				ImGui::InputText("##lua_script_path", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
				ImGui::SameLine();
				if (ImGui::Button(". . .")) {
					std::string path;
					if (FileDialog::Open(path, L"Lua Script\0*.lua\0All\0*.*\0")) {
						comp = LuaScriptComponent(path);
					}
				}
			});

			// Add Component Button
			{
				if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvailWidth(), 0.0f)))
					ImGui::OpenPopup("add_component");

				if (ImGui::BeginPopup("add_component")) {
					if (!reg.Has<CameraComponent>(mSelectedEntity)) {
						if (ImGui::MenuItem("Camera"))
							reg.Emplace<CameraComponent>(mSelectedEntity, Camera::CreatePerspective(ToRadians(45.0f), 0.1f, 1000.0f));
					}
					if (!reg.Has<SpriteRendererComponent>(mSelectedEntity)) {
						if (ImGui::MenuItem("Sprite Renderer"))
							reg.Emplace<SpriteRendererComponent>(mSelectedEntity);
					}
					if (!reg.Has<MeshComponent>(mSelectedEntity)) {
						if (ImGui::MenuItem("Mesh"))
							reg.Emplace<MeshComponent>(mSelectedEntity);
					}
					if (!reg.Has<LightComponent>(mSelectedEntity)) {
						if (ImGui::MenuItem("Light"))
							reg.Emplace<LightComponent>(mSelectedEntity, float3(1.0f, 1.0f, 1.0f), 1.0f, LightType::Point);
					}
					if (!reg.Has<LuaScriptComponent>(mSelectedEntity)) {
						if (ImGui::MenuItem("Lua Script"))
							reg.Emplace<LuaScriptComponent>(mSelectedEntity);
					}
					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();
		
	}

}