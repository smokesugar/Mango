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

	class UniformBuffer {
	public:
		virtual ~UniformBuffer() {}
		
		virtual void VSBind(size_t slot) const = 0;
		virtual void PSBind(size_t slot) const = 0;

		template<typename T>
		void SetData(const T& data) {
			InternalSetData((void*)&data, sizeof(T));
		}

		template<typename T>
		static UniformBuffer* Create() {
			return InternalCreate(sizeof(T));
		}
	protected:
		virtual void InternalSetData(void* data, size_t sizeBytes) = 0;
	private:
		static UniformBuffer* InternalCreate(size_t sizeBytes);
	};

}