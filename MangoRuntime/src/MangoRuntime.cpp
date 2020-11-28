#include "Mango.h"
#include "Mango/Core/EntryPoint.h"
#include "RuntimeLayer.h"

namespace Mango {

	class MangoRuntime : public Application {
	public:
		MangoRuntime() {
			PushLayer(new RuntimeLayer());
		}

		~MangoRuntime() {

		}
	};

}

Mango::Application* Mango::CreateApplication() {
	return new Mango::MangoRuntime();
}