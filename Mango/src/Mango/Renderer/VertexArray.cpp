#include "mgpch.h"
#include "VertexArray.h"

namespace Mango {

	VertexArray::VertexArray(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib)
	{
		MG_CORE_ASSERT(vb, "Vertex Buffer was nullptr.");

		mDrawCount = ib ? ib->GetDrawCount() : vb->GetDrawCount();
		mBuffers.push_back(vb);
		mBuffers.push_back(ib);
	}

	void VertexArray::Bind() const
	{
		for (auto& buffer : mBuffers)
			buffer->Bind();
	}

}