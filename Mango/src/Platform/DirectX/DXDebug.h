#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <dxgidebug.h>

#include "Mango/Core/Base.h"

#ifdef MG_DEBUG
#define HR_CALL(fn) { DXGIInfoQueue::Clear(); if(FAILED(fn)) { MG_CORE_ASSERT(false, "Failed HR call:\n{0}", DXGIInfoQueue::Get()); } };
#define VOID_CALL(fn) { DXGIInfoQueue::Clear(); (fn); std::string _message = DXGIInfoQueue::Get(); MG_CORE_ASSERT(_message.empty(), "Failed void call:\n{0}", _message); }
#else
#define HR_CALL(fn) fn
#define VOID_CALL(fn) fn
#endif
namespace Mango {
	
	class DXGIInfoQueue {
	public:
		static void Init();
		static void Shutdown();
		static void Clear();
		static std::string Get();
	private:
		static uint32_t mIndex;
		static Microsoft::WRL::ComPtr<IDXGIInfoQueue> mInternal;
	};

}