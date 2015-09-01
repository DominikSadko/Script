#ifndef FRAMEWORK_SCRIPT_STACK_STACKFUNCTION_HPP
#define FRAMEWORK_SCRIPT_STACK_STACKFUNCTION_HPP

#include <Framework/Script/Basic.hpp>
#include <Framework/Script/FunctionInvoker.hpp>
#include <Framework/Script/Stack/StackArguments.hpp>
#include <Framework/Script/Stack/StackBasic.hpp>
#include <Framework/Script/TypeTraits.hpp>

#include <functional>
#include <memory>

namespace Script
{

template <>
struct Stack< std::function< int(lua_State*) > >
{
	using Function = std::function< int(lua_State*) >;

	static void Push(lua_State* L, Function function)
	{
		const auto garbaceCollector = [](lua_State* L) {
			Function** pointer = static_cast< Function** >(lua_touserdata(L, -1));
			delete *pointer;
			*pointer = nullptr;
			return 0;
		};

		const auto call = [](lua_State* L) {
			Function** pointer = static_cast< Function** >(lua_touserdata(L, -lua_gettop(L)));
			lua_remove(L, -lua_gettop(L));
			return (**pointer)(L);
		};

		Function** pointer = static_cast< Function** >(lua_newuserdata(L, sizeof(Function)));
		*pointer = new Function{ function };

		lua_newtable(L);
		lua_pushcfunction(L, garbaceCollector);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, call);
		lua_setfield(L, -2, "__call");
		lua_setmetatable(L, -2);
	}

	inline static bool Is(lua_State*, const int32_t)
	{
		return true;
	}
};

template < typename Return, typename... Args >
struct Stack< Return (*)(Args...) >
{
	static void Push(lua_State* L, Return (*function)(Args... args))
	{
		const auto invoke = std::function{
			[ function ](Args... args) {
				if constexpr (!std::is_same_v< Return, void >) {
					return function(std::forward< Args >(args)...);
				} else {
					function(std::forward< Args >(args)...);
				}
			}
		};

		Stack< std::function< Return(Args...) > >::Push(L, invoke);
	}
};

template < class Class, typename Return, typename... Args >
struct Stack< Return (Class::*)(Args...) >
{
	static void Push(lua_State* L, Return (Class::*function)(Args...))
	{
		Stack< std::function< Return(Class*, Args...) > >::Push(L, Create(function));
	}

private:
	template < typename Function >
	[[nodiscard]] static auto Create(Function function)
	{
		const auto functionWrapper = std::mem_fn(function);
		return std::function{
			[ functionWrapper ](Class* instance, Args... args) {
				if constexpr (!std::is_same_v< Return, void >) {
					return functionWrapper(instance, std::forward< Args >(args)...);
				} else {
					functionWrapper(instance, std::forward< Args >(args)...);
				}
			}
		};
	}
};

template < class Class, typename Return, typename... Args >
struct Stack< Return (Class::*)(Args...) const >
{
	static void Push(lua_State* L, Return (Class::*function)(Args...) const)
	{
		Stack< std::function< Return(Class*, Args...) > >::Push(L, Create(function));
	}

private:
	template < typename Function >
	[[nodiscard]] static auto Create(Function function)
	{
		const auto functionWrapper = std::mem_fn(function);
		return std::function{
			[ functionWrapper ](Class* instance, Args... args) {
				if constexpr (!std::is_same_v< Return, void >) {
					return functionWrapper(instance, std::forward< Args >(args)...);
				} else {
					functionWrapper(instance, std::forward< Args >(args)...);
				}
			}
		};
	}
};

template < typename Ret, typename... Args >
struct Stack< std::function< Ret(Args...) > >
{
	using Function = std::function< Ret(Args...) >;
	using Tuple = typename std::tuple< typename TypeTraits::RemoveConstReference< Args >::Type... >;

	static Function Get(lua_State* L, const int32_t idx)
	{
		lua_pushvalue(L, idx);
		const int32_t reference = Utils::StrongRefSet(L);

		Function* launcher = new Function{
			[ L, reference ](Args&&... args) {
				Utils::StrongRefGet(L, reference);
				Stack< void >::Push(L, std::forward< Args >(args)...);

				if constexpr (!std::is_same_v< Ret, void >) {
					if (lua_pcall(L, sizeof...(Args), 1, 0)) {
						throw std::string("<Script::Stack::Get> ") + lua_tostring(L, -1);
					}

					Ret ret = Stack< Ret >::Get(L, -1);
					lua_pop(L, 1);
					return ret;
				} else {
					if (lua_pcall(L, sizeof...(Args), 0, 0)) {
						throw std::string("<Script::Stack::Get> ") + lua_tostring(L, -1);
					}
				}
			}
		};

		const auto removeReference = [ L, reference ](const Function* launcher) {
			Utils::StrongUnref(L, reference);
			delete launcher;
		};

		const std::shared_ptr< Function > function = {
			launcher,
			removeReference
		};

		return [ function ](Args&&... args) {
			return (*function.get())(args...);
		};
	}

	static void Push(lua_State* L, Function function)
	{
		const auto garbaceCollector = [](lua_State* L) {
			Function** pointer = static_cast< Function** >(lua_touserdata(L, -1));
			delete *pointer;
			*pointer = nullptr;
			return 0;
		};

		const auto call = [](lua_State* L) {
			const size_t topStack = static_cast< size_t >(lua_gettop(L));
			const size_t topArgsNum = std::min(topStack, sizeof...(Args));

			Tuple tuple;
			Script::StackArguments< sizeof...(Args) >(L, tuple, topArgsNum);
			lua_pop(L, topArgsNum);

			Function** pointer = static_cast< Function** >(lua_touserdata(L, -1));

			const auto invoker = std::function< int(Args...) >([ L, pointer ](Args... args) {
				if constexpr (!std::is_same_v< Ret, void >) {
					Stack< Ret >::Push(L, (**pointer)(args...));
					return 1;
				} else {
					(void)L;
					(**pointer)(args...);
					return 0;
				}
			});

			return FunctionInvoker::Call< int32_t >(invoker, std::forward< Tuple >(tuple));
		};

		Function** pointer = static_cast< Function** >(lua_newuserdata(L, sizeof(Function)));
		*pointer = new Function{ function };

		lua_newtable(L);
		lua_pushcfunction(L, garbaceCollector);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, call);
		lua_setfield(L, -2, "__call");
		lua_setmetatable(L, -2);
	}

	inline static bool Is(lua_State*, const int32_t)
	{
		return true;
	}
};

} // namespace Script

#endif
