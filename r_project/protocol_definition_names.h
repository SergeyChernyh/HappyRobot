#ifndef __PROTOCOL_DEFINITION_NAMES__
#define __PROTOCOL_DEFINITION_NAMES__

#include "package.h"

namespace robot { namespace protocol_definition_names
{
    template <typename Key, typename T>
    using pair = metaprogramming::pair<Key, T>;

    template <typename ...Args>
    using pattern = package_creation::pattern<Args...>;

    template <typename ...Args>
    using pack = package_creation::package<Args...>;

    using nothing = pattern<>;

    template <typename T, T C>
    using constant = std::integral_constant<T, C>;

    using any = package_creation::serialization::any;

    template <typename SizeType, typename ValueType>
    using repeat = package_creation::repeat<SizeType, ValueType>;
}

}
#endif //__PROTOCOL_DEFINITION_NAMES__
