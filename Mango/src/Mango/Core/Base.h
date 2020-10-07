#pragma once

#include <memory>

#include "Log.h"

#ifdef MG_DEBUG
	#define MG_CORE_ASSERT(x, ...) if(!(x)) { MG_CORE_ERROR(__VA_ARGS__); __debugbreak(); }
	#define MG_ASSERT(x, ...) if(!(x)) { MG_ERROR(__VA_ARGS__); __debugbreak(); }
#else
	#define MG_CORE_ASSERT(x, ...)
	#define MG_ASSERT(x, ...)
#endif

namespace Mango {
	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename... Args>
	inline static Ref<T> CreateRef(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename... Args>
	inline static Scope<T> CreateScope(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}