#include "mgpch.h"
#include "Camera.h"

namespace Mango {

	OrthographicCamera::OrthographicCamera()
		: mAspectRatio(1.0f), mZoom(2.5f)
	{
		CalculateProjectionMatrix();
	}

	OrthographicCamera::OrthographicCamera(float aspectRatio, float zoom)
		: mAspectRatio(aspectRatio), mZoom(zoom)
	{
		CalculateProjectionMatrix();
	}

	void OrthographicCamera::CalculateProjectionMatrix()
	{
		float left = -mAspectRatio * mZoom * 0.5f;
		float right = mAspectRatio * mZoom * 0.5f;
		float bottom = -mZoom * 0.5f;
		float top = mZoom * 0.5f;

		mProjectionMatrix = XMMatrixOrthographicOffCenterLH(
			left, right,
			bottom, top,
			-1.0f, 1.0f
		);
	}

	PerspectiveCamera::PerspectiveCamera()
		: mFOV(ToRadians(45.0f)), mAspectRatio(1.0f), mNearPlane(0.1f), mFarPlane(100.0f)
	{
	}

	PerspectiveCamera::PerspectiveCamera(float fovy, float aspectRatio, float nearPlane, float farPlane)
		: mFOV(fovy), mAspectRatio(aspectRatio), mNearPlane(nearPlane), mFarPlane(farPlane)
	{
	}

	xmmatrix PerspectiveCamera::GetProjectionMatrix() const
	{
		return XMMatrixPerspectiveFovLH(mFOV, mAspectRatio, mNearPlane, mFarPlane);
	}

}
