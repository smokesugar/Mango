#pragma once

#include <string>

namespace Mango {

	class Shader {
	public:
		virtual ~Shader() {}

		virtual void Bind() const = 0;

		static Shader* Create(const std::string& vertex, const std::string& pixel);
	};

}