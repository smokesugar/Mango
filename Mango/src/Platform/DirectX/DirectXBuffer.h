#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <cinttypes>

#include "Mango/Renderer/Buffer.h"

namespace Mango {
	
	class DirectXVertexBuffer : public VertexBuffer {
	public:
		DirectXVertexBuffer(void* data, size_t count, uint32_t stride);
		virtual void Bind() const override;
	private:
		uint32_t mStride;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mInternal;
	};

	class DirectXIndexBuffer : public IndexBuffer {
	public:
		DirectXIndexBuffer(uint16_t* data, size_t count);
		virtual void Bind() const override;
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> mInternal;
	};

	class DirectXUniformBuffer : public UniformBuffer {
	public:
		DirectXUniformBuffer(size_t sizeBytes);

		virtual void InternalSetData(void* data, size_t sizeBytes) override;

		virtual void VSBind(size_t slot) const override;
		virtual void PSBind(size_t slot) const override;
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> mInternal;
	};

}