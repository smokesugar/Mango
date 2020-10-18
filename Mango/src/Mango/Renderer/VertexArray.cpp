#include "mgpch.h"
#include "VertexArray.h"

namespace Mango {

	VertexArray::VertexArray(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib)
	{
		MG_CORE_ASSERT(vb, "Vertex Buffer was nullptr.");

		mBuffers.push_back(vb);
		if (ib) {
			mDrawCount = ib->GetDrawCount();
			mBuffers.push_back(ib);
		}
		else {
			mDrawCount = vb->GetDrawCount();
		}
	}

	void VertexArray::Bind() const
	{
		for (auto& buffer : mBuffers)
			buffer->Bind();
	}

}