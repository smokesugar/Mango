#include "mgpch.h"
#include "RenderCommand.h"

namespace Mango {

	RendererAPI* RenderCommand::sRendererAPI = RendererAPI::Create();

	void RenderCommand::Draw(size_t count, size_t offset)
	{
		sRendererAPI->Draw(count, offset);
	}

	void RenderCommand::DrawIndexed(size_t count, size_t offset)
	{
		sRendererAPI->DrawIndexed(count, offset);
	}

}