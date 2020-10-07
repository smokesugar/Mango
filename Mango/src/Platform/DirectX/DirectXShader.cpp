#include "mgpch.h"
#include "DirectXShader.h"

#include <d3dcompiler.h>

#include "Mango/Core/Application.h"
#include "DirectXContext.h"

namespace Mango {
	
	Shader* Shader::Create(const std::string& vertex, const std::string& pixel) {
		return new DirectXShader(vertex, pixel);
	}

	DirectXShader::DirectXShader(const std::string& vertex, const std::string& pixel)
	{
		auto& context = RetrieveContext();
		Microsoft::WRL::ComPtr<ID3DBlob> blob;

		// Shaders
		if (FAILED(D3DReadFileToBlob(WidenString(pixel).c_str(), &blob)))
			MG_CORE_ASSERT(false, "Failed to load shader '{0}'.", pixel);
		if (FAILED(context.GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mPixelShader)))
			MG_CORE_ASSERT(false, "Failed to load shader '{0}'.", vertex);

		HR_CALL(D3DReadFileToBlob(WidenString(vertex).c_str(), &blob));
		HR_CALL(context.GetDevice()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mVertexShader));

		// Input Layout
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflection;
		HR_CALL(D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, &reflection));

		D3D11_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
		for (uint32_t i = 0; i < shaderDesc.InputParameters; i++) {
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			reflection->GetInputParameterDesc(i, &paramDesc);

			D3D11_INPUT_ELEMENT_DESC elementDesc;
			elementDesc.SemanticName = paramDesc.SemanticName;
			elementDesc.SemanticIndex = paramDesc.SemanticIndex;
			elementDesc.InputSlot = 0;
			elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;
			elementDesc.Format = GetDXGIFormat(paramDesc.Mask, paramDesc.ComponentType);

			ied.push_back(elementDesc);
		}

		HR_CALL(context.GetDevice()->CreateInputLayout(ied.data(), (uint32_t)ied.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &mInputLayout));
	}

	void DirectXShader::Bind() const
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->VSSetShader(mVertexShader.Get(), nullptr, 0));
		VOID_CALL(context.GetDeviceContext()->PSSetShader(mPixelShader.Get(), nullptr, 0));
		VOID_CALL(context.GetDeviceContext()->IASetInputLayout(mInputLayout.Get()));
	}

	DXGI_FORMAT DirectXShader::GetDXGIFormat(BYTE mask, D3D_REGISTER_COMPONENT_TYPE type)
	{
		if (mask == 1)
		{
			     if (type == D3D_REGISTER_COMPONENT_UINT32)  return DXGI_FORMAT_R32_UINT;
			else if (type == D3D_REGISTER_COMPONENT_SINT32)  return DXGI_FORMAT_R32_SINT;
			else if (type == D3D_REGISTER_COMPONENT_FLOAT32) return DXGI_FORMAT_R32_FLOAT;
		}
		else if (mask <= 3)
		{
			     if (type == D3D_REGISTER_COMPONENT_UINT32)  return DXGI_FORMAT_R32G32_UINT;
			else if (type == D3D_REGISTER_COMPONENT_SINT32)  return DXGI_FORMAT_R32G32_SINT;
			else if (type == D3D_REGISTER_COMPONENT_FLOAT32) return DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (mask <= 7)
		{
			if      (type == D3D_REGISTER_COMPONENT_UINT32)  return DXGI_FORMAT_R32G32B32_UINT;
			else if (type == D3D_REGISTER_COMPONENT_SINT32)  return DXGI_FORMAT_R32G32B32_SINT;
			else if (type == D3D_REGISTER_COMPONENT_FLOAT32) return DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (mask <= 15)
		{
				 if (type == D3D_REGISTER_COMPONENT_UINT32)  return DXGI_FORMAT_R32G32B32A32_UINT;
			else if (type == D3D_REGISTER_COMPONENT_SINT32)  return DXGI_FORMAT_R32G32B32A32_SINT;
			else if (type == D3D_REGISTER_COMPONENT_FLOAT32) return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		MG_CORE_ASSERT(false, "Invalid arguments.");
		return DXGI_FORMAT_R32_UINT;
	}

}