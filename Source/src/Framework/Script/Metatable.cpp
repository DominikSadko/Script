#include <Framework/Script/Metatable.hpp>

#include <Framework/Script/Engine.hpp>
#include <Framework/Script/ObjectOwnership.hpp>

namespace Script
{

Metatable::Metatable(const Engine* engine, std::string_view metatable)
	: Metatable(engine, metatable, std::string{})
{
}

Metatable::Metatable(const Engine* engine, std::string_view metatable, std::string_view parent)
{
	mReference = engine->GetGlobal(metatable.data());
	if (mReference.GetType() != Script::VariableType::Nil) {
		return;
	}

	lua_State* L = engine->State();

	luaL_newmetatable(L, metatable.data());
	lua_pushvalue(L, -1);

	lua_setfield(L, -2, "__index");

	if (!parent.empty()) {
		luaL_getmetatable(L, parent.data());

		lua_pushvalue(L, -1);
		lua_setmetatable(L, -3);

		{
			const std::vector< std::string_view > OverloadOperators = {
				"__eq",
				"__add",
				"__sub",
				"__mul",
				"__div",
				"__mod",
				"__pow",
				"__unm",
				"__len",
				"__lt",
				"__le",
				"__concat",
				"__call",
				"__tostring",
			};

			for (const std::string_view& copyOperator : OverloadOperators) {
				lua_getfield(L, -2, copyOperator.data());
				lua_setfield(L, -3, copyOperator.data());
			}
		}

		lua_pop(L, 1);
	}

	lua_setglobal(L, metatable.data());
	mReference = engine->GetGlobal(metatable.data());
}

auto Metatable::RegisterReferenceDestructor(const Engine*) -> Metatable*
{
	const auto GarbageCollector = std::function([](lua_State* L) {
		const VariableType type = static_cast< VariableType >(lua_type(L, -1));

		if (type == VariableType::UserData) {
			ObjectOwnership** pointer = static_cast< ObjectOwnership** >(lua_touserdata(L, -1));
			delete *pointer;
			*pointer = nullptr;

		} else if (type == VariableType::LightUserData) {
			throw std::string{ "<Script::Metatable::RegisterReferenceDestructor> Invalid collector type: 'LightUserData'" };
		}

		return 0;
	});

	mReference.SetField("__gc", GarbageCollector);
	return this;
}

} // namespace Script
