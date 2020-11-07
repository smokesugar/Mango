#include "mgpch.h"
#include "CascadedShadowmap.h"

#include "Mango/Core/Log.h"

namespace Mango {

	std::vector<float> CascadedShadowmap::GenerateCascadeDistances(const xmmatrix& projection, uint32_t numCascades)
	{
		xmvector zNear = { 0.0f, 0.0f, 1.0f, 1.0f };
		xmvector zFar = { 0.0f, 0.0f, 0.0f, 1.0f };

		xmmatrix invProj = XMMatrixInverse(nullptr, projection);

		zNear = XMVector4Transform(zNear, invProj);
		zFar = XMVector4Transform(zFar, invProj);
		float nearPlane = XMVectorGetZ(zNear) / XMVectorGetW(zNear);
		float farPlane = XMVectorGetZ(zFar) / XMVectorGetW(zFar);

		std::vector<float> cascadeEnds((size_t)numCascades + 1);
		for (uint32_t i = 0; i < numCascades; i++) {
			float IDM = i / float(numCascades);
			float log = nearPlane * powf(farPlane / nearPlane, IDM);
			float uniform = nearPlane + (farPlane - nearPlane) * IDM;
			cascadeEnds[i] = log * 0.5f + uniform * 0.5f;
		}
		cascadeEnds[0] = nearPlane;
		cascadeEnds[numCascades] = farPlane;
		return cascadeEnds;
	}
	std::vector<xmmatrix> CascadedShadowmap::GenerateMatrices(const float3& direction, const xmmatrix& view, const xmmatrix& projection, uint32_t numCascades, uint32_t textureSize)
	{
		xmmatrix invProj = XMMatrixInverse(nullptr, projection);
		xmmatrix invViewProjection = XMMatrixInverse(nullptr, view * projection);

		xmvector normDir = XMVector3Normalize(XMLoadFloat3(&direction));
		xmmatrix lightView = XMMatrixLookAtLH(normDir, { 0.0f, 0.0f, 0.0f }, XMVector3Normalize(XMVector3Cross(normDir, {1.0f, 0.0f, 0.0f})));

		std::vector<float> cascadeEnds = GenerateCascadeDistances(projection, numCascades);

		const float2 clipSpaceCorners[] =
		{
			{-1.0f,  1.0f},
			{ 1.0f,  1.0f},
			{ 1.0f, -1.0f},
			{-1.0f, -1.0f}
		};

		std::vector<xmmatrix> matrices(numCascades);
		for (uint32_t cascade = 0; cascade < numCascades; cascade++)
		{
			xmvector cascadeCorners[8];
			
			float nearView = cascadeEnds[cascade];
			float farView  = cascadeEnds[cascade + 1];
			xmvector nearClipVector = XMVector4Transform({ 0.0f, 0.0f, nearView, 1.0f }, projection);
			xmvector farClipVector = XMVector4Transform({ 0.0f, 0.0f, farView, 1.0f }, projection);
			float nearClip = XMVectorGetZ(nearClipVector) / XMVectorGetW(nearClipVector);
			float farClip = XMVectorGetZ(farClipVector) / XMVectorGetW(farClipVector);

			for (int i = 0; i < 4; i++)
			{
				cascadeCorners[i  ]  = XMVector4Transform({ clipSpaceCorners[i].x, clipSpaceCorners[i].y, nearClip, 1.0f }, invViewProjection);
				cascadeCorners[i+4]  = XMVector4Transform({ clipSpaceCorners[i].x, clipSpaceCorners[i].y, farClip , 1.0f }, invViewProjection);
				cascadeCorners[i  ] /= XMVectorGetW(cascadeCorners[i]);
				cascadeCorners[i+4] /= XMVectorGetW(cascadeCorners[i+4]);
			}

			float3 minCorner(INFINITY, INFINITY, INFINITY);
			float3 maxCorner(-INFINITY, -INFINITY, -INFINITY);

			for (int i = 0; i < 8; i++) {
				xmvector pv = XMVector4Transform(cascadeCorners[i], lightView);
				float3 p; XMStoreFloat3(&p, pv);

				minCorner.x = Min(minCorner.x, p.x);
				minCorner.y = Min(minCorner.y, p.y);
				minCorner.z = Min(minCorner.z, p.z);

				maxCorner.x = Max(maxCorner.x, p.x);
				maxCorner.y = Max(maxCorner.y, p.y);
				maxCorner.z = Max(maxCorner.z, p.z);
			}

			xmmatrix lightProj = XMMatrixOrthographicOffCenterLH(minCorner.x, maxCorner.x, minCorner.y, maxCorner.y, maxCorner.z, minCorner.z);
			xmmatrix lightViewProj = lightView * lightProj;

			// Rounding - reduces shimmer
			xmvector shadowOrigin = XMVector4Transform({0.0f, 0.0f, 0.0f, 1.0f}, lightViewProj);
			shadowOrigin /= XMVectorGetW(shadowOrigin);
			shadowOrigin *= (float)textureSize / 2.0f;
			float3 origin; XMStoreFloat3(&origin, shadowOrigin);
			float2 roundedOrigin = float2(round(origin.x), round(origin.y));
			float2 rounding = {roundedOrigin.x - origin.x, roundedOrigin.y - origin.y};
			rounding = float2(rounding.x/(textureSize/2.0f), rounding.y / (textureSize / 2.0f));
			
			xmmatrix roundMatrix = XMMatrixTranslation(rounding.x, rounding.y, 0.0f);

			matrices[cascade] = lightViewProj*roundMatrix;
		}

		return matrices;
	}

}