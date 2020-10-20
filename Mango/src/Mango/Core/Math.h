#pragma once

#include <DirectXMath.h>

using namespace DirectX;

#define ValuePtr(x) (float*)&x

#define PI 3.14159265359f
#define TAU 6.28318530718f

namespace Mango {
	
	using float2 = DirectX::XMFLOAT2;
	using float3 = DirectX::XMFLOAT3;
	using float4 = DirectX::XMFLOAT4;
	using float3x3 = DirectX::XMFLOAT3X3;
	using float4x4 = DirectX::XMFLOAT4X4;
	using xmvector = DirectX::XMVECTOR;
	using xmmatrix = DirectX::XMMATRIX;

	template<typename T>
	T Lerp(T a, T b, T f)
	{
		return a + f * (b - a);
	}

	template<typename T>
	static T Max(T a, T b) {
		return a > b ? a : b;
	}

	template<typename T>
	static T Min(T a, T b) {
		return a < b ? a : b;
	}

	static float3 Max(const float3& a, const float3& b) {
		float3 o;
		o.x = Max(a.x, b.x);
		o.y = Max(a.y, b.y);
		o.z = Max(a.z, b.z);
		return o;
	}

	static float3 Min(const float3& a, const float3& b) {
		float3 o;
		o.x = Min(a.x, b.x);
		o.y = Min(a.y, b.y);
		o.z = Min(a.z, b.z);
		return o;
	}

	template<typename T>
	static T ToRadians(T degrees) {
		return degrees * PI/180.0f;
	}

	template<typename T>
	static T ToDegrees(T rads) {
		return rads * 180.0f / PI;
	}


	static void DecomposeMatrix(float3* translation, float3* rotation, float3* scale, const xmmatrix& matrix)
	{
		float4x4 mat;
		XMStoreFloat4x4(&mat, matrix);

		float sx = sqrtf(pow(mat.m[0][0], 2) + pow(mat.m[0][1], 2) + pow(mat.m[0][2], 2));
		float sy = sqrtf(pow(mat.m[1][0], 2) + pow(mat.m[1][1], 2) + pow(mat.m[1][2], 2));
		float sz = sqrtf(pow(mat.m[2][0], 2) + pow(mat.m[2][1], 2) + pow(mat.m[2][2], 2));

		mat.m[0][0] /= sx;
		mat.m[0][1] /= sx;
		mat.m[0][2] /= sx;

		mat.m[1][0] /= sy;
		mat.m[1][1] /= sy;
		mat.m[1][2] /= sy;

		mat.m[2][0] /= sz;
		mat.m[2][1] /= sz;
		mat.m[2][2] /= sz;

		if (scale) {
			scale->x = sx;
			scale->y = sy;
			scale->z = sz;
		}

		if (rotation) {
			rotation->x = ToDegrees(atan2f(mat.m[1][2], mat.m[2][2]));
			rotation->y = ToDegrees(atan2f(-mat.m[0][2], sqrtf(mat.m[1][2] * mat.m[1][2] + mat.m[2][2] * mat.m[2][2])));
			rotation->z = ToDegrees(atan2f(mat.m[0][1], mat.m[0][0]));
		}

		if (translation) {
			translation->x = mat.m[3][0];
			translation->y = mat.m[3][1];
			translation->z = mat.m[3][2];
		}
	}

	static xmmatrix RecomposeMatrix(const float3& translation, const float3& rotation, const float3& scale)
	{
		xmvector directionUnary[3] = { {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f} };
		
		xmmatrix rot[3];
		float* _rotation = ValuePtr(rotation);
		for (int i = 0; i < 3; i++)
		{
			rot[i] = XMMatrixRotationAxis(directionUnary[i], ToRadians(_rotation[i]));
		}

		xmmatrix rotationMat = rot[0] * rot[1] * rot[2];

		return XMMatrixScaling(scale.x, scale.y, scale.z) * rotationMat * XMMatrixTranslation(translation.x, translation.y, translation.z);
	}

}