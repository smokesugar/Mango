#include "EditorLayer.h"

#include "Panels/Dockspace.h"
#include "ViewportInteraction.h"

namespace Mango {

	EditorLayer::EditorLayer()
	{
		mScene = CreateRef<Scene>();

		mSceneHierarchyPanel.SetScene(mScene.get());
		mModelLibraryPanel.SetScene(mScene.get());

		mFramebuffer = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA16_FLOAT, Texture_RenderTarget));
		ViewportInteraction::Init();
	}

	EditorLayer::~EditorLayer()
	{
		ViewportInteraction::Shutdown();
	}

	inline void EditorLayer::OnUpdate(float dt) {
		mFPS = 1.0f / dt;

		mFramebuffer->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
		mFramebuffer->Clear(RENDERER_CLEAR_COLOR);
		if(mScene->IsPlaying())
			mScene->OnUpdate(dt, mFramebuffer);
		else
			mScene->OnUpdate(dt, mFramebuffer, mEditorCamera.GetProjectionMatrix(mViewportSize.x/mViewportSize.y), mEditorCamera.GetTransform());

		if (!mScene->IsPlaying() && mScene->GetRegistry().Valid(mSceneHierarchyPanel.GetSelectedEntity())) {
			xmmatrix viewProjection = XMMatrixInverse(nullptr, mEditorCamera.GetTransform()) * mEditorCamera.GetProjectionMatrix(mViewportSize.x / mViewportSize.y);
			ViewportInteraction::RenderSelectionOutline(mScene, mSceneHierarchyPanel.GetSelectedEntity(), viewProjection, mFramebuffer);
		}
	}

	void EditorLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyDownEvent>(MG_BIND_FN(EditorLayer::OnKeyDown));
		dispatcher.Dispatch<MouseButtonDownEvent>(MG_BIND_FN(EditorLayer::OnMouseButtonDown));
		mEditorCamera.OnEvent(e);
	}

	void EditorLayer::OnImGuiRender()
	{
		Dockspace::Begin();

		ImGui::Begin("Runtime");
		if (ImGui::Button(!mScene->IsPlaying() ? "Play" : "Stop", ImVec2(30, 30))) {
			if (mScene->IsPlaying())
				mScene->Stop();
			else
				mScene->Start();
		}
		ImGui::End();

		ImGui::Begin("Performance");
		ImGui::Text("FPS: %f", mFPS);
		ImGui::End();

		ImGui::Begin("Scene Settings");
		ImGui::DragFloat("Environment Strength", &Renderer::EnvironmentStrength(), 0.01f, 0.0f, 100.0f);
		if (ImGui::Button("Select HDRi")) {
			std::string path;
			if (FileDialog::Open(path, L"HDR Texture\0*.hdr\0All\0*.*\0")) {
				Ref<Cubemap> newHdri = Ref<Cubemap>(Cubemap::Create(path, SKYBOX_RESOLUTION));
				Renderer::InitializeCubemap(newHdri);
				Renderer::SetSkybox(newHdri);
			}
		}
		ImGui::End();

		// Menu bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open")) {
					std::string path;
					if (FileDialog::Open(path, L"Mango Scene\0*.json;*.mango\0All\0*.*\0")) {
						mScene = Serializer::DeserializeScene(path);
						mSceneHierarchyPanel.SetScene(mScene.get());
						mModelLibraryPanel.SetScene(mScene.get());
					}
				}

				if (ImGui::MenuItem("Save As")) {
					std::string path;
					if (FileDialog::Save(path, L"Mango Scene\0*.json;*.mango\0All\0*.*\0"))
						Serializer::SerializeScene(mScene, path);
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
		mViewportFocused = ImGui::IsWindowFocused();
		mViewportHovered = ImGui::IsWindowHovered();
		mViewportMousePosition = *(float2*)&(ImGui::GetMousePos());
		mViewportMousePosition.x -= ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMin().x;
		mViewportMousePosition.y -= ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMin().y;
		ImVec2 size = ImGui::GetContentRegionAvail();
		mViewportSize = *(float2*)&size;
		ImGui::Image(mFramebuffer->GetNativeTexture(), size);
		mEditorCamera.SetAcceptingInput(mViewportHovered);
		
		bool entitySelected = mScene->GetRegistry().Valid(mSceneHierarchyPanel.GetSelectedEntity());

		if (entitySelected && !mScene->IsPlaying())
		{
			ImGuizmo::BeginFrame();
			ImVec2 pos = ImGui::GetWindowPos();
			ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);
			ImGuizmo::SetDrawlist();

			auto& transformComp = mScene->GetRegistry().Get<TransformComponent>(mSceneHierarchyPanel.GetSelectedEntity());

			float4x4 view; XMStoreFloat4x4(&view, XMMatrixInverse(nullptr, mEditorCamera.GetTransform()));
			float4x4 proj; XMStoreFloat4x4(&proj, mEditorCamera.GetProjectionMatrixNotInverted(size.x/size.y));
			float4x4 transform; XMStoreFloat4x4(&transform, transformComp.GetMatrix());

			mGizmoHovered = ImGuizmo::IsOver();
			if (ImGuizmo::Manipulate(ValuePtr(view), ValuePtr(proj), mGizmoOperation, ImGuizmo::LOCAL, ValuePtr(transform))) {
				DecomposeMatrix(&transformComp.Translation, &transformComp.Rotation, &transformComp.Scale, XMLoadFloat4x4(&transform));
			}
		}

		ImGui::PopStyleVar();

		static bool selectedEntityHovered = ECS::Null;
		if (!mScene->IsPlaying() && mViewportHovered && ImGui::IsMouseClicked(1)) {
			xmmatrix viewProjection = XMMatrixInverse(nullptr, mEditorCamera.GetTransform()) * mEditorCamera.GetProjectionMatrix(mViewportSize.x / mViewportSize.y);
			ECS::Entity hoveredEntity = ViewportInteraction::GetHoveredEntity(mScene, mViewportSize, mViewportMousePosition, viewProjection);
			selectedEntityHovered = mSceneHierarchyPanel.GetSelectedEntity() == hoveredEntity;
			ImGui::OpenPopup("viewport_context_menu");
		}

		if (ImGui::BeginPopup("viewport_context_menu")) {
			if (selectedEntityHovered) {
				if (ImGui::MenuItem("Delete Entity"))
					mScene->GetRegistry().Destroy(mSceneHierarchyPanel.GetSelectedEntity());
			}
			else
			{
				if (ImGui::MenuItem("Create Empty"))
					mSceneHierarchyPanel.SetSelectedEntity(mScene->Create());
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Log");
		if (ImGui::Button("Clear")) { Application::Get().GetRuntimeLog().Clear(); }
		ImGui::Separator();
		ImGui::BeginChild("scroll panel");
		ImGui::TextUnformatted(Application::Get().GetRuntimeLog().GetBuffer());
		if (Application::Get().GetRuntimeLog().HandleScroll())
			ImGui::SetScrollHere(1.0f);
		ImGui::EndChild();
		ImGui::End();
		
		mSceneHierarchyPanel.OnImGuiRender();
		mModelLibraryPanel.OnImGuiRender();

		Dockspace::End();
	}

	bool EditorLayer::OnKeyDown(KeyDownEvent& e)
	{
		if (mViewportHovered || mViewportFocused) {
			if (e.GetKeycode() == KeyCode::G)
				mGizmoOperation = ImGuizmo::TRANSLATE;
			if (e.GetKeycode() == KeyCode::R)
				mGizmoOperation = ImGuizmo::ROTATE;
			if (e.GetKeycode() == KeyCode::S)
				mGizmoOperation = ImGuizmo::SCALE;
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonDown(MouseButtonDownEvent& e)
	{
		if (e.GetButton() == MouseCode::LBUTTON && !mScene->IsPlaying() && mViewportHovered && !mGizmoHovered) {
			xmmatrix viewProjection = XMMatrixInverse(nullptr, mEditorCamera.GetTransform()) * mEditorCamera.GetProjectionMatrix(mViewportSize.x / mViewportSize.y);
			ECS::Entity hoveredEntity = ViewportInteraction::GetHoveredEntity(mScene, mViewportSize, mViewportMousePosition, viewProjection);
			mSceneHierarchyPanel.SetSelectedEntity(hoveredEntity);
		}
		return false;
	}

}