#include "Mango.h"
#include "Mango/Core/EntryPoint.h"
#include "EditorLayer.h"

namespace Mango {

	class SandboxApp : public Application {
	public:
		SandboxApp() {
			PushLayer(new EditorLayer());
		}

		~SandboxApp() {

		}
	};

}

Mango::Application* Mango::CreateApplication() {
	return new SandboxApp();
}