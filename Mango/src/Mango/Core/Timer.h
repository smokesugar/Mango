#pragma once

#include <chrono>
#include "Mango/Core/Base.h"

namespace Mango {
	
	class Timer {
	public:
		Timer(const char* name)
			: mName(name)
		{
			mStartTimepoint = std::chrono::high_resolution_clock::now();
		}

		~Timer() {
			Stop();
		}

		void Stop() {
			auto endTimepoint = std::chrono::high_resolution_clock::now();
			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(mStartTimepoint).time_since_epoch().count();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			auto duration = end - start;
			double ms = duration * 0.001;

			MG_CORE_TRACE("Timer {0}: {1}ms", mName, ms);
		}
	private:
		const char* mName;
		std::chrono::time_point<std::chrono::high_resolution_clock> mStartTimepoint;
	};

}