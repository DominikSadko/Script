#ifndef FRAMEWORK_SCRIPT_OBJECTOWNERSHIP_HPP
#define FRAMEWORK_SCRIPT_OBJECTOWNERSHIP_HPP

#include <memory>

namespace Script
{

using ObjectPtr = std::shared_ptr< class Object >;

class ObjectOwnership final
{
public:
	inline explicit ObjectOwnership(ObjectPtr object);

	[[nodiscard]] inline auto Get() const -> ObjectPtr;

private:
	const ObjectPtr mObject = {};
};

ObjectOwnership::ObjectOwnership(ObjectPtr object)
	: mObject(std::move(object))
{
}

auto ObjectOwnership::Get() const -> ObjectPtr
{
	return mObject;
}

} // namespace Script

#endif
