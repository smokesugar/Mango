#include "EditorCamera.h"

#include "Mango/Core/Input.h"

namespace Mango {

	EditorCamera::EditorCamera()
		: mFOV(ToRadians(45.0f)), mFocus({ 0.0f, 0.0f, 0.0f, 1.0f }), mOffset({ 0.0f, 3.0f, -5.0f, 1.0f }), mUp({ 0.0f, 1.0f, 0.0f, 0.0f }), mRight({ 1.0f, 0.0f, 0.0f, 0.0f })
	{
	}

	xmmatrix EditorCamera::GetTransform() const
	{
		return XMMatrixInverse(nullptr, XMMatrixLookAtLH(mFocus + mOffset, mFocus, mUp));
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<RawMouseMoveEvent>(MG_BIND_FN(EditorCamera::OnRawMouseMove));
		dispatcher.Dispatch<MouseButtonDownEvent>(MG_BIND_FN(EditorCamera::OnMouseButtonDown));
		dispatcher.Dispatch<MouseButtonUpEvent>(MG_BIND_FN(EditorCamera::OnMouseButtonUp));
		dispatcher.Dispatch<MouseScrollEvent>(MG_BIND_FN(EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnRawMouseMove(RawMouseMoveEvent& e)
	{
		switch (mMode)
		{
		case MovementMode::Translation: {
			float sensitivity = XMVectorGetX(XMVector3Length(mOffset)) * 0.001f;
			mFocus += mRight * sensitivity * e.GetDeltaX();
			mFocus += mUp * sensitivity * e.GetDeltaY();
			break;
		}
		case MovementMode::Rotation: {
			float sensitivity = 0.01f;

			xmmatrix yaw = XMMatrixRotationAxis({ 0.0f, 1.0f, 0.0f }, e.GetDeltaX() * sensitivity);
			mOffset = XMVector4Transform(mOffset, yaw);
			mUp = XMVector4Transform(mUp, yaw);

			xmvector forward = XMVector3Normalize(-mOffset);
			mRight = XMVector3Normalize(XMVector3Cross(forward, mUp));

			xmmatrix pitch = XMMatrixRotationAxis(mRight, -e.GetDeltaY() * sensitivity);
			mOffset = XMVector4Transform(mOffset, pitch);
			mUp = XMVector4Transform(mUp, pitch);
			break;
		}
		}

		return false;
	}

	bool EditorCamera::OnMouseButtonDown(MouseButtonDownEvent& e)
	{
		if (e.GetButton() == MouseCode::MBUTTON && mAcceptingInput) {
			mMode = Input::IsKeyDown(KeyCode::LSHIFT) ? MovementMode::Translation : MovementMode::Rotation;
		}
		return false;
	}

	bool EditorCamera::OnMouseButtonUp(MouseButtonUpEvent& e)
	{
		if (e.GetButton() == MouseCode::MBUTTON)
			mMode = MovementMode::None;
		return false;
	}

	bool EditorCamera::OnMouseScroll(MouseScrollEvent& e)
	{
		mOffset -= e.GetDelta() * mOffset * 0.05f * (float)mAcceptingInput;
		return false;
	}

}