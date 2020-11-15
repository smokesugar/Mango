#include "ModelLibraryPanel.h"

#include <imgui.h>

namespace Mango {

	void ModelLibraryPanel::OnImGuiRender()
	{
		ImGui::Begin("Model Library");
		uint32_t counter = 0;
		auto& meshLib = mScene->GetMeshLibrary();
		for (int i = 0; i < meshLib.Size(); i++) {
			auto& [name, mesh] = meshLib[i];
			char buf[64];
			memset(buf, 0, sizeof(buf));
			memcpy(buf, name.c_str(), Min(sizeof(buf), name.size()));
			if (ImGui::InputText(std::string("##mesh_name" + std::to_string(counter)).c_str(), buf, sizeof(buf)))
				name = buf;

			if(ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("meshindex", &i, sizeof(i), ImGuiCond_Once);
				ImGui::Text(buf);
				ImGui::EndDragDropSource();
			}

			counter++;
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
			ImGui::OpenPopup("context_menu");

		if (ImGui::BeginPopup("context_menu")) {
			if (ImGui::MenuItem("Create Cube")) {
				meshLib.Push("Cube Mesh", Mesh::CreateCube(Renderer::CreateDefaultMaterial()));
			}
			if (ImGui::MenuItem("Create Sphere")) {
				meshLib.Push("Sphere Mesh", Mesh::CreateSphere(Renderer::CreateDefaultMaterial()));
			}
			if (ImGui::MenuItem("Create Capsule")) {
				meshLib.Push("Capsule Mesh", Mesh::CreateCapsule(Renderer::CreateDefaultMaterial()));
			}
			if (ImGui::MenuItem("Create Model")) {
				std::string path;
				if(FileDialog::Open(path, L"3D Model\0*.gltf;*.obj;*.fbx;*.dae\0All*.*\0"))
					meshLib.Push("Unnamed Mesh", Mesh::CreateModel({}, mScene->GetTextureLibrary(), path));
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}

}
