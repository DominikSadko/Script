#ifndef FRAMEWORK_SCRIPT_SCRIPTENGINE_HPP
#define FRAMEWORK_SCRIPT_SCRIPTENGINE_HPP

#include <functional>
#include <string>

#include <Framework/Script/Reference.hpp>
#include <Framework/Script/Stack/Stack.hpp>

namespace Script
{

using MetatablePtr = std::shared_ptr< class Metatable >;
using SandboxPtr = std::shared_ptr< class Sandbox >;

class Engine final
{
public:
	explicit Engine();
	Engine(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine& operator=(Engine&&) = delete;
	~Engine();

	[[nodiscard]] inline auto State() const -> lua_State*;
	[[nodiscard]] inline auto IsStackTop() const -> bool;

	[[nodiscard]] auto LoadScriptFile(const std::string& filename, const char* mode = nullptr) const -> bool;
	[[nodiscard]] auto LoadScript(const std::string& script) const -> bool;
	[[nodiscard]] auto LoadScript(const char* script) const -> bool;

	[[nodiscard]] auto ExecuteRaw(const std::string& script) const -> bool;
	[[nodiscard]] auto ExecuteRaw(const char* script) const -> bool;
	[[nodiscard]] auto Execute(const std::string& script) const -> Reference;
	[[nodiscard]] auto ExecuteFile(const std::string& filename, const char* mode = nullptr) const -> bool;

	void CollectGarbage();

	template < typename Type >
	void SetGlobal(const std::string& name, const Type& value) const;
	void RemoveGlobal(const std::string& name) const;
	[[nodiscard]] auto GetGlobal() const -> Reference;
	[[nodiscard]] auto GetGlobal(const std::string& name) const -> Reference;
	[[nodiscard]] auto operator[](const std::string& name) const -> Reference;

	[[nodiscard]] auto GetMetatable(const std::string_view& name) const -> MetatablePtr;
	[[nodiscard]] auto GetMetatable(const std::string_view& name, const std::string_view& parentName) const -> MetatablePtr;
	[[nodiscard]] auto GetSandbox(const std::string_view& name) const -> SandboxPtr;

private:
	[[nodiscard]] auto Call(int32_t nargs = 0, int32_t nresults = 0, int32_t ctx = 0) const -> bool;

private:
	lua_State* L = {};
};

auto Engine::State() const -> lua_State*
{
	return L;
}

auto Engine::IsStackTop() const -> bool
{
	return (lua_gettop(L) == 0);
}

template < typename Type >
void Engine::SetGlobal(const std::string& name, const Type& value) const
{
	GetGlobal().SetField(name, value);
}

} // namespace Script

#endif
