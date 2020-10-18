#include "SceneHierarchyPanel.h"

#include <queue>

#include "Mango/Core/FileDialog.h"
#include "Mango/Scene/Components.h"
#include "Mango/Scene/Entity.h"

#include <imgui.h>

namespace Mango {

	SceneHierarchyPanel::SceneHierarchyPanel()
		: mScene(nullptr)
	{
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		// Scene Hierarchy -------------------------------------------------------------------------------------------------------------

		static ECS::Entity rightClickedEntity = ECS::Null;

		ImGui::Begin("Scene Hierarchy");
		if (ImGui::Button("Create Entity"))
			mScene->Create();
		auto query = mScene->GetRegistry().QueryE<TagComponent>();
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
				if (ImGui::IsItemClicked(1)) {
					ImGui::OpenPopup("delete_entity");
					rightClickedEntity = id;
				}

				if (open)
				{
					ImGui::TreePop();
				}
			}
		}
		
		if (ImGui::BeginPopup("delete_entity")) {
			if (ImGui::Button("Delete")) {
				if(mScene->GetRegistry().Valid(rightClickedEntity))
					mScene->GetRegistry().Destroy(rightClickedEntity);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0))
			mSelectedEntity = ECS::Null;

		ImGui::End();

		// Properties ----------------------------------------------------------------------------------------------------------------

		ImGui::Begin("Properties");
		if (mScene->GetRegistry().Valid(mSelectedEntity))
		{
			Entity entity = Entity(mSelectedEntity, mScene);
			static std::function<void()> deleteFn = []() {};

			// Tag Component
			{
				std::string& tag = entity.GetComponent<TagComponent>().Tag;
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
					ImGui::Text("Rotation");
					ImGui::Text("Scale");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1.0f);
					xmmatrix& transform = entity.GetComponent<TransformComponent>().Transform;
					float3 position, rotation, scale;
					DecomposeMatrix(&position, &rotation, &scale, transform);
					ImGui::DragFloat3("##transform_position", ValuePtr(position), 0.01f);
					ImGui::DragFloat3("##transform_rotation", ValuePtr(rotation), 0.1f);
					ImGui::DragFloat3("##transform_scale", ValuePtr(scale), 0.01f, 0.001f, INFINITY);
					scale = Max(float3(0.001f, 0.001f, 0.001f), scale);
					transform = RecomposeMatrix(position, rotation, scale);
					ImGui::PopItemWidth();
					ImGui::Columns(1);
					ImGui::TreePop();
				}
				ImGui::Separator();
			}

			// Camera Component
			{
				if (entity.HasComponent<CameraComponent>())
				{
					bool open = ImGui::TreeNodeEx(typeid(CameraComponent).name(), 0, "Camera");

					if (ImGui::IsItemClicked(1)) {
						deleteFn = [&]() {
							Entity ent(mSelectedEntity, mScene);
							ent.RemoveComponent<CameraComponent>();
						};
						ImGui::OpenPopup("remove_component");
					}

					if (open)
					{
						auto& cameraComp = entity.GetComponent<CameraComponent>();

						bool orthographic = cameraComp.Camera.GetType() == Camera::Type::Orthographic;

						ImGui::Columns(2);
						ImGui::AlignTextToFramePadding();

						ImGui::Text("Type");

						if (!orthographic) {
							ImGui::Text("FOV");
							ImGui::Text("Near Plane");
							ImGui::Text("Far Plane");
						}
						if (orthographic) {
							ImGui::Text("Size");
						}

						ImGui::Text("Primary");
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1.0f);

						const char* wantedType = orthographic ? "o" : "p";
						if (ImGui::BeginCombo("##camera_type", orthographic ? "Orthographic" : "Perspective"))
						{
							if (ImGui::Selectable("Orthographic", orthographic)) {
								wantedType = "o";
							}
							if (ImGui::Selectable("Perspective", !orthographic)) {
								wantedType = "p";
							}

							ImGui::EndCombo();
						}
						if (wantedType == "o" && !orthographic) {
							cameraComp.Camera = Camera::CreateOrthographic(2.5f);
							orthographic = true;
						}
						if (wantedType == "p" && orthographic) {
							cameraComp.Camera = Camera::CreatePerspective(ToRadians(45.0f), 0.1f, 100.0f);
							orthographic = false;
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

						ImGui::Checkbox("##camera_primary", &cameraComp.Primary);
						ImGui::PopItemWidth();
						ImGui::Columns(1);
						ImGui::TreePop();
					}
					ImGui::Separator();
				}
			}

			// Sprite Renderer Component
			{
				if (entity.HasComponent<SpriteRendererComponent>())
				{
					bool open = ImGui::TreeNodeEx(typeid(SpriteRendererComponent).name(), 0, "Sprite Renderer");

					if (ImGui::IsItemClicked(1)) {
						deleteFn = [&]() {
							Entity ent(mSelectedEntity, mScene);
							ent.RemoveComponent<SpriteRendererComponent>();
						};
						ImGui::OpenPopup("remove_component");
					}

					if (open) {
						auto& sprite = entity.GetComponent<SpriteRendererComponent>();
						ImGui::Columns(2);
						ImGui::AlignTextToFramePadding();

						ImGui::Text("Type");
						if (sprite.UsesTexture)
							ImGui::Text("Texture");
						else
							ImGui::Text("Color");

						ImGui::NextColumn();
						ImGui::PushItemWidth(-1.0f);

						const char* wanted = sprite.UsesTexture ? "t" : "c";
						if (ImGui::BeginCombo("##sprite_type", sprite.UsesTexture ? "Texture" : "Color")) {
							if (ImGui::Selectable("Texture", sprite.UsesTexture))
								wanted = "t";
							if (ImGui::Selectable("Color", !sprite.UsesTexture))
								wanted = "c";
							ImGui::EndCombo();
						}
						if (wanted == "c" && sprite.UsesTexture)
							sprite = SpriteRendererComponent(float4(1.0f, 1.0f, 1.0f, 1.0f));
						if (wanted == "t" && !sprite.UsesTexture)
							sprite = SpriteRendererComponent(Ref<Texture2D>());

						if (sprite.UsesTexture) {
							char path[64];
							memset(path, 0, sizeof(path));
							ImGui::PopItemWidth();
							ImGui::InputText("##sprite_texture_path", path, sizeof(path), ImGuiInputTextFlags_ReadOnly);
							ImGui::SameLine();
							if (ImGui::Button("..."))
							{
								std::string path;
								if (FileDialog::Open(path)) {
									sprite.Texture = mScene->GetTextureLibrary().Get(path);
								}
							}
							ImGui::PushItemWidth(-1.0f);
						}
						else
							ImGui::ColorEdit4("##sprite_color", ValuePtr(sprite.Color));

						ImGui::PopItemWidth();
						ImGui::Columns(1);
						ImGui::TreePop();
					}
					ImGui::Separator();
				}
			}

			// Mesh Component
			{
				if (entity.HasComponent<MeshComponent>()) {
					bool open = ImGui::TreeNodeEx(typeid(SpriteRendererComponent).name(), 0, "Mesh");

					if (ImGui::IsItemClicked(1)) {
						deleteFn = [&]() {
							Entity ent(mSelectedEntity, mScene);
							ent.RemoveComponent<MeshComponent>();
						};
						ImGui::OpenPopup("remove_component");
					}

					Mesh& mesh = entity.GetComponent<MeshComponent>().Mesh;

					const char* wanted = "Empty";

					if (mesh.Type == MeshType::Cube) wanted = "Cube";
					
					if (open) {
						if (ImGui::BeginCombo("Type", wanted)) {
							if (ImGui::Selectable("Cube", wanted == "Cube"))
								wanted = "Cube";
							ImGui::EndCombo();
						}
						if (wanted == "Cube" && mesh.Type != MeshType::Cube)
							mesh = Mesh::CreateCube();

						ImGui::TreePop();
					}
					
					ImGui::Separator();
				}
			}

			// Add Component Button
			{
				if (ImGui::Button("Add Component"))
					ImGui::OpenPopup("add_component");

				if (ImGui::BeginPopup("add_component")) {
					if (!entity.HasComponent<CameraComponent>()) {
						if (ImGui::Button("Camera")) {
							entity.AddComponent<CameraComponent>(Camera::CreatePerspective(ToRadians(45.0f), 0.1f, 100.0f));
							ImGui::CloseCurrentPopup();
						}
					}
					if (!entity.HasComponent<SpriteRendererComponent>()) {
						if (ImGui::Button("Sprite Renderer")) {
							entity.AddComponent<SpriteRendererComponent>();
							ImGui::CloseCurrentPopup();
						}
					}
					if (!entity.HasComponent<MeshComponent>()) {
						if (ImGui::Button("Mesh")) {
							entity.AddComponent<MeshComponent>();
							ImGui::CloseCurrentPopup();
						}
					}
					ImGui::EndPopup();
				}
			}

			// Remove Component Popup
			{
				if (ImGui::BeginPopup("remove_component")) {
					if (ImGui::Button("Remove")) {
						deleteFn();
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();
		
	}

}