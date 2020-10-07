#pragma once

namespace Mango {
	
	class VertexBuffer {
	public:
		virtual ~VertexBuffer() {}
		virtual void Bind() const = 0;

		static VertexBuffer* Create(void* data, size_t count, uint32_t stride);
	};

	class IndexBuffer {
	public:
		virtual ~IndexBuffer() {}
		virtual void Bind() const = 0;

		static IndexBuffer* Create(uint16_t* data, size_t count);
	};

}