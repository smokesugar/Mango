#pragma once

#include <utility>
#include "BaseEvent.h"
#include "Mango/Core/InputCodes.h"

namespace Mango {
	
	class MouseButtonEvent : public Event {
	public:
		MouseButtonEvent(uint32_t button)
			: mButton((MouseCode)button) {}
		virtual ~MouseButtonEvent() {}

		inline MouseCode GetButton() const { return mButton; }

		EVENT_SET_CATEGORY(EventCategory_Mouse | EventCategory_Input)
	protected:
		MouseCode mButton;
	};

	class MouseButtonDownEvent : public MouseButtonEvent {
	public:
		MouseButtonDownEvent(uint32_t button)
			: MouseButtonEvent(button) {}

		virtual std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseButtonDownEvent: " << (uint16_t)mButton;
			return ss.str();
		}

		EVENT_SET_TYPE(MouseButtonDown)
	};

	class MouseButtonUpEvent : public MouseButtonEvent {
	public:
		MouseButtonUpEvent(uint32_t button)
			: MouseButtonEvent(button) {}

		virtual std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseButtonUpEvent: " << (uint16_t)mButton;
			return ss.str();
		}

		EVENT_SET_TYPE(MouseButtonUp)
	};

	class MouseMoveEvent : public Event {
	public:
		MouseMoveEvent(float x, float y)
			: mXPos(x), mYPos(y) {}

		virtual std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseMoveEvent: " << mXPos << ", " << mYPos;
			return ss.str();
		}

		inline float GetXPos() const { return mXPos; }
		inline float GetYPos() const { return mYPos; }
		inline std::pair<float, float> GetPos() const { return { mXPos, mYPos }; }

		EVENT_SET_CATEGORY(EventCategory_Mouse | EventCategory_Input)
		EVENT_SET_TYPE(MouseMove)
	private:
		float mXPos, mYPos;
	};

	class RawMouseMoveEvent : public Event {
	public:
		RawMouseMoveEvent(float deltaX, float deltaY)
			: mDeltaX(deltaX), mDeltaY(deltaY)
		{}

		virtual std::string ToString() const override {
			std::stringstream ss;
			ss << "RawMouseMoveEvent: " << mDeltaX << ", " << mDeltaY;
			return ss.str();
		}

		inline float GetDeltaX() const { return mDeltaX; }
		inline float GetDeltaY() const { return mDeltaY; }
		inline std::pair<float, float> GetDelta() const { return {mDeltaX, mDeltaY}; }

		EVENT_SET_CATEGORY(EventCategory_Mouse | EventCategory_Input)
		EVENT_SET_TYPE(RawMouseMove)
	private:
		float mDeltaX, mDeltaY;
	};

	class MouseScrollEvent : public Event {
	public:
		MouseScrollEvent(float delta)
			: mDelta(delta) {}

		virtual std::string ToString() const override {
			std::stringstream ss;
			ss << "MouseScrollEvent: " << mDelta;
			return ss.str();
		}

		inline float GetDelta() const { return mDelta; }

		EVENT_SET_CATEGORY(EventCategory_Mouse | EventCategory_Input)
		EVENT_SET_TYPE(MouseScroll)
	private:
		float mDelta;
	};

}
