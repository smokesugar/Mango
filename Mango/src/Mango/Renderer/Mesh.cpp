#include "mgpch.h"
#include "Mesh.h"

#include "Mango/Scene/Components.h"

namespace Mango {

	void MeshLibrary::Push(const std::string& name, const Ref<Mesh>& mesh)
	{
		mMeshes.push_back({ name, mesh });
	}

	void MeshLibrary::Pop(size_t index, ECS::Registry& reg)
	{
		mMeshes[index] = mMeshes[mMeshes.size() - 1];
		
		auto query = reg.Query<MeshComponent>();
		for (auto& [size, meshes] : query) {
			for (size_t i = 0; i < size; i++) {
				auto& meshComp = meshes[i];
				if (meshComp.MeshIndex == index)
					meshComp.MeshIndex = -1;
				if (meshComp.MeshIndex == mMeshes.size() - 1)
					meshComp.MeshIndex = index;
			}
		}

		mMeshes.pop_back();
	}

}