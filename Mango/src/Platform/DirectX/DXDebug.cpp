#include "mgpch.h"
#include "DXDebug.h"

#include "Mango/Core/Base.h"

namespace Mango {

	uint32_t DXGIInfoQueue::mIndex = 0;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> DXGIInfoQueue::mInternal;

	void DXGIInfoQueue::Init()
	{
		typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);
		const auto hModDxgiDebug = LoadLibraryEx(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		MG_CORE_ASSERT(hModDxgiDebug, "Unable to load DXGI Debug Library: {0}", Translate(GetLastError()));
		const auto DxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(
			reinterpret_cast<void*>(GetProcAddress(hModDxgiDebug, "DXGIGetDebugInterface"))
		);
		MG_CORE_ASSERT(DxgiGetDebugInterface, "Unable to create DXGI Debug Interface: {0}", Translate(GetLastError()));

		HRESULT hr;
		if (FAILED(hr = DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &mInternal))) {
			MG_CORE_ASSERT(false, "Unable to create DXGI InfoQueue: {0}", Translate(hr));
		}
	}

	void DXGIInfoQueue::Shutdown()
	{
		mInternal = nullptr;
	}

	void DXGIInfoQueue::Clear()
	{
		mIndex = (uint32_t)mInternal->GetNumStoredMessages(DXGI_DEBUG_ALL);
	}

	std::string DXGIInfoQueue::Get()
	{
		std::stringstream ss;
		const auto end = mInternal->GetNumStoredMessages(DXGI_DEBUG_ALL);
		for (auto i = mIndex; i < end; i++)
		{
			HRESULT hr;
			SIZE_T messageLength;
			// get the size of message i in bytes
			if (FAILED(hr = mInternal->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength))) {
				MG_CORE_ASSERT(false, "Unable to get message from InfoQueue: {0}", Translate(hr));
			}
			// allocate memory for message
			auto bytes = std::make_unique<byte[]>(messageLength);
			auto pMessage = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
			// get the message and push its description into the vector
			if (FAILED(hr = mInternal->GetMessage(DXGI_DEBUG_ALL, i, pMessage, &messageLength))) {
				MG_CORE_ASSERT(false, "Unable to get message from InfoQueue: {0}", Translate(hr));
			}

			ss << pMessage->pDescription;
			if (i != end - 1) {
				ss << "\n";
			}
		}

		return ss.str();
	}

}
