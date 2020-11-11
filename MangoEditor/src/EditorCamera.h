#pragma once

#include "Mango/Core/Math.h"
#include "Mango/Events/Events.h"

namespace Mango {

	class EditorCamera {
	public:
		EditorCamera();
		inline xmmatrix GetProjectionMatrix(float aspect) const { return XMMatrixPerspectiveFovLH(mFOV, aspect, 100.0f, 0.1f); }
		inline xmmatrix GetProjectionMatrixNotInverted(float aspect) const { return XMMatrixPerspectiveFovLH(mFOV, aspect, 0.1f, 100.0f); }
		xmmatrix GetTransform() const;
		void OnEvent(Event& e);
		inline void SetAcceptingInput(bool accepting) { mAcceptingInput = accepting; }
	private:
		bool OnRawMouseMove(RawMouseMoveEvent& e);
		bool OnMouseButtonDown(MouseButtonDownEvent& e);
		bool OnMouseButtonUp(MouseButtonUpEvent& e);
		bool OnMouseScroll(MouseScrollEvent& e);
	private:
		enum class MovementMode
		{
			None = 0,
			Translation,
			Rotation
		};

		MovementMode mMode = MovementMode::None;
		bool mAcceptingInput = false;

		float mFOV;
		xmvector mFocus;
		xmvector mOffset;
		xmvector mUp;
		xmvector mRight;
	};
}