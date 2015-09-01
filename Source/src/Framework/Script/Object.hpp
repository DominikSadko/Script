#ifndef FRAMEWORK_SCRIPT_OBJECT_HPP
#define FRAMEWORK_SCRIPT_OBJECT_HPP

#include <memory>

namespace Script
{

class Object
{
public:
	explicit Object() = default;
	virtual ~Object() = default;
	[[nodiscard]] virtual auto GetMetatable() const -> std::string_view = 0;
};

using ObjectPtr = std::shared_ptr< Object >;

} // namespace Script

#endif
