#include "mgpch.h"
#include "DirectXAPI.h"

#include "Mango/Core/Application.h"
#include "DirectXContext.h"

namespace Mango {

	RendererAPI* RendererAPI::Create() {
		return new DirectXAPI();
	}

	void DirectXAPI::Draw(size_t count, size_t offset)
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->Draw((uint32_t)count, (uint32_t)offset));
	}

	void DirectXAPI::DrawIndexed(size_t count, size_t offset)
	{
		auto& context = RetrieveContext();
		VOID_CALL(context.GetDeviceContext()->DrawIndexed((uint32_t)count, (uint32_t)offset, 0));
	}

}