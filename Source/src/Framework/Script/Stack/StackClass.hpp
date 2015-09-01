#ifndef FRAMEWORK_SCRIPT_STACK_STACKCLASS_HPP
#define FRAMEWORK_SCRIPT_STACK_STACKCLASS_HPP

#include <Framework/Script/Basic.hpp>
#include <Framework/Script/ObjectOwnership.hpp>
#include <Framework/Script/Stack/StackBasic.hpp>
#include <Framework/Script/VariableType.hpp>

namespace Script
{

template < class Class >
struct Stack< Class*, std::enable_if_t< std::is_class_v< Class > > >
{
	static Class* Get(lua_State* L, const int32_t idx)
	{
		const VariableType type = static_cast< VariableType >(lua_type(L, idx));

		if (type == VariableType::UserData) {
			ObjectOwnership** pointer = static_cast< ObjectOwnership** >(lua_touserdata(L, idx));
			if constexpr (std::is_base_of_v< Object, Class >) {
				return dynamic_cast< Class* >((*pointer)->Get().get());
			} else {
				return reinterpret_cast< Class* >((*pointer)->Get().get());
			}

		} else if (type == VariableType::LightUserData) {
			return static_cast< Class* >(lua_touserdata(L, idx));
		}

		return nullptr;
	}

	static void Push(lua_State* L, Class* value)
	{
		if (!value) {
			Stack< std::nullptr_t >::Push(L, nullptr);
			return;
		}

		lua_pushlightuserdata(L, static_cast< void* >(value));

		const std::string metatable = Utils::DemangleClassName< Class >();
		luaL_setmetatable(L, metatable.c_str());
	}

	static bool Is(lua_State* L, const int32_t idx) { return lua_isuserdata(L, idx); }
};

template < class Class >
struct Stack< std::shared_ptr< Class > >
{
	static std::shared_ptr< Class > Get(lua_State* L, const int32_t idx)
	{
		const VariableType type = static_cast< VariableType >(lua_type(L, idx));
		if (type == VariableType::UserData) {
			if (ObjectOwnership** pointer = static_cast< ObjectOwnership** >(lua_touserdata(L, idx))) {
				return std::reinterpret_pointer_cast< Class >((*pointer)->Get());
			}
		}

		return nullptr;
	}

	static void Push(lua_State* L, const std::shared_ptr< Class >& thing)
	{
		if (!thing) {
			Stack< std::nullptr_t >::Push(L, nullptr);
			return;
		}

		ObjectOwnership* userdata = new ObjectOwnership{ std::reinterpret_pointer_cast< Object >(thing) };
		ObjectOwnership** pointer = static_cast< ObjectOwnership** >(lua_newuserdata(L, sizeof(ObjectOwnership*)));
		*pointer = userdata;

		if constexpr (std::is_base_of_v< Object, Class >) {
			luaL_setmetatable(L, thing->GetMetatable().data());
		} else {
			const std::string metatable = Utils::DemangleClassName< Class >();
			luaL_setmetatable(L, metatable.data());
		}
	}

	inline static bool Is(lua_State* L, const int32_t idx)
	{
		const VariableType type = static_cast< VariableType >(lua_type(L, idx));
		if (type == VariableType::UserData) {
			ObjectOwnership** pointer = static_cast< ObjectOwnership** >(lua_touserdata(L, idx));
			return (std::dynamic_pointer_cast< Class >((*pointer)->Get()) != nullptr);
		}

		return false;
	}
};

} // namespace Script

#endif
