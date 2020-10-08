#pragma once

#include "Buffer.h"

namespace Mango {
	
	class VertexArray {
	public:
		VertexArray(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib);
		void Bind() const;
		inline bool IsIndexed() const { return mBuffers.size() == 2; };
		inline size_t GetDrawCount() const { return mDrawCount; }
	private:
		size_t mDrawCount = 0;
		std::vector<Ref<DrawableBuffer>> mBuffers;
	};

}