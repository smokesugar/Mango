#include "mgpch.h"
#include "DirectXAPI.h"

#include "Mango/Core/Application.h"
#include "DirectXContext.h"

namespace Mango {

	RendererAPI* RendererAPI::Create() {
		return new DirectXAPI();
	}

	void DirectXAPI::Draw(size_t count, size_t offset)
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->Draw((uint32_t)count, (uint32_t)offset));
	}

	void DirectXAPI::DrawIndexed(size_t count, size_t offset)
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->DrawIndexed((uint32_t)count, (uint32_t)offset, 0));
	}

	void DirectXAPI::EnableBlending()
	{
		auto& context = RetrieveContext();

		D3D11_BLEND_DESC desc = {};
		desc.RenderTarget[0].BlendEnable = TRUE;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

		Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
		HR_CALL(context.GetDevice()->CreateBlendState(&desc, &blendState));
		VOID_CALL(context.GetDeviceContext()->OMSetBlendState(blendState.Get(), nullptr, 0xffffffff));
	}

	void DirectXAPI::DisableBlending()
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff));
	}

	void DirectXAPI::EnableInvertedDepthTesting()
	{
		auto& context = RetrieveContext();

		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_GREATER;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> dss;
		HR_CALL(context.GetDevice()->CreateDepthStencilState(&desc, &dss));
		VOID_CALL(context.GetDeviceContext()->OMSetDepthStencilState(dss.Get(), 1));
	}

}