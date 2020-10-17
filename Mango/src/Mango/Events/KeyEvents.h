#pragma once

#include <utility>
#include "BaseEvent.h"
#include "Mango/Core/InputCodes.h"

namespace Mango {
	
	class KeyEvent : public Event {
	public:
		KeyEvent(uint32_t keycode)
			: mKeycode((KeyCode)keycode) {}

		virtual ~KeyEvent() {}

		inline KeyCode GetKeycode() const { return mKeycode; }

		EVENT_SET_CATEGORY(EventCategory_Keyboard | EventCategory_Input)
	protected:
		KeyCode mKeycode;
	};

	class KeyDownEvent : public KeyEvent {
	public:
		KeyDownEvent(uint32_t keycode)
			: KeyEvent(keycode) {}

		virtual std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyDownEvent: " << (uint16_t)mKeycode;
			return ss.str();
		}

		EVENT_SET_TYPE(KeyDown)
	};

	class KeyUpEvent : public KeyEvent {
	public:
		KeyUpEvent(uint32_t keycode)
			: KeyEvent(keycode) {}

		virtual std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyUpEvent: " << (uint16_t)mKeycode;
			return ss.str();
		}

		EVENT_SET_TYPE(KeyUp)
	};

}