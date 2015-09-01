#ifndef FRAMEWORK_SCRIPT_STACK_STACKBASIC_HPP
#define FRAMEWORK_SCRIPT_STACK_STACKBASIC_HPP

extern "C" {
#include <lauxlib.h>
}

#include <Framework/Script/TypeTraits.hpp>

#include <optional>
#include <string>
#include <variant>

struct lua_State;

namespace Script
{

template < class Type, class Enable = void >
struct Stack
{ };

template <>
struct Stack< void >
{
	static void Push(lua_State*) { }

	template < typename Type, typename... Args >
	static void Push(lua_State* L, const Type& value, Args&&... args)
	{
		Stack< Type >::Push(L, value);
		Stack< void >::Push(L, std::forward< Args >(args)...);
	}
};

template <>
struct Stack< std::nullptr_t >
{
	static std::nullptr_t Get(lua_State*, const int32_t) { return nullptr; }
	static void Push(lua_State* L, const std::nullptr_t) { lua_pushnil(L); }
	static bool Is(lua_State* L, const int32_t idx) { return (lua_type(L, idx) == LUA_TNIL); }
};

template <>
struct Stack< std::monostate >
{
	static std::monostate Get(lua_State*, const int32_t) { return std::monostate{}; }
	static void Push(lua_State* L, const std::monostate) { lua_pushnil(L); }
	static bool Is(lua_State* L, const int32_t idx) { return (lua_type(L, idx) == LUA_TNIL); }
};

////////////////      Integers     ////////////////

template <>
struct Stack< bool >
{
	static bool Get(lua_State* L, const int32_t idx) { return lua_toboolean(L, idx); }
	static void Push(lua_State* L, const bool value) { lua_pushboolean(L, value); }
	static bool Is(lua_State* L, const int32_t idx) { return (lua_type(L, idx) == LUA_TBOOLEAN); }
};

template < class Type >
struct Stack< Type, typename std::enable_if_t< TypeTraits::IsInteger< Type >::value > >
{
	static Type Get(lua_State* L, const int32_t idx) { return lua_tointeger(L, idx); }
	static void Push(lua_State* L, const Type value) { lua_pushinteger(L, value); }
	static bool Is(lua_State* L, const int32_t idx) { return (lua_type(L, idx) == LUA_TNUMBER); }
};

template < class Type >
struct Stack< Type, std::enable_if_t< std::is_floating_point_v< Type > > >
{
	static Type Get(lua_State* L, const int32_t idx) { return static_cast< Type >(lua_tonumber(L, idx)); }
	static void Push(lua_State* L, const Type value) { lua_pushnumber(L, static_cast< Type >(value)); }
	static bool Is(lua_State* L, const int32_t idx) { return (lua_type(L, idx) == LUA_TNUMBER); }
};

////////////////      Strings     ////////////////
template < std::size_t Size >
struct Stack< char[ Size ] >
{
	static const char* Get(lua_State* L, const int32_t idx) { return lua_tolstring(L, idx, nullptr); }
	static void Push(lua_State* L, const char* value) { lua_pushstring(L, value); }
	static bool Is(lua_State* L, const int32_t idx) { return (lua_type(L, idx) == LUA_TSTRING); }
};

template < class Type >
struct Stack< Type, typename std::enable_if_t< TypeTraits::IsCharPointer< Type >::value > >
{
	static const char* Get(lua_State* L, const int32_t idx) { return lua_tolstring(L, idx, nullptr); }
	static void Push(lua_State* L, const char* value) { lua_pushstring(L, value); }
	static bool Is(lua_State* L, const int32_t idx) { return (lua_type(L, idx) == LUA_TSTRING); }
};

template <>
struct Stack< std::string >
{
	static std::string Get(lua_State* L, const int32_t idx) { return std::string{ lua_tolstring(L, idx, nullptr) }; }
	static void Push(lua_State* L, const std::string& value) { lua_pushstring(L, value.c_str()); }
	static bool Is(lua_State* L, const int32_t idx) { return (lua_type(L, idx) == LUA_TSTRING); }
};

template <>
struct Stack< std::string_view >
{
	static std::string Get(lua_State* L, const int32_t idx) { return std::string{ lua_tolstring(L, idx, nullptr) }; }
	static void Push(lua_State* L, const std::string_view& value) { lua_pushstring(L, value.data()); }
	static bool Is(lua_State* L, const int32_t idx) { return (lua_type(L, idx) == LUA_TSTRING); }
};

template < class EnumType >
struct Stack< EnumType, typename std::enable_if_t< std::is_enum< EnumType >::value > >
{
	static EnumType Get(lua_State* L, const int32_t idx) { return static_cast< EnumType >(lua_tointeger(L, idx)); }
	static void Push(lua_State* L, const EnumType value) { lua_pushinteger(L, static_cast< int32_t >(value)); }
	static bool Is(lua_State* L, const int32_t idx) { return (lua_type(L, idx) == LUA_TBOOLEAN); }
};

template < class ValueType >
struct Stack< std::optional< ValueType > >
{
	static std::optional< ValueType > Get(lua_State* L, const int32_t idx)
	{
		if (lua_isnil(L, idx)) {
			return {};
		}

		return Stack< ValueType >::Get(L, idx);
	}

	static void Push(lua_State* L, const std::optional< ValueType >& value)
	{
		if (value) {
			return Stack< ValueType >::Push(L, value.value());
		}

		return lua_pushnil(L);
	}

	static bool Is(lua_State* L, const int32_t idx)
	{
		return ((lua_type(L, idx) == LUA_TNIL) || Stack< ValueType >::Is(L, idx));
	}
};

template < class Pair >
struct Stack< Pair, typename std::enable_if_t< TypeTraits::IsTemplateBase< std::remove_const_t< Pair >, std::pair >::value > >
{
	using FirstType = typename Pair::first_type;
	using SecondType = typename Pair::second_type;

	static Pair Get(lua_State* L, const int32_t idx)
	{
		Pair pair = {};

		lua_pushinteger(L, 1);
		lua_gettable(L, idx - 1);
		if (!lua_isnil(L, -1)) {
			pair.first = Stack< FirstType >::Get(L, -1);
		}
		lua_pop(L, 1);

		lua_pushinteger(L, 2);
		lua_gettable(L, idx - 1);
		if (!lua_isnil(L, -1)) {
			pair.second = Stack< SecondType >::Get(L, -1);
		}
		lua_pop(L, 1);

		return pair;
	}

	static void Push(lua_State* L, const Pair& pair)
	{
		lua_newtable(L);

		Stack< int32_t >::Push(L, 1);
		Stack< FirstType >::Push(L, pair.first);
		lua_settable(L, -3);

		Stack< int32_t >::Push(L, 2);
		Stack< SecondType >::Push(L, pair.second);
		lua_settable(L, -3);
	}

	static bool Is(lua_State* L, const int32_t idx) { return lua_istable(L, idx); }
};

template < typename... Args >
struct Stack< std::variant< Args... > >
{
	template < size_t Size >
	struct VariantAssigner
	{
		VariantAssigner(lua_State* L, std::variant< Args... >& variant, const int32_t idx)
		{
			(void)L;
			(void)variant;
			(void)idx;

			if constexpr (Size > 0) {
				if (!Get(L, variant, idx)) {
					VariantAssigner< Size - 1 >(L, variant, idx);
				}
			}
		}

		VariantAssigner(bool& is, lua_State* L, const int32_t idx)
		{
			(void)is;
			(void)L;
			(void)idx;

			if constexpr (Size > 0) {
				if (!Is(L, idx)) {
					VariantAssigner< Size - 1 >(is, L, idx);
				} else {
					is = true;
				}
			}
		}

	private:
		static bool Get(lua_State* L, std::variant< Args... >& variant, const int32_t idx)
		{
			using VariantType = std::variant_alternative_t<
				std::variant_size< std::variant< Args... > >::value - Size,
				std::variant< Args... > >;
			if (!Stack< VariantType >::Is(L, idx)) {
				return false;
			}
			variant = Stack< VariantType >::Get(L, idx);
			return true;
		}

		static bool Is(lua_State* L, const int32_t idx)
		{
			using VariantType = std::variant_alternative_t<
				std::variant_size< std::variant< Args... > >::value - Size,
				std::variant< Args... > >;
			return Stack< VariantType >::Is(L, idx);
		}
	};

	static std::variant< Args... > Get(lua_State* L, const int32_t idx)
	{
		std::variant< Args... > result;
		VariantAssigner< std::variant_size< std::variant< Args... > >::value >{ L, result, idx };
		return result;
	}

	template < class... Ts >
	struct Overload : Ts...
	{
		using Ts::operator()...;
	};
	template < class... Ts >
	Overload(Ts...) -> Overload< Ts... >;

	static void Push(lua_State* L, const std::variant< Args... >& value)
	{
		std::visit(
			Overload{
				[ &L ](auto&& arg) {
					Stack< std::decay_t< decltype(arg) > >::Push(L, arg);
				},
			},
			value);
	}

	static bool Is(lua_State* L, const int32_t idx)
	{
		bool is = false;
		VariantAssigner< std::variant_size< std::variant< Args... > >::value >{ is, L, idx };
		return is;
	}
};

} // namespace Script

#endif
