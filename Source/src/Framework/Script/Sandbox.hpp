#ifndef FRAMEWORK_SCRIPT_SANDBOX_HPP
#define FRAMEWORK_SCRIPT_SANDBOX_HPP

#include <memory>
#include <string_view>

#include <Framework/Script/Reference.hpp>

namespace Script
{

class Engine;

class Sandbox final
{
public:
	explicit Sandbox(const Engine*, std::string_view sandbox);

	[[nodiscard]] auto Execute(const std::string& script) -> bool;

	template < typename Function >
	auto SetField(const std::string& name, Function function) -> Sandbox*;

private:
	Reference mReference = {};
};

template < typename Function >
auto Sandbox::SetField(const std::string& name, Function function) -> Sandbox*
{
	mReference.SetField(name, function);
	return this;
}

using SandboxPtr = std::shared_ptr< Sandbox >;

} // namespace Script

#endif
