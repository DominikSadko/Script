#ifndef FRAMEWORK_SCRIPT_STACK_STACKCONTAINER_HPP
#define FRAMEWORK_SCRIPT_STACK_STACKCONTAINER_HPP

#include <Framework/Script/Stack/StackBasic.hpp>
#include <Framework/Script/TypeTraits.hpp>

namespace Script
{

template < class Container >
struct Stack< Container, typename std::enable_if_t< TypeTraits::IsValueContainerSequenced< Container >::value > >
{
	using ValueType = typename Container::value_type;

	static Container Get(lua_State* L, const int32_t idx)
	{
		Container container = {};

		lua_pushnil(L);

		while (lua_next(L, idx - 1)) {
			lua_pushvalue(L, -2);

			if (Stack< int32_t >::Is(L, -1) && Stack< ValueType >::Is(L, -2)) {
				container.push_back(Stack< ValueType >::Get(L, -2));
			}

			lua_pop(L, 2);
		}

		return container;
	}

	static void Push(lua_State* L, Container container)
	{
		lua_newtable(L);

		int32_t id = 0;

		for (const ValueType& it : container) {
			Stack< int32_t >::Push(L, (++id));
			Stack< ValueType >::Push(L, it);
			lua_settable(L, -3);
		}
	}

	static bool Is(lua_State* L, const int32_t idx) { return lua_istable(L, idx); }
};

template < class Container >
struct Stack< Container, typename std::enable_if_t< TypeTraits::IsValueContainerAssociatived< Container >::value > >
{
	using ValueType = typename Container::value_type;

	static Container Get(lua_State* L, const int32_t idx)
	{
		Container container = {};

		lua_pushnil(L);

		while (lua_next(L, idx - 1)) {
			lua_pushvalue(L, -2);

			if (Stack< int32_t >::Is(L, -1) && Stack< ValueType >::Is(L, -2)) {
				container.insert(Stack< ValueType >::Get(L, -2));
			}

			lua_pop(L, 2);
		}

		return container;
	}

	static void Push(lua_State* L, const Container& value)
	{
		lua_newtable(L);

		int32_t id = 0;

		for (const ValueType& it : value) {
			Stack< int32_t >::Push(L, (++id));
			Stack< ValueType >::Push(L, it);
			lua_settable(L, -3);
		}
	}

	static bool Is(lua_State* L, const int32_t idx)
	{
		return lua_istable(L, idx);
	}
};

template < class Container >
struct Stack< Container, typename std::enable_if_t< TypeTraits::IsMapContainerAssociatived< Container >::value > >
{
	using KeyType = typename Container::key_type;
	using ValueType = typename Container::mapped_type;

	static Container Get(lua_State* L, const int32_t idx)
	{
		Container container = {};

		lua_pushnil(L);

		while (lua_next(L, idx - 1)) {
			lua_pushvalue(L, -2);
			container.emplace(Stack< KeyType >::Get(L, -1), Stack< ValueType >::Get(L, -2));
			lua_pop(L, 2);
		}

		return container;
	}

	static void Push(lua_State* L, const Container& container)
	{
		lua_newtable(L);

		for (const std::pair< KeyType, ValueType >& it : container) {
			Stack< KeyType >::Push(L, it.first);
			Stack< ValueType >::Push(L, it.second);
			lua_settable(L, -3);
		}
	}

	static bool Is(lua_State* L, const int32_t idx) { return lua_istable(L, idx); }
};

} // namespace Script

#endif
