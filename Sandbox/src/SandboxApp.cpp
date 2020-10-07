#include "Mango.h"
#include "Mango/Core/EntryPoint.h"

using namespace Mango;

class SandboxApp : public Application {
public:
	SandboxApp() {

	}

	~SandboxApp() {
		
	}
};

Mango::Application* Mango::CreateApplication() {
	return new SandboxApp();
}