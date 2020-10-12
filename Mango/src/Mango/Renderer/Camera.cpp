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

}
