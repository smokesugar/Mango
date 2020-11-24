#pragma once

#include "Mango.h"

namespace Mango {
	
	class ModelLibraryPanel {
	public:
		ModelLibraryPanel();
		void SetScene(Scene* scene) { mScene = scene; }

		void OnImGuiRender();
	private:
		void CreateIcon(Mesh* mesh);
		void RenderNode(const Node& node, const xmmatrix& parentTransform);
		void DeleteMesh(size_t index);
	private:
		Scene* mScene = nullptr;
		std::unordered_map<Mesh*, Ref<Texture>> mIcons;
		Ref<DepthBuffer> mDepthBuffer;
		Ref<Shader> mShader;
		Ref<UniformBuffer> mTransformBuffer;
		xmmatrix mViewProjection;
		Mesh* mMeshNameBeingEdited = nullptr;
	};

}