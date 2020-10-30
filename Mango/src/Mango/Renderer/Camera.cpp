#include "mgpch.h"
#include "Camera.h"

namespace Mango {

	Camera::Camera(Type type, float pfov, float pnear, float pfar, float osize)
		: mType(type), mPFOV(pfov), mPNear(pnear), mPFar(pfar), mOSize(osize)
	{
	}

	xmmatrix Camera::GetProjectionMatrix(float aspect)
	{
		if (mType == Type::Orthographic)
			return XMMatrixOrthographicOffCenterLH(aspect * -mOSize / 2, aspect * mOSize / 2, -mOSize / 2, mOSize / 2, 1.0f, -1.0f);
		else
			return XMMatrixPerspectiveFovLH(mPFOV, aspect, mPFar, mPNear);
	}

	Camera Camera::CreateOrthographic(float size)
	{
		return Camera(Type::Orthographic, 0.0f, 0.0f, 0.0f, size);
	}

	Camera Camera::CreatePerspective(float fov, float nearPlane, float farPlane)
	{
		return Camera(Type::Perspective, fov, nearPlane, farPlane, 0.0f);
	}

}
