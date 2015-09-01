#ifndef FRAMEWORK_SCRIPT_METATABLE_HPP
#define FRAMEWORK_SCRIPT_METATABLE_HPP

#include <memory>
#include <string_view>

#include <Framework/Script/Reference.hpp>

namespace Script
{

class Engine;

class Metatable final
{
public:
	explicit Metatable(const Engine*, std::string_view metatable);
	explicit Metatable(const Engine*, std::string_view metatable, std::string_view parent);

	auto RegisterReferenceDestructor(const Engine*) -> Metatable*;

	template < typename Function >
	auto SetField(const std::string& name, Function function) -> Metatable*;

private:
	Reference mReference = {};
};

template < typename Function >
auto Metatable::SetField(const std::string& name, Function function) -> Metatable*
{
	mReference.SetField(name, function);
	return this;
}

using MetatablePtr = std::shared_ptr< Metatable >;

} // namespace Script

#endif
