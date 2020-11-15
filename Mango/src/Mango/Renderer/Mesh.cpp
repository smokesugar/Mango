#include "mgpch.h"
#include "Mesh.h"

namespace Mango {

	void MeshLibrary::Push(const std::string& name, const Ref<Mesh>& mesh)
	{
		mMeshes.push_back({ name, mesh });
	}

}