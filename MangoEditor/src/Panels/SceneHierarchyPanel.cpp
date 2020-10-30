#include "SceneHierarchyPanel.h"

#include <queue>

#include "Mango/Core/FileDialog.h"
#include "Mango/Scene/Components.h"

#include <imgui.h>

namespace Mango {

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
					if (ImGui::MenuItem("Delete"))
						remove = true;
					ImGui::EndPopup();
				}

				if (open)
				{
					ImGui::TreePop();
				}

				if (remove)
					reg.Destroy(id);
			}
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0))
			mSelectedEntity = ECS::Null;

		if (ImGui::BeginPopupContextWindow(0, 1, false)) {
			if (ImGui::MenuItem("Create Empty"))
				mScene->Create();
			ImGui::EndPopup();
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
					ImGui::Text("Rotation");
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
					ImGui::Text("FOV");
					ImGui::Text("Near Plane");
					ImGui::Text("Far Plane");
				}
				if (orthographic) {
					ImGui::Text("Size");
				}

				ImGui::Text("Enabled");
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
					if (sprite.Texture)
						memcpy(path, sprite.Texture->GetPath().c_str(), Min(sizeof(path), sprite.Texture->GetPath().size()));
					ImGui::PopItemWidth();
					ImGui::InputText("##sprite_texture_path", path, sizeof(path), ImGuiInputTextFlags_ReadOnly);
					ImGui::SameLine();
					if (ImGui::Button("..."))
					{
						std::string path;
						if (FileDialog::Open(path, L"Image File\0*.jpg;*.png;*.bmp\0All\0*.*\0")) {
							sprite.Texture = mScene->GetTextureLibrary().Get(path);
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
				
				ImGui::Columns(2);
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Type");
				if (comp.Type == MeshType::Model)
					ImGui::Text("Path");

				ImGui::NextColumn();
				ImGui::PushItemWidth(-1.0f);

				const char* wanted = "Empty";
				if (comp.Type == MeshType::Cube) wanted = "Cube";
				if (comp.Type == MeshType::Sphere) wanted = "Sphere";
				if (comp.Type == MeshType::Capsule) wanted = "Capsule";
				if (comp.Type == MeshType::Model) wanted = "Model";

				if (ImGui::BeginCombo("##mesh_type", wanted)) {
					if (ImGui::Selectable("Cube", wanted == "Cube"))
						wanted = "Cube";
					if (ImGui::Selectable("Sphere", wanted == "Sphere"))
						wanted = "Sphere";
					if (ImGui::Selectable("Capsule", wanted == "Capsule"))
						wanted = "Capsule";
					if (ImGui::Selectable("Model", wanted == "Model"))
						wanted = "Model";
					ImGui::EndCombo();
				}
				if (wanted == "Cube" && comp.Type != MeshType::Cube)
					comp = MeshComponent(Mesh::CreateCube(), MeshType::Cube);
				if (wanted == "Sphere" && comp.Type != MeshType::Sphere)
					comp = MeshComponent(Mesh::CreateSphere(), MeshType::Sphere);
				if (wanted == "Capsule" && comp.Type != MeshType::Capsule)
					comp = MeshComponent(Mesh::CreateCapsule(), MeshType::Capsule);
				if (wanted == "Model" && comp.Type != MeshType::Model)
					comp = MeshComponent(Mesh(), MeshType::Model);

				if (comp.Type == MeshType::Model) {
					char buf[64];
					memset(buf, 0, sizeof(buf));
					memcpy(buf, comp.Path.c_str(), Min(comp.Path.size(), sizeof(buf)));
					ImGui::PopItemWidth();
					ImGui::InputText("##sprite_texture_path", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
					ImGui::SameLine();
					if (ImGui::Button("...")) {
						std::string path;
						if (FileDialog::Open(path, L"3D Model\0*.obj;*.fbx;*.gltf\0All\0*.*\0")) {
							comp = MeshComponent(Mesh::CreateModel(path), MeshType::Model);
							comp.Path = path;
						}
					}
					ImGui::PushItemWidth(-1.0f);
				}

				ImGui::Columns(1);
				ImGui::PopItemWidth();
			});

			// Add Component Button
			{
				if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvailWidth(), 0.0f)))
					ImGui::OpenPopup("add_component");

				if (ImGui::BeginPopup("add_component")) {
					if (!reg.Has<CameraComponent>(mSelectedEntity)) {
						if (ImGui::MenuItem("Camera"))
							reg.Emplace<CameraComponent>(mSelectedEntity, Camera::CreatePerspective(ToRadians(45.0f), 0.1f, 100.0f));
					}
					if (!reg.Has<SpriteRendererComponent>(mSelectedEntity)) {
						if (ImGui::MenuItem("Sprite Renderer"))
							reg.Emplace<SpriteRendererComponent>(mSelectedEntity);
					}
					if (!reg.Has<MeshComponent>(mSelectedEntity)) {
						if (ImGui::MenuItem("Mesh"))
							reg.Emplace<MeshComponent>(mSelectedEntity);
					}
					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();
		
	}

}