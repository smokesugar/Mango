#pragma once

#include "Mango/Core/Base.h"

namespace Mango {

	class DrawableBuffer {
	public:
		virtual void Bind() const = 0;
	};
	
	class VertexBuffer : public DrawableBuffer {
	public:
		VertexBuffer(size_t count)
			: mDrawCount(count) {}
		virtual ~VertexBuffer() {}
		inline size_t GetDrawCount() const { return mDrawCount; }
		static VertexBuffer* Create(void* data, size_t count, uint32_t stride);
	private:
		size_t mDrawCount;
	};

	class IndexBuffer : public DrawableBuffer {
	public:
		IndexBuffer(size_t count)
			: mDrawCount(count) {}
		virtual ~IndexBuffer() {}
		inline size_t GetDrawCount() const { return mDrawCount; }
		static IndexBuffer* Create(uint16_t* data, size_t count);
	private:
		size_t mDrawCount;
	};

	class UniformBuffer {
	public:
		virtual ~UniformBuffer() {}
		
		virtual void VSBind(size_t slot) const = 0;
		virtual void PSBind(size_t slot) const = 0;

		template<typename T>
		void SetData(const T& data) {
			MG_CORE_ASSERT(mBufferType == typeid(T).name(), "You must update the uniform buffer with the type you created it with.");
			InternalSetData((void*)&data, sizeof(T));
		}

		template<typename T>
		static UniformBuffer* Create() {
			UniformBuffer* buffer = InternalCreate(sizeof(T));
			buffer->mBufferType = typeid(T).name();
			return buffer;
		}
	protected:
		virtual void InternalSetData(void* data, size_t sizeBytes) = 0;
	private:
		static UniformBuffer* InternalCreate(size_t sizeBytes);
	private:
		const char* mBufferType = nullptr;
	};

}