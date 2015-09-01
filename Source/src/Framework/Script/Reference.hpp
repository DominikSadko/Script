#ifndef FRAMEWORK_SCRIPT_REFERENCE_HPP
#define FRAMEWORK_SCRIPT_REFERENCE_HPP

#include <Framework/Script/Basic.hpp>
#include <Framework/Script/Stack/StackBasic.hpp>
#include <Framework/Script/VariableType.hpp>

#include <memory>

namespace Script
{

class Reference final
{
public:
	Reference() = default;
	Reference(const Reference&) = default;
	Reference(Reference&&) = default;
	Reference& operator=(const Reference&) = default;
	Reference& operator=(Reference&&) = default;
	explicit Reference(lua_State*, const std::string& name);
	explicit Reference(lua_State*, const int32_t idx, bool popFromStack);
	~Reference()
	{
		if (mPointer && mPointer.use_count() <= 1) {
			if (mPointer->id != LUA_REFNIL) {
				Utils::StrongUnref(mPointer->L, mPointer->id);
			}
		}
	}

	[[nodiscard]] auto GetType() const -> VariableType;

	explicit operator bool() const;
	auto operator!() const -> bool;

	template < typename... Args >
	auto operator()(Args&&... args) const -> Reference;

	template < typename Value >
	auto operator=(const Value& value) -> Reference&;

	[[nodiscard]] auto CallSandbox(const std::string& script) const -> bool;

	template < typename Return >
	[[nodiscard]] auto Get() const -> Return;

	template < typename Key, typename Ret, typename... Args >
	void SetField(const Key& key, Ret (*function)(Args... args));

	template < typename Key, typename Value >
	void SetField(const Key& key, const Value& value);

	template < typename Field >
	auto operator[](const Field& field) const -> Reference;

	void Push() const;

	[[nodiscard]] inline auto GetId() const -> int32_t;

private:
	struct Pointer
	{
		lua_State* L = {};
		int32_t id = LUA_REFNIL;
	};

	using PointerPtr = std::shared_ptr< struct Pointer >;

	PointerPtr mPointer = {};
};

template <>
struct Stack< Reference >
{
	static auto Get(lua_State* L, const int32_t idx) -> Reference
	{
		return Reference{ L, idx, false };
	}

	static void Push(lua_State* L, const Reference& reference)
	{
		if (reference) {
			const int32_t id = reference.GetId();
			if (id != LUA_REFNIL) {
				Utils::StrongRefGet(L, id);
				return;
			}
		}
		lua_pushnil(L);
	}

	static auto Is(lua_State*, const int32_t) -> bool
	{
		return true;
	}
};

template < typename... Args >
auto Reference::operator()(Args&&... args) const -> Reference
{
	lua_State* L = mPointer->L;

	Push();
	if (!lua_isfunction(L, -1)) {
		if (static_cast< VariableType >(lua_type(L, -1)) == VariableType::UserData) {
			lua_getmetatable(L, -1);
			if (static_cast< VariableType >(lua_type(L, -1)) == VariableType::Table) {
				lua_getfield(L, -1, "__call");
				lua_remove(L, 2);
				lua_remove(L, 2);
			} else {
				throw std::string("<Script::Reference::Call> is not metatable function");
			}
		} else {
			throw std::string("<Script::Reference::Call> is not userdata function");
		}
	}

	Stack< void >::Push(L, std::forward< Args >(args)...);

	if (lua_pcall(L, sizeof...(Args), 1, 0)) {
		throw std::string("<Script::Reference::Call> ") + lua_tostring(L, -1);
	}

	const Reference reference = Stack< Reference >::Get(L, -1);
	lua_pop(L, 1);
	return reference;
}

template < typename Value >
auto Reference::operator=(const Value& value) -> Reference&
{
	lua_State* L = mPointer->L;
	Push();
	Stack< Value >::Push(L, value);

	mPointer = PointerPtr{
		new Pointer{
			.L = L,
			.id = Utils::StrongRefSet(L),
		},
	};
	lua_pop(L, 1);
	return *this;
}

template < typename Return >
auto Reference::Get() const -> Return
{
	lua_State* L = mPointer->L;
	Push();
	Return ret = Stack< Return >::Get(L, -1);
	lua_pop(L, 1);
	return ret;
}

template < typename Key, typename Ret, typename... Args >
void Reference::SetField(const Key& key, Ret (*function)(Args... args))
{
	lua_State* L = mPointer->L;
	Push();

	Stack< Key >::Push(L, key);
	Stack< Ret (*)(Args...) >::Push(L, function);

	lua_settable(L, -3);
	lua_pop(L, 1);
}

template < typename Key, typename Value >
void Reference::SetField(const Key& key, const Value& value)
{
	lua_State* L = mPointer->L;
	Push();

	Stack< Key >::Push(L, key);
	Stack< Value >::Push(L, value);

	lua_settable(L, -3);
	lua_pop(L, 1);
}

template < typename Field >
auto Reference::operator[](const Field& field) const -> Reference
{
	lua_State* L = mPointer->L;

	Push();

	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return {};
	}

	Stack< Field >::Push(L, field);
	lua_gettable(L, -2);
	lua_remove(L, -2);
	return Reference{ L, -1, true };
}

auto Reference::GetId() const -> int32_t
{
	return mPointer->id;
}

} // namespace Script

#endif
