#include "ModelLibraryPanel.h"

#include <imgui.h>

#define WIDGET_SIZE 78
#define ICON_SIZE 64

namespace Mango {

	struct TransformBuffer {
		xmmatrix Transform;
		xmmatrix MVP;
	};

	ModelLibraryPanel::ModelLibraryPanel()
	{
		mShader = Ref<Shader>(Shader::Create("assets/shaders/MeshIcon_vs.cso", "assets/shaders/MeshIcon_ps.cso"));
		mDepthBuffer = Ref<DepthBuffer>(DepthBuffer::Create(ICON_SIZE, ICON_SIZE));
		mTransformBuffer = Ref<UniformBuffer>(UniformBuffer::Create<TransformBuffer>());
	}

	void ModelLibraryPanel::OnImGuiRender()
	{
		ImGui::Begin("Model Library");
		auto& meshLib = mScene->GetMeshLibrary();

		if (ImGui::BeginPopupContextWindow()) {
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
				if (FileDialog::Open(path, L"3D Model\0*.gltf;*.obj;*.fbx;*.dae\0All*.*\0"))
					meshLib.Push("Unnamed Mesh", Mesh::CreateModel({}, mScene->GetTextureLibrary(), path));
			}
			ImGui::EndPopup();
		}

		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() || (mMeshNameBeingEdited && ImGui::IsKeyPressed(ImGuiKey_Enter, false))) {
			mMeshNameBeingEdited = nullptr;
		}

		int columns = (int)(ImGui::GetContentRegionAvailWidth() / (float)WIDGET_SIZE);
		columns = Max(columns, 1);

		std::queue<int> deletionQueue;

		ImGui::Columns(columns, 0, false);
		for (int i = 0; i < meshLib.Size(); i++) {
			auto& [name, mesh] = meshLib[i];

			if (mIcons.find(mesh.get()) == mIcons.end())
				CreateIcon(mesh.get());

			auto& texture = mIcons[mesh.get()];
			
			ImGui::SetCursorPos({ ImGui::GetCursorPos().x +(float)WIDGET_SIZE / 2.0f - (float)ICON_SIZE / 2.0f , ImGui::GetCursorPos().y });
			ImGui::Image(texture->GetNativeTexture(), { (float)texture->GetWidth(), (float)texture->GetHeight() });
			if (ImGui::BeginPopupContextItem((const char*)mesh.get())) {
				if (ImGui::MenuItem("Delete")) {
					deletionQueue.push(i);
				}
				ImGui::EndPopup();
			}
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
				ImGui::SetDragDropPayload("payload_meshindex", &i, sizeof(i), ImGuiCond_Once);
				ImGui::Image(texture->GetNativeTexture(), { (float)texture->GetWidth(), (float)texture->GetHeight() });
				ImGui::EndDragDropSource();
			}

			char buf[64];
			memset(buf, 0, sizeof(buf));
			memcpy(buf, name.c_str(), Min(sizeof(buf), name.size()));
			
			if (mMeshNameBeingEdited == mesh.get())
			{
				float width = ImGui::CalcItemWidth();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX()+WIDGET_SIZE/2-width/2);
				if (ImGui::InputText("##editing_mesh", buf, sizeof(buf), ImGuiInputTextFlags_AutoSelectAll))
					name = buf;
			}
			else
			{
				float textWidth = ImGui::CalcTextSize(buf).x;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (float)WIDGET_SIZE / 2 - textWidth / 2);
				ImGui::Text(buf);
				if (ImGui::IsItemClicked()) {
					mMeshNameBeingEdited = mesh.get();
				}
			}

			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		while (!deletionQueue.empty()) {
			DeleteMesh(deletionQueue.front());
			deletionQueue.pop();
		}

		ImGui::End();
	}

	void ModelLibraryPanel::RenderNode(const Node& node, const xmmatrix& parentTransform) {
		xmmatrix transform = node.Transform * parentTransform;
		mTransformBuffer->SetData<TransformBuffer>({transform, transform*mViewProjection});
		for (auto& [mesh, mat] : node.Submeshes) {
			mesh->Bind();
			if (mesh->IsIndexed())
				RenderCommand::DrawIndexed(mesh->GetDrawCount(), 0);
			else
				RenderCommand::Draw(mesh->GetDrawCount(), 0);
		}
		for (auto& child : node.Children)
			RenderNode(child, transform);
	}

	void ModelLibraryPanel::DeleteMesh(size_t index)
	{
		mIcons.erase(mScene->GetMeshLibrary()[index].second.get());
		mScene->GetMeshLibrary().Pop(index, mScene->GetRegistry());
	}

	void ModelLibraryPanel::CreateIcon(Mesh* mesh)
	{
		mIcons[mesh] = Ref<Texture>(Texture::Create(nullptr, ICON_SIZE, ICON_SIZE, Format::RGBA8_UNORM, Texture_RenderTarget));
		BindRenderTargets({ mIcons[mesh] }, mDepthBuffer);
		mIcons[mesh]->Clear(float4(0.5f, 0.5f, 0.5f, 1.0f));
		mDepthBuffer->Clear(0.0f);

		mTransformBuffer->VSBind(0);
		mShader->Bind();

		float minX = mesh->AABB.Min.x;
		float maxX = mesh->AABB.Max.x;
		float minY = mesh->AABB.Min.y;
		float maxY = mesh->AABB.Max.y;
		float minZ = mesh->AABB.Min.z;
		float maxZ = mesh->AABB.Max.z;

		float mx = minX + maxX;
		float my = minY + maxY;
		float mz = minZ + maxZ;

		if (mx == 0.0f && my == 0.0f && mz == 0.0f) return;

		float3 aabbCenter = float3(mx/2.0f, my/2.0f, mz/2.0f);

		float dx = minX - maxX;
		float dy = minY - maxY;
		float dz = minZ - maxZ;
		float boundSphereRadius = sqrtf(dx * dx + dy * dy + dz * dz)/2.0f;

		float fov = ToRadians(45.0f);
		float camDistance = (boundSphereRadius) / tanf(fov / 2.0f);

		xmmatrix view = XMMatrixLookAtLH({ aabbCenter.x, aabbCenter.y, aabbCenter.z - camDistance }, XMLoadFloat3(&aabbCenter), {0.0f, 1.0f, 0.0f});
		xmmatrix projection = XMMatrixPerspectiveFovLH(fov, 1.0f, camDistance+boundSphereRadius, 0.1f);

		mViewProjection = view * projection;
		RenderNode(mesh->RootNode, XMMatrixIdentity());
	}

}
