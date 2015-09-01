#ifndef FRAMEWORK_SCRIPT_TYPETRAITS_HPP
#define FRAMEWORK_SCRIPT_TYPETRAITS_HPP

#include <cstdint>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Script::TypeTraits
{

template < typename T >
struct RemoveConstReference
{
	using Type = typename std::remove_const< typename std::remove_reference< T >::type >::type;
};

template < class Template, template < typename... > class Type >
struct IsTemplateBase : std::false_type
{ };

template < class... Template, template < typename... > class Type >
struct IsTemplateBase< Type< Template... >, Type > : std::true_type
{ };

template < class Container >
using IsValueContainerSequenced = std::disjunction<
	TypeTraits::IsTemplateBase< Container, std::deque >,
	TypeTraits::IsTemplateBase< Container, std::list >,
	TypeTraits::IsTemplateBase< Container, std::vector > >;

template < class Container >
using IsValueContainerAssociatived = std::disjunction<
	TypeTraits::IsTemplateBase< Container, std::set >,
	TypeTraits::IsTemplateBase< Container, std::multiset >,
	TypeTraits::IsTemplateBase< Container, std::unordered_set > >;

template < class Container >
using IsMapContainerAssociatived = std::disjunction<
	TypeTraits::IsTemplateBase< Container, std::map >,
	TypeTraits::IsTemplateBase< Container, std::multimap >,
	TypeTraits::IsTemplateBase< Container, std::unordered_map >,
	TypeTraits::IsTemplateBase< Container, std::unordered_multimap > >;

template < class Type >
using IsInteger = std::disjunction<
	std::is_same< Type, uint8_t >,
	std::is_same< Type, int8_t >,
	std::is_same< Type, uint16_t >,
	std::is_same< Type, int16_t >,
	std::is_same< Type, uint32_t >,
	std::is_same< Type, int32_t >,
	std::is_same< Type, uint64_t >,
	std::is_same< Type, int64_t > >;

template < class Type >
using IsCharPointer = std::disjunction<
	std::is_same< Type, char* >,
	std::is_same< Type, const char* > >;

} // namespace Script::TypeTraits

#endif
