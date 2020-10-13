#include "Mango.h"
#include "Mango/Core/EntryPoint.h"
#include "SandboxLayer.h"

namespace Mango {

	class SandboxApp : public Application {
	public:
		SandboxApp() {
			PushLayer(new SandboxLayer());
		}

		~SandboxApp() {

		}
	};

}

Mango::Application* Mango::CreateApplication() {
	return new SandboxApp();
}