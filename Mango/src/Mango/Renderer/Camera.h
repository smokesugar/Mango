#pragma once

#include "Mango/Core/Math.h"

namespace Mango {
	
	class Camera {
	public:
		enum class Type {
			Orthographic, Perspective
		};
	public:
		virtual ~Camera() {}
		virtual xmmatrix GetProjectionMatrix() const = 0;
		virtual void SetAspectRatio(float aspectRation) = 0;
		virtual float GetAspectRatio() const = 0;
		virtual Type GetType() const = 0;
	};

	class OrthographicCamera : public Camera {
	public:
		OrthographicCamera();
		OrthographicCamera(float aspectRatio, float zoom);
		inline virtual xmmatrix GetProjectionMatrix() const override { return mProjectionMatrix; }
		inline virtual void SetAspectRatio(float aspectRatio) override { mAspectRatio = aspectRatio; CalculateProjectionMatrix(); }
		inline virtual float GetAspectRatio() const override { return mAspectRatio; }
		inline virtual Type GetType() const override { return Type::Orthographic; }
		inline void SetZoom(float zoom) { mZoom = zoom; CalculateProjectionMatrix(); }
		inline float GetZoom() const { return mZoom; }
	private:
		void CalculateProjectionMatrix();
	private:
		float mAspectRatio;
		float mZoom;
		xmmatrix mProjectionMatrix;
	};

}