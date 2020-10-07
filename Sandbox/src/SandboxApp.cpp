#include "Mango.h"

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