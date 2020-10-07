#include "mgpch.h"
#include "DirectXContext.h"

#include "DXDebug.h"

namespace Mango {

	GraphicsContext* GraphicsContext::Create() {
		return new DirectXContext();
	}

	DirectXContext::DirectXContext()
	{
		UINT flags = 0;
		#ifdef MG_DEBUG
			DXGIInfoQueue::Init();
			flags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		HR_CALL(D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			flags,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&mDevice,
			nullptr,
			&mDeviceContext
		));
	}

	DirectXContext::~DirectXContext()
	{
		#ifdef MG_DEBUG
			DXGIInfoQueue::Shutdown();
		#endif
	}

}