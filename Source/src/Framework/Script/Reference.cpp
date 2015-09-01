#include <Framework/Script/Reference.hpp>

namespace Script
{

Reference::Reference(lua_State* L, const std::string& name)
{
	if (name.empty()) {
		lua_pushvalue(L, LUA_GLOBALSINDEX);
	} else {
		lua_getglobal(L, name.c_str());
	}

	mPointer = PointerPtr{
		new Pointer{
			.L = L,
			.id = Utils::StrongRefSet(L),
		},
	};
}

Reference::Reference(lua_State* L, const int32_t idx, const bool popFromStack)
{
	lua_pushvalue(L, idx);

	mPointer = PointerPtr{
		new Pointer{
			.L = L,
			.id = Utils::StrongRefSet(L),
		},
	};

	if (popFromStack) {
		lua_pop(L, 1);
	}
}

auto Reference::GetType() const -> VariableType
{
	lua_State* L = mPointer->L;
	Push();

	VariableType type = static_cast< VariableType >(lua_type(L, -1));
	if (type == VariableType::UserData) {
		lua_getmetatable(L, -1);
		if (static_cast< VariableType >(lua_type(L, -1)) == VariableType::Table) {
			lua_getfield(L, -1, "__call");
			if (static_cast< VariableType >(lua_type(L, -1)) == VariableType::Function) {
				type = VariableType::Function;
			}
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
	return type;
}

Reference::operator bool() const
{
	return (mPointer && mPointer->id != LUA_REFNIL);
}

auto Reference::operator!() const -> bool
{
	return (!mPointer || mPointer->id == LUA_REFNIL);
}

auto Reference::CallSandbox(const std::string& script) const -> bool
{
	lua_State* L = mPointer->L;
	Push();

	if (luaL_loadstring(L, script.c_str())) {
		return false;
	}

	lua_pushvalue(L, -2);
	lua_setfenv(L, -2);
	lua_call(L, 0, 0);

	lua_pop(L, 1);
	return true;
}

void Reference::Push() const
{
	Utils::StrongRefGet(mPointer->L, mPointer->id);
}

} // namespace Script
