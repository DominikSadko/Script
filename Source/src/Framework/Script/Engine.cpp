#include <Framework/Script/Engine.hpp>

#include <Framework/Script/Basic.hpp>
#include <Framework/Script/Metatable.hpp>
#include <Framework/Script/Sandbox.hpp>

extern "C" {
#include <lualib.h>
}

namespace Script
{

Engine::Engine()
	: L(luaL_newstate())
{
	luaL_openlibs(L);

	Utils::WeakRefCreate(L);
}

Engine::~Engine()
{
	lua_close(L);
	L = nullptr;
}

auto Engine::Call(const int32_t nargs, const int32_t nresults, const int32_t ctx) const -> bool
{
	if (lua_pcall(L, nargs, nresults, ctx)) {
		throw std::string{ lua_tostring(L, -1) };
	}
	return true;
}

auto Engine::LoadScriptFile(const std::string& filename, const char* mode) const -> bool
{
	if (luaL_loadfilex(L, filename.c_str(), mode)) {
		throw std::string{ lua_tostring(L, -1) };
	}
	return true;
}

auto Engine::LoadScript(const std::string& script) const -> bool
{
	return LoadScript(script.c_str());
}

auto Engine::LoadScript(const char* script) const -> bool
{
	if (luaL_loadstring(L, script)) {
		throw std::string{ lua_tostring(L, -1) };
	}
	return true;
}

auto Engine::ExecuteRaw(const std::string& script) const -> bool
{
	return LoadScript(script) && Call();
}

auto Engine::ExecuteRaw(const char* script) const -> bool
{
	return LoadScript(script) && Call();
}

auto Engine::Execute(const std::string& script) const -> Reference
{
	if (!LoadScript(script) || !Call(0, 1, 0)) {
		return {};
	}
	return Reference(L, -1, true);
}

auto Engine::ExecuteFile(const std::string& filename, const char* mode) const -> bool
{
	return LoadScriptFile(filename, mode) && Call();
}

void Engine::CollectGarbage()
{
	lua_gc(L, LUA_GCCOLLECT, 0);
}

void Engine::RemoveGlobal(const std::string& name) const
{
	lua_pushnil(L);
	lua_setglobal(L, name.c_str());
}

auto Engine::GetGlobal() const -> Reference
{
	return Reference{ L, std::string{} };
}

auto Engine::GetGlobal(const std::string& name) const -> Reference
{
	return Reference{ L, name };
}

auto Engine::operator[](const std::string& name) const -> Reference
{
	return Reference{ L, name };
}

auto Engine::GetMetatable(const std::string_view& name) const -> MetatablePtr
{
	return MetatablePtr{ new Metatable{ this, name } };
}

auto Engine::GetMetatable(const std::string_view& name, const std::string_view& parentName) const -> MetatablePtr
{
	return MetatablePtr{ new Metatable{ this, name, parentName } };
}

auto Engine::GetSandbox(const std::string_view& name) const -> SandboxPtr
{
	return SandboxPtr{ new Sandbox{ this, name } };
}

} // namespace Script
