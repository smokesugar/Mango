#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "Mango/Renderer/Shader.h"

namespace Mango {
	
	class DirectXShader : public Shader {
	public:
		DirectXShader(const std::string& vertex, const std::string& pixel);
		DirectXShader(const std::string& vertex, const std::string& geometry, const std::string& pixel);
		virtual void Bind() const override;
	private:
		void CreateInputLayout(ID3DBlob* blob);
		DXGI_FORMAT GetDXGIFormat(BYTE mask, D3D_REGISTER_COMPONENT_TYPE type);
	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> mGeometryShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
	};

}