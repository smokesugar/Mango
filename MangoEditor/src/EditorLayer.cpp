#include "EditorLayer.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include "Panels/Dockspace.h"

namespace Mango {

	EditorLayer::EditorLayer()
	{
		mScene = CreateRef<Scene>();
		mSceneHierarchy.SetScene(mScene.get());

		FramebufferProperties props;
		props.Width = Application::Get().GetWindow().GetWidth();
		props.Height = Application::Get().GetWindow().GetHeight();
		props.Depth = true;
		mFramebuffer = Ref<Framebuffer>(Framebuffer::Create(props));

		mTexture = Ref<Texture2D>(Texture2D::Create("assets/textures/Mango.png"));
	}

	inline void EditorLayer::OnUpdate(float dt) {
		Window& window = Application::Get().GetWindow();
		auto buf = window.GetSwapChain().GetFramebuffer();

		mFramebuffer->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);

		mFramebuffer->Bind();
		mFramebuffer->Clear(float4(0.1f, 0.1f, 0.1f, 1.0f));

		mScene->SetAspectRatio(mViewportSize.x / mViewportSize.y);
		mScene->OnUpdate(dt);
	}

	void EditorLayer::OnImGuiRender()
	{
		ImGuizmo::BeginFrame();

		Dockspace::Begin();

		// Menu bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open")) {
					std::string path;
					if (FileDialog::Open(path)) {
						mScene = DataManager::DeserializeScene(path);
						mSceneHierarchy.SetScene(mScene.get());
					}
				}

				if (ImGui::MenuItem("Save As")) {
					std::string path;
					if (FileDialog::Save(path))
						DataManager::SerializeScene(mScene, path);
				}

				if (ImGui::MenuItem("Exit"))
					Mango::Application::Get().Close();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::Begin("Viewport");
		ImVec2 size = ImGui::GetContentRegionAvail();
		mViewportSize = *(float2*)&size;
		ImGui::Image(mFramebuffer->GetTextureAttachment(), size);
		DrawGizmo();
		ImGui::End();
		ImGui::PopStyleVar();

		// Scene Hierarchy
		mSceneHierarchy.OnImGuiRender();

		Dockspace::End();
	}

	void EditorLayer::DrawGizmo()
	{
		ECS::Entity selectedEntity = mSceneHierarchy.GetSelectedEntity();

		if (mScene->GetRegistry().Valid(selectedEntity))
		{
			Camera* activeCamera = nullptr;
			xmmatrix* cameraTransform;
			ECS::Entity cameraEntity = ECS::Null;
			auto query = mScene->GetRegistry().QueryE<CameraComponent, TransformComponent>();
			for (auto& [size, entities, cameras, transforms] : query)
			{
				for (size_t i = 0; i < size; i++) {
					if (activeCamera || !cameras[i].Primary) continue;
					cameraEntity = entities[i];
					activeCamera = cameras[i].Camera.get();
					cameraTransform = &transforms[i].Transform;
				}
			}

			if (activeCamera && cameraEntity != selectedEntity)
			{
				float4x4 view; XMStoreFloat4x4(&view, XMMatrixInverse(nullptr, *cameraTransform));
				float4x4 proj; XMStoreFloat4x4(&proj, activeCamera->GetProjectionMatrix());

				xmmatrix* xmmodel = &mScene->GetRegistry().Get<TransformComponent>(selectedEntity).Transform;
				float4x4 model; XMStoreFloat4x4(&model, *xmmodel);

				auto pos = ImGui::GetWindowPos();
				ImGuizmo::SetOrthographic(activeCamera->GetType() == Camera::Type::Orthographic);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
				if(ImGuizmo::Manipulate(ValuePtr(view), ValuePtr(proj), ImGuizmo::TRANSLATE, ImGuizmo::WORLD, ValuePtr(model), nullptr, nullptr, nullptr, nullptr))
					*xmmodel = XMLoadFloat4x4(&model);
			}
		}
	}

}