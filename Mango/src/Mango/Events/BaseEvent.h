#pragma once

#include <sstream>
#include <string>

#include <spdlog/fmt/ostr.h>

#include "Mango/Core/Base.h"

namespace Mango {
	
	enum class EventType {
		None = 0,
		WindowClose, WindowResize,
		MouseMove, RawMouseMove, MouseButtonDown, MouseButtonUp, MouseScroll,
		KeyDown, KeyUp
	};

	enum EventCategory{
		None = 0,
		EventCategory_Application = BIT(0),
		EventCategory_Input       = BIT(1),
		EventCategory_Keyboard    = BIT(2),
		EventCategory_Mouse       = BIT(3)
	};

	#define EVENT_SET_CATEGORY(category) virtual int GetCategory() const override { return category; }
	#define EVENT_SET_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
							 virtual EventType GetType() const override { return GetStaticType(); }

	class Event {
	public:
		virtual ~Event() {}
		virtual std::string ToString() const = 0;
		virtual int GetCategory() const = 0;
		virtual EventType GetType() const = 0;
		inline bool IsInCategory(EventCategory category) {
			return GetCategory() & category;
		}
	public:
		bool Handled = false;
	};

	class EventDispatcher {
	public:
		EventDispatcher(Event& e)
			: mEvent(e) {}

		template<typename T, typename Fn>
		void Dispatch(const Fn& func) {
			if (mEvent.GetType() == T::GetStaticType()) {
				mEvent.Handled = func(*(T*)&mEvent);
			}
		}
	private:
		Event& mEvent;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e) {
		return os << e.ToString();
	}

}