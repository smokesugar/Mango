#include "mgpch.h"
#include "ScriptEngine.h"

#include "Components.h"

namespace Mango {

	// Utility

	static void UploadFloat3(lua_State* L, const float3& vec) {
		lua_newtable(L);
		
		lua_pushstring(L, "x");
		lua_pushnumber(L, vec.x);
		lua_settable(L, -3);

		lua_pushstring(L, "y");
		lua_pushnumber(L, vec.y);
		lua_settable(L, -3);

		lua_pushstring(L, "z");
		lua_pushnumber(L, vec.z);
		lua_settable(L, -3);
	}

	static float3 RetrieveFloat3(lua_State* L) {
		float3 out;

		lua_pushstring(L, "x");
		lua_gettable(L, -2);
		out.x = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "y");
		lua_gettable(L, -2);
		out.y = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "z");
		lua_gettable(L, -2);
		out.z = (float)lua_tonumber(L, -1);
		lua_pop(L, 1);

		return out;
	}

	// Lua Side ---------------------------------------------------

	static int lua_Log(lua_State* L) {
		const char* str = lua_tostring(L, 1);
		MG_CORE_TRACE("[Script] {0}", str);
		Application::Get().GetRuntimeLog().AddLog(std::string("[Script] ") + str + "\n");
		return 0;
	}

	static int lua_GetTransform(lua_State* L) {
		auto& transform = *(TransformComponent*)lua_touserdata(L, lua_upvalueindex(1));
		lua_newtable(L);

		lua_pushstring(L, "translation");
		UploadFloat3(L, transform.Translation);
		lua_settable(L, -3);

		lua_pushstring(L, "rotation");
		UploadFloat3(L, transform.Rotation);
		lua_settable(L, -3);

		lua_pushstring(L, "scale");
		UploadFloat3(L, transform.Scale);
		lua_settable(L, -3);

		return 1;
	}

	static int lua_SetTransform(lua_State* L) {
		auto& transform = *(TransformComponent*)lua_touserdata(L, lua_upvalueindex(1));
		
		lua_pushstring(L, "translation");
		lua_gettable(L, -2);
		transform.Translation = RetrieveFloat3(L);
		lua_pop(L, 1);
		
		lua_pushstring(L, "rotation");
		lua_gettable(L, -2);
		transform.Rotation = RetrieveFloat3(L);
		lua_pop(L, 1);
		
		lua_pushstring(L, "scale");
		lua_gettable(L, -2);
		transform.Scale = RetrieveFloat3(L);
		lua_pop(L, 1);

		lua_pop(L, 1);
		return 0;
	}

	// -----------------------------------------------------------------

	void ScriptEngine::InitializeScriptComponents(ECS::Registry& reg)
	{
		auto query = reg.Query<LuaScriptComponent, TransformComponent>();
		for (auto& [size, scriptComps, transformComps] : query) {
			for (int i = 0; i < size; i++)
			{
				auto& comp = scriptComps[i];
				auto& L = comp.L;

				// Initialize
				L = luaL_newstate();
				luaL_openlibs(L);

				// Give Data
				lua_register(L, "Log", lua_Log);
				lua_pushlightuserdata(L, &transformComps[i]);
				lua_pushcclosure(L, lua_GetTransform, 1);
				lua_setglobal(L, "GetTransform");
				lua_pushlightuserdata(L, &transformComps[i]);
				lua_pushcclosure(L, lua_SetTransform, 1);
				lua_setglobal(L, "SetTransform");

				// Setup time
				lua_newtable(L);
				lua_pushstring(L, "elapsedTime");
				lua_pushnumber(L, 0);
				lua_settable(L, -3);
				lua_pushstring(L, "deltaTime");
				lua_pushnumber(L, 0);
				lua_settable(L, -3);
				lua_setglobal(L, "Time");

				// Run the actual file
				LUA_FN_CALL(L, luaL_dofile(L, comp.Path.c_str()));
			}
		}
	}

	void ScriptEngine::ShutdownScriptComponents(ECS::Registry& reg)
	{
		auto query = reg.Query<LuaScriptComponent>();
		for (auto& [size, scriptComps] : query) {
			for (int i = 0; i < size; i++) {
				auto& L = scriptComps[i].L;
				lua_close(L);
				L = nullptr;
			}
		}
	}

	bool ScriptEngine::UpdateScriptComponents(ECS::Registry& reg, float deltaTime, float elapsedTime)
	{
		auto scriptQuery = reg.Query<LuaScriptComponent>();
		bool error = false;
		for (auto& [size, scripts] : scriptQuery) {
			for (size_t i = 0; i < size; i++) {
				auto L = scripts[i].L;

				lua_getglobal(L, "Time");
				lua_pushstring(L, "elapsedTime");
				lua_pushnumber(L, elapsedTime);
				lua_settable(L, -3);
				lua_pushstring(L, "deltaTime");
				lua_pushnumber(L, deltaTime);
				lua_settable(L, -3);
				lua_pop(L, 1);

				lua_getglobal(L, "Update");
				if (lua_isfunction(L, -1)) {
					if (!LUA_FN_CALL(L, lua_pcall(L, 0, 0, 0)))
						error = true;
				}
				else {
					lua_pop(L, 1); // We need to call pop here in the case where we can't call "lua_pcall", which pops for us.
				}
			}
		}
		return !error;
	}

}