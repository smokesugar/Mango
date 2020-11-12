#include "EditorLayer.h"

#include "Panels/Dockspace.h"

namespace Mango {

	struct MousePickerData {
		xmmatrix MVP;
		float3 Val;
		float padding;
	};

	struct RGB32 {
		union {
			struct { uint8_t r, g, b; };
			uint32_t raw;
		};
	};

	EditorLayer::EditorLayer()
	{
		mScene = CreateRef<Scene>();

		mSceneHierarchyPanel.SetScene(mScene.get());

		mFramebuffer = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA16_FLOAT, Texture_RenderTarget));
		mMousePickerTexture = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA8_UNORM, Texture_RenderTarget));
		mMousePickerDepthBuffer = Ref<DepthBuffer>(DepthBuffer::Create(800, 600));
		mMousePickerTextureStaging = Ref<Texture>(Texture::Create(nullptr, 800, 600, Format::RGBA8_UNORM, Texture_CPU));
		mMousePickerShader = Ref<Shader>(Shader::Create("assets/shaders/MousePicker_vs.cso", "assets/shaders/MousePicker_ps.cso"));
		mMousePickerUniforms = Ref<UniformBuffer>(UniformBuffer::Create<MousePickerData>());
	}

	inline void EditorLayer::OnUpdate(float dt) {
		mFPS = 1.0f / dt;

		mFramebuffer->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
		mFramebuffer->Clear(RENDERER_CLEAR_COLOR);
		if(mScenePlaying)
			mScene->OnUpdate(dt, mFramebuffer);
		else
			mScene->OnUpdate(dt, mFramebuffer, mEditorCamera.GetProjectionMatrix(mViewportSize.x/mViewportSize.y), mEditorCamera.GetTransform());
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

		static bool seeShit = false;

		ImGui::Begin("Temp Panel");
		bool& b = Renderer::TAAEnabled();
		ImGui::Text("FPS: %f", mFPS);
		ImGui::Checkbox("TAA", &b);
		ImGui::Checkbox("Scene Playing", &mScenePlaying);
		ImGui::Checkbox("See Shit", &seeShit);
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
		ImGui::Image(seeShit ? mMousePickerTexture->GetNativeTexture() : mFramebuffer->GetNativeTexture(), size);
		mEditorCamera.SetAcceptingInput(mViewportHovered);

		if (mScene->GetRegistry().Valid(mSceneHierarchyPanel.GetSelectedEntity()) && !mScenePlaying) {
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

		ImGui::End();
		ImGui::PopStyleVar();

		mSceneHierarchyPanel.OnImGuiRender();

		Dockspace::End();
	}

	bool EditorLayer::OnKeyDown(KeyDownEvent& e)
	{
		if (mViewportFocused) {
			if (e.GetKeycode() == KeyCode::G)
				mGizmoOperation = ImGuizmo::TRANSLATE;
			if (e.GetKeycode() == KeyCode::R)
				mGizmoOperation = ImGuizmo::ROTATE;
			if (e.GetKeycode() == KeyCode::S)
				mGizmoOperation = ImGuizmo::SCALE;
		}

		return false;
	}

	void EditorLayer::RenderNode(const Node& node, const xmmatrix& parentTransform, float3 color) {
		xmmatrix transform = node.Transform * parentTransform;

		for (auto& [va, mat] : node.Submeshes) {
			mMousePickerUniforms->SetData<MousePickerData>({
										transform *
										XMMatrixInverse(nullptr, mEditorCamera.GetTransform()) *
										mEditorCamera.GetProjectionMatrix(mViewportSize.x / mViewportSize.y), color });
			va->Bind();
			if (va->IsIndexed())
				RenderCommand::DrawIndexed(va->GetDrawCount(), 0);
			else
				RenderCommand::Draw(va->GetDrawCount(), 0);
		}

		for (auto& child : node.Children) {
			RenderNode(child, transform, color);
		}
	}

	bool EditorLayer::OnMouseButtonDown(MouseButtonDownEvent& e)
	{
		if (e.GetButton() == MouseCode::LBUTTON && !mScenePlaying && mViewportHovered && !mGizmoHovered) {

			mMousePickerTexture->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
			mMousePickerDepthBuffer->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
			mMousePickerTextureStaging->EnsureSize((uint32_t)mViewportSize.x, (uint32_t)mViewportSize.y);
			BindRenderTargets({ mMousePickerTexture }, mMousePickerDepthBuffer);
			mMousePickerTexture->Clear(float4(0.0f, 0.0f, 0.0f, 1.0f));
			mMousePickerDepthBuffer->Clear(0.0f);
			mMousePickerShader->Bind();
			mMousePickerUniforms->VSBind(0);

			auto& reg = mScene->GetRegistry();

			std::unordered_map<uint32_t, ECS::Entity> valueEntityMap;
			RGB32 nullcolor;
			nullcolor.r = 0;
			nullcolor.g = 0;
			nullcolor.b = 0;
			valueEntityMap[nullcolor.raw] = ECS::Null;
			auto query0 = reg.QueryE<MeshComponent, TransformComponent>();
			float3 col = float3(0.0f, 0.0f, 0.0f);
			float increment = 1.0f / 255.0f;
			for (auto& [size, entities, meshes, transforms] : query0) {
				for (size_t i = 0; i < size; i++) {
					auto& mesh = meshes[i];
					auto& transform = transforms[i];
					
					col.x += increment;
					if (col.x > 1.0f) {
						col.y += increment;
						col.x = 0.0f;
					}
					if (col.y > 1.0f) {
						col.z += increment;
						col.y = 0.0f;
					}
					
					RGB32 colorint;
					colorint.r = (uint8_t)(col.x*255.0f);
					colorint.g = (uint8_t)(col.y*255.0f);
					colorint.b = (uint8_t)(col.z*255.0f);

					valueEntityMap[colorint.raw] = entities[i];
					RenderNode(mesh.Mesh.RootNode, transform.GetMatrix(), col);
				}
			}

			uint8_t color[3];
			mMousePickerTexture->GetData((uint32_t)mViewportMousePosition.x, (uint32_t)mViewportMousePosition.y, mMousePickerTextureStaging, color, sizeof(color));
			
			RGB32 colorint;
			memcpy(&colorint, color, sizeof(color));

			mSceneHierarchyPanel.SetSelectedEntity(valueEntityMap[colorint.raw]);
		}
		return false;
	}

}