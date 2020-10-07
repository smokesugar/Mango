#pragma once

#include <DirectXMath.h>

#define ValuePtr(x) (float*)&x

namespace Mango {
	
	using float2 = DirectX::XMFLOAT2;
	using float3 = DirectX::XMFLOAT3;
	using float4 = DirectX::XMFLOAT4;
	using float3x3 = DirectX::XMFLOAT3X3;
	using float4x4 = DirectX::XMFLOAT4X4;
	using xmvector = DirectX::XMVECTOR;
	using xmmatrx = DirectX::XMMATRIX;

}