#include <Framework/Script/Sandbox.hpp>

#include <Framework/Script/Engine.hpp>

namespace Script
{

Sandbox::Sandbox(const Engine* engine, std::string_view sandbox)
{
	mReference = engine->GetGlobal(sandbox.data());
	if (mReference.GetType() != Script::VariableType::Nil) {
		return;
	}

	lua_State* L = engine->State();

	lua_newtable(L);
	lua_pushvalue(L, -1);

	lua_newtable(L);
	lua_getglobal(L, "_G");
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
	lua_pop(L, 1);

	lua_setglobal(L, sandbox.data());
	mReference = engine->GetGlobal(sandbox.data());
}

auto Sandbox::Execute(const std::string& script) -> bool
{
	return mReference.CallSandbox(script);
}

} // namespace Script
