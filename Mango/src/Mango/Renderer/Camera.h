#pragma once

#include "Mango/Core/Math.h"

namespace Mango {
	
	class Camera {
	public:
		virtual ~Camera() {}
		virtual xmmatrix GetProjectionMatrix() const = 0;
		virtual void SetAspectRatio(float aspectRation) = 0;
	};

	class OrthographicCamera : public Camera {
	public:
		OrthographicCamera();
		OrthographicCamera(float aspectRatio, float zoom);
		inline virtual xmmatrix GetProjectionMatrix() const override { return mProjectionMatrix; }
		inline virtual void SetAspectRatio(float aspectRatio) override { mAspectRatio = aspectRatio; CalculateProjectionMatrix(); }
		inline void SetZoom(float zoom) { mZoom = zoom; CalculateProjectionMatrix(); }
	private:
		void CalculateProjectionMatrix();
	private:
		float mAspectRatio;
		float mZoom;
		xmmatrix mProjectionMatrix;
	};

}