#include "mgpch.h"
#include "DirectXBuffer.h"

#include "Mango/Core/Application.h"
#include "DirectXContext.h"

namespace Mango {

	VertexBuffer* VertexBuffer::Create(void* data, size_t count, uint32_t stride) {
		return new DirectXVertexBuffer(data, count, stride);
	}

	DirectXVertexBuffer::DirectXVertexBuffer(void* data, size_t count, uint32_t stride)
		: mStride(stride), VertexBuffer(count)
	{
		auto& context = RetrieveContext();

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = (uint32_t)count * stride;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = stride;

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = data;

		HR_CALL(context.GetDevice()->CreateBuffer(&desc, &sd, &mInternal));
	}

	void DirectXVertexBuffer::Bind() const
	{
		auto& context = RetrieveContext();
		uint32_t offsets = 0;

		VOID_CALL(context.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		VOID_CALL(context.GetDeviceContext()->IASetVertexBuffers(0, 1, mInternal.GetAddressOf(), &mStride, &offsets));
	}

	IndexBuffer* IndexBuffer::Create(uint16_t* data, size_t count) {
		return new DirectXIndexBuffer(data, count);
	}

	DirectXIndexBuffer::DirectXIndexBuffer(uint16_t* data, size_t count)
		: IndexBuffer(count)
	{
		auto& context = RetrieveContext();

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = (uint32_t)count * sizeof(uint16_t);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = sizeof(uint16_t);

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = data;

		HR_CALL(context.GetDevice()->CreateBuffer(&desc, &sd, &mInternal));
	}

	void DirectXIndexBuffer::Bind() const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->IASetIndexBuffer(mInternal.Get(), DXGI_FORMAT_R16_UINT, 0));
	}

	UniformBuffer* UniformBuffer::InternalCreate(size_t sizeBytes) {
		return new DirectXUniformBuffer(sizeBytes);
	}

	DirectXUniformBuffer::DirectXUniformBuffer(size_t sizeBytes)
	{
		auto& context = RetrieveContext();

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = (uint32_t)sizeBytes;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = (uint32_t)sizeBytes;

		HR_CALL(context.GetDevice()->CreateBuffer(&desc, nullptr, &mInternal));
	}

	void DirectXUniformBuffer::InternalSetData(void* data, size_t sizeBytes)
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->UpdateSubresource(mInternal.Get(), 0, nullptr, data, (uint32_t)sizeBytes, 0));
	}

	void DirectXUniformBuffer::VSBind(size_t slot) const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->VSSetConstantBuffers((uint32_t)slot, 1, mInternal.GetAddressOf()));
	}

	void DirectXUniformBuffer::PSBind(size_t slot) const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->PSSetConstantBuffers((uint32_t)slot, 1, mInternal.GetAddressOf()));
	}

}