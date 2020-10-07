#pragma once

#include <utility>
#include "BaseEvent.h"

namespace Mango {
	
	class WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() {}

		virtual std::string ToString() const override {
			return "WindowCloseEvent";
		}

		EVENT_SET_CATEGORY(EventCategory_Application)
		EVENT_SET_TYPE(WindowClose)
	};

	class WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(uint32_t width, uint32_t height)
			: mWidth(width), mHeight(height) {}

		virtual std::string ToString() const override {
			std::stringstream ss;
			ss << "WindowResizeEvent: " << mWidth << ", " << mHeight;
			return ss.str();
		}

		inline uint32_t GetWidth() const { return mWidth; }
		inline uint32_t GetHeight() const { return mHeight; }

		inline std::pair<uint32_t, uint32_t> GetDimensions() const { return {mWidth, mHeight}; }

		EVENT_SET_CATEGORY(EventCategory_Application)
		EVENT_SET_TYPE(WindowResize)
	private:
		uint32_t mWidth;
		uint32_t mHeight;
	};

}