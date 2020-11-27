#pragma once

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "Mango/Core/Application.h"

namespace Mango {

	static bool LUA_FN_CALL(lua_State* L, int r) {
		if (r != LUA_OK) {
			std::string errormsg = "Script error:\n";
			errormsg.append(lua_tostring(L, -1));
			errormsg.append("\n");
			MG_ERROR(errormsg);
			Application::Get().GetRuntimeLog().AddLog(errormsg);
			return false;
		}
		return true;
	}

}