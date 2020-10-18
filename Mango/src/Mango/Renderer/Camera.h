#pragma once

#include "Mango/Core/Math.h"

namespace Mango {
	
	class Camera {
	public:
		enum class Type {
			Orthographic, Perspective
		};
	public:
		Camera(Type type, float pfov, float pnear, float pfar, float osize);
		xmmatrix GetProjectionMatrix(float aspect);
		inline Type GetType() const { return mType; }

		inline float GetPFOV() const { return mPFOV; }
		inline float GetPNear() const { return mPNear; }
		inline float GetPFar() const { return mPFar; }
		inline float GetOSize() const { return mOSize; }

		inline void SetPFOV(float pfov) { mPFOV = pfov; }
		inline void SetPNear(float pnear) { mPNear = pnear; }
		inline void SetPFar(float pfar) { mPFar = pfar; }
		inline void SetOSize(float osize) { mOSize = osize; }

		static Camera CreateOrthographic(float size);
		static Camera CreatePerspective(float fov, float nearPlane, float farPlane);
	private:
		Type mType;

		float mPFOV;
		float mPNear;
		float mPFar;
		
		float mOSize;
	};

}