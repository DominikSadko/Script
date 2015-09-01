#ifndef FRAMEWORK_SCRIPT_VARIABLETYPE_HPP
#define FRAMEWORK_SCRIPT_VARIABLETYPE_HPP

#include <cstdint>

namespace Script
{

enum class VariableType : uint8_t {
	Nil = 0,
	Boolean = 1,
	LightUserData = 2,
	Number = 3,
	String = 4,
	Table = 5,
	Function = 6,
	UserData = 7,
	Thread = 8,
};

} // namespace Script

#endif
