#include <Framework/Script/Basic.hpp>

extern "C" {
#include <lauxlib.h>
}

#include <cstring>
#include <cxxabi.h>

namespace Script
{

auto ReplaceAll(std::string str, const std::string& fromStr, const std::string& toStr) -> std::string
{
	size_t startPos = size_t{};
	while ((startPos = str.find(fromStr, startPos)) != std::string::npos) {
		str.replace(startPos, fromStr.length(), toStr);
		startPos += toStr.length();
	}
	return str;
}

auto Utils::DemangleClassName(const std::string& name) -> std::string
{
	constexpr size_t bufferSize = 1024;
	static std::vector< char > buffer(bufferSize);
	size_t len = 0;
	int status = 0;
	char* demangled = abi::__cxa_demangle(name.c_str(), nullptr, &len, &status);

	if (demangled) {
		strncpy(buffer.data(), demangled, bufferSize - 1);
		buffer[ bufferSize - 1 ] = '\0';
		std::free(demangled);
	} else {
		buffer[ 0 ] = '\0';
	}

	return ReplaceAll(buffer.data(), "::", "");
}

auto Utils::StringExplode(const std::string& string) -> std::vector< std::string >
{
	std::size_t pos = string.find(".");
	if (pos == std::string::npos) {
		return std::vector< std::string >{ { string } };
	}

	std::vector< std::string > result = {};
	std::size_t lastpos = 0;
	while (pos != std::string::npos) {
		result.emplace_back(string, lastpos, pos - lastpos);

		lastpos = ++pos;

		if (pos = string.find(".", pos); pos == std::string::npos) {
			result.emplace_back(string, lastpos, pos - lastpos);
		}
	}

	return result;
}

auto Utils::StrongRefSet(lua_State* L) -> int
{
	const int referenceId = luaL_ref(L, LUA_REGISTRYINDEX);
	return referenceId;
}

void Utils::StrongUnref(lua_State* L, const int referenceId)
{
	return luaL_unref(L, LUA_REGISTRYINDEX, referenceId);
}

void Utils::StrongRefGet(lua_State* L, const int referenceId)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, referenceId);
}

void Utils::WeakRefCreate(lua_State* L)
{
	lua_newtable(L);
	lua_newtable(L);
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_pushnumber(L, 0);
	lua_rawseti(L, -2, 0);
	lua_setglobal(L, "__ref");
}

auto Utils::WeakRefSet(lua_State* L) -> int
{
	lua_getglobal(L, "__ref");
	lua_pushvalue(L, -2);

	const int referenceId = luaL_ref(L, -2);
	lua_pop(L, 2);
	return referenceId;
}

void Utils::WeakUnref(lua_State* L, const int referenceId)
{
	lua_getglobal(L, "__ref");
	luaL_unref(L, -1, referenceId);
	lua_pop(L, 1);
}

void Utils::WeakRefGet(lua_State* L, const int referenceId)
{
	lua_getglobal(L, "__ref");
	lua_rawgeti(L, -1, referenceId);
	lua_remove(L, -2);
}

} // namespace Script
