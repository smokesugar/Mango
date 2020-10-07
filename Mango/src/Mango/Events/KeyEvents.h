#pragma once

#include <utility>
#include "BaseEvent.h"

namespace Mango {
	
	class KeyEvent : public Event {
	public:
		KeyEvent(uint32_t keycode)
			: mKeycode(keycode) {}

		virtual ~KeyEvent() {}

		inline uint32_t GetKeycode() const { return mKeycode; }

		EVENT_SET_CATEGORY(EventCategory_Keyboard | EventCategory_Input)
	protected:
		uint32_t mKeycode;
	};

	class KeyDownEvent : public KeyEvent {
	public:
		KeyDownEvent(uint32_t keycode)
			: KeyEvent(keycode) {}

		virtual std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyDownEvent: " << mKeycode;
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
			ss << "KeyUpEvent: " << mKeycode;
			return ss.str();
		}

		EVENT_SET_TYPE(KeyUp)
	};

}