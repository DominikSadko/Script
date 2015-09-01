#ifndef FRAMEWORK_SCRIPT_STACKARGUMENTS_HPP
#define FRAMEWORK_SCRIPT_STACKARGUMENTS_HPP

#include <Framework/Script/Stack/StackBasic.hpp>

#include <cstdint>
#include <type_traits>

namespace Script
{

template < size_t Size >
struct StackArguments
{
	template < typename Tuple >
	StackArguments(lua_State* L, Tuple& tuple, const int32_t topArgsNum)
	{
		const int32_t currentArgIndex = static_cast< int32_t >(std::tuple_size< Tuple >::value - Size);

		if (topArgsNum > currentArgIndex) {
			Get(L, std::get< std::tuple_size< Tuple >::value - Size >(tuple), (-topArgsNum + currentArgIndex));
		}

		StackArguments< Size - 1 >(L, tuple, topArgsNum);
	}

private:
	template < typename T >
	inline static void Get(lua_State* L, T& value, const int32_t idx)
	{
		value = Stack< T >::Get(L, idx);
	}
};

template <>
struct StackArguments< 0 >
{
	template < typename Tuple >
	StackArguments(lua_State*, Tuple&, const int32_t)
	{ }
};

} // namespace Script

#endif
