#pragma once

#include "Mango/Renderer/GraphicsContext.h"

#include <wrl.h>
#include "Mango/Core/WinCore.h"
#include <d3d11.h>

#include "DXDebug.h"

#define RetrieveContext() *(DirectXContext*)&Application::Get().GetGraphicsContext()

namespace Mango {
	
	class DirectXContext : public GraphicsContext {
	public:
		DirectXContext();
		virtual ~DirectXContext();

		inline ID3D11Device* GetDevice() { return mDevice.Get(); }
		inline ID3D11DeviceContext* GetDeviceContext() { return mDeviceContext.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> mDeviceContext;
	};

}