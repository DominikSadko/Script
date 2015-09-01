#ifndef FRAMEWORK_SCRIPT_FUNCTIONINVOKER_HPP
#define FRAMEWORK_SCRIPT_FUNCTIONINVOKER_HPP

#include <cstddef>
#include <tuple>

namespace Script
{

struct FunctionInvoker
{
	template < class Return, typename Arguments, typename Function, bool Done, size_t Total, size_t... N >
	struct ParametersForwarder
	{
		inline static auto Call(Function& function, Arguments&& arguments) -> Return
		{
			using Forwarder = ParametersForwarder< Return, Arguments, Function, Total == 1 + sizeof...(N), Total, N..., sizeof...(N) >;
			return Forwarder::Call(function, std::forward< Arguments >(arguments));
		}
	};

	template < class Return, typename Arguments, typename Function, size_t Total, size_t... N >
	struct ParametersForwarder< Return, Arguments, Function, true, Total, N... >
	{
		inline static auto Call(Function& function, Arguments&& arguments) -> Return
		{
			return function(std::get< N >(std::forward< Arguments >(arguments))...);
		}
	};

	template < class Return, typename Arguments, typename Function >
	inline static auto Call(Function& function, Arguments&& arguments) -> Return
	{
		using Type = typename std::decay< Arguments >::type;
		using Forwarder = ParametersForwarder< Return, Arguments, Function, 0 == std::tuple_size< Type >::value, std::tuple_size< Type >::value >;
		return Forwarder::Call(function, std::forward< Arguments >(arguments));
	}
};

} // namespace Script

#endif
