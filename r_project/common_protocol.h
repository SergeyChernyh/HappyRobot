#ifndef __COMMON_PROTOCOL__
#define __COMMON_PROTOCOL__

#include "protocol_definition_names.h"

namespace robot { namespace common_protocol
{
    using namespace protocol_definition_names;

    struct header_key;
    struct body_key;

    struct marker_key;
    struct group_key;
    struct type_key;
    struct message_num_key;
    struct data_size_key;

    template <uint16_t MsgGroup, uint16_t MsgType>
    using message_header =
    pattern
    <
        pair<marker_key     , constant<uint16_t, 0xA5D7  >>,
        pair<group_key      , constant<uint16_t, MsgGroup>>,
        pair<type_key       , constant<uint16_t, MsgType >>,
        pair<message_num_key, uint32_t>,
        pair<data_size_key  , uint32_t>
    >;

    template <uint16_t MsgGroup, typename ...T>
    using message_group = pair<constant<uint16_t, MsgGroup>, pattern<T...>>;

    template <uint16_t MsgType, typename T>
    using message_type = pair<constant<uint16_t, MsgType>, T>;

    using common_protocol_table =
    pattern
    <
        message_group
        <
            0x0,
            message_type<0x0, pattern<uint16_t>>,
            message_type<0x1, nothing>,
            message_type<0x2, pattern<uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t>>,
            message_type<0x6, nothing>,
            message_type<0x7, pattern<uint16_t>>,
            message_type<0x8, pattern<uint32_t, uint16_t>>
        >,
        message_group
        <
            0x1,
            message_type<0x0, nothing>,
            message_type<0x1, pattern<uint64_t>>,
            message_type<0x2, nothing>,
            message_type<0x3, pattern<uint64_t>>,
            message_type<0x4, package_creation::repeat<uint32_t, pattern<uint16_t, uint16_t>>>,
            message_type<0x5, pattern<uint16_t, uint16_t>>,
            message_type<0x6, pattern<uint32_t, uint16_t, uint16_t>>,
            message_type<0x7, pattern<uint16_t, uint16_t, package_creation::repeat<uint8_t, package_creation::serialization::any>>>
        >,
        message_group
        <
            0x2,
            message_type<0x0, pattern<uint16_t, uint16_t, package_creation::repeat<uint8_t, pattern<uint8_t, uint8_t>>>>,
            message_type<0x1, pattern<uint16_t, uint16_t, package_creation::repeat<uint8_t, pattern<uint8_t, uint8_t>>>>,
            message_type<0x2, pattern<uint16_t, uint16_t, package_creation::repeat<uint8_t, pattern<uint8_t, uint8_t>>>>,
            message_type<0x3, pattern<uint16_t, uint16_t, package_creation::repeat<uint8_t, pattern<uint8_t, uint8_t>>, uint32_t>>,
            message_type<0x4, pattern<uint16_t, uint16_t, package_creation::repeat<uint8_t, uint8_t>>>,
            message_type<0x5, pattern<uint16_t, uint16_t, package_creation::repeat<uint8_t, uint8_t>>>,
            message_type<0x6, pattern<uint16_t, uint16_t, package_creation::repeat<uint8_t, package_creation::serialization::any>>>,
            message_type<0x7, pattern<uint16_t, uint16_t, package_creation::repeat<uint8_t, pattern<uint8_t, uint8_t>>>>,
            message_type<0x8, pattern<uint16_t, uint16_t, package_creation::repeat<uint8_t, package_creation::serialization::any>>>,
            message_type<0x9, nothing>,
            message_type<0x10, package_creation::serialization::any>
        >
    >;

    template <uint16_t MsgGroup, uint16_t MsgType>
    using message_body =
    metaprogramming::at_key
    <
        constant<uint16_t, MsgType>,
        metaprogramming::at_key
        <
            constant<uint16_t, MsgGroup>,
            common_protocol_table
        >
    >;

    template <uint16_t MsgGroup, uint16_t MsgType>
    using message =
    pattern
    <
        pair<header_key, message_header<MsgGroup, MsgType>>,
        pair<body_key, message_body<MsgGroup, MsgType>>
    >;
}

}
#endif //__COMMON_PROTOCOL__
