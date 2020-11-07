#include "mgpch.h"
#include "RenderCommand.h"

#include "RendererAPI.h"

namespace Mango {

	static RendererAPI* sRendererAPI = RendererAPI::Create();

	void RenderCommand::Draw(size_t count, size_t offset)
	{
		sRendererAPI->Draw(count, offset);
	}

	void RenderCommand::DrawIndexed(size_t count, size_t offset)
	{
		sRendererAPI->DrawIndexed(count, offset);
	}

	void RenderCommand::EnableBlending()
	{
		sRendererAPI->EnableBlending();
	}

	void RenderCommand::DisableBlending()
	{
		sRendererAPI->DisableBlending();
	}

	void RenderCommand::EnableCulling()
	{
		sRendererAPI->DefaultRasterizerState();
	}

	void RenderCommand::DisableCulling()
	{
		sRendererAPI->DisableCulling();
	}

	void RenderCommand::EnableInvertedDepthTesting()
	{
		sRendererAPI->EnableInvertedDepthTesting();
	}

}