#ifndef FRAMEWORK_SCRIPT_BASIC_HPP
#define FRAMEWORK_SCRIPT_BASIC_HPP

extern "C" {
#include <lauxlib.h>
}

#include <cstdint>
#include <string>
#include <typeinfo>
#include <vector>

struct lua_State;

namespace Script
{

class Utils
{
public:
	template < class Class >
	[[nodiscard]] inline static auto DemangleClassName() -> std::string;
	[[nodiscard]] static auto DemangleClassName(const std::string& name) -> std::string;
	[[nodiscard]] static auto StringExplode(const std::string& str) -> std::vector< std::string >;

	[[nodiscard]] static auto StrongRefSet(lua_State*) -> int;
	static void StrongUnref(lua_State*, const int referenceId);
	static void StrongRefGet(lua_State*, const int referenceId);

	static void WeakRefCreate(lua_State*);
	[[nodiscard]] static auto WeakRefSet(lua_State*) -> int;
	static void WeakUnref(lua_State*, const int referenceId);
	static void WeakRefGet(lua_State*, const int referenceId);
};

template < class Class >
auto Utils::DemangleClassName() -> std::string
{
	return DemangleClassName(typeid(Class).name());
}

} // namespace Script

#endif
