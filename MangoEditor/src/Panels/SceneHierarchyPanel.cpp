#include "SceneHierarchyPanel.h"

#include <queue>

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

					if (open) {
						auto& cameraComp = entity.GetComponent<CameraComponent>();
						ImGui::Columns(2);
						ImGui::AlignTextToFramePadding();
						ImGui::Text("Zoom");
						ImGui::Text("Primary");
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1.0f);
						if (cameraComp.Camera->GetType() == Camera::Type::Orthographic) {
							float zoom = std::static_pointer_cast<OrthographicCamera>(cameraComp.Camera)->GetZoom();
							ImGui::DragFloat("##camera_zoom", &zoom, 0.1f, 0.001f, INFINITY);
							std::static_pointer_cast<OrthographicCamera>(cameraComp.Camera)->SetZoom(Max(zoom, 0.001f));
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
					bool open = ImGui::TreeNodeEx(typeid(SpriteRendererComponent).name(), 0, "Sprite");

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
						ImGui::Text("Color");
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1.0f);
						ImGui::ColorEdit4("##sprite_color", ValuePtr(sprite.Color));
						ImGui::PopItemWidth();
						ImGui::Columns(1);
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
							entity.AddComponent<CameraComponent>(CreateRef<OrthographicCamera>());
							ImGui::CloseCurrentPopup();
						}
					}
					if (!entity.HasComponent<SpriteRendererComponent>()) {
						if (ImGui::Button("Sprite")) {
							entity.AddComponent<SpriteRendererComponent>();
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