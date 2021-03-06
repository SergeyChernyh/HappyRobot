#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <functional>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <limits>

#include <boost/signals2.hpp>

#include "dimension.h"
#include "connection.h"

namespace robot
{
///////////////////////////////////////////////////////////
//
//                        misc
//
///////////////////////////////////////////////////////////

// exceptions

struct parameter_access_error: public std::logic_error
{
    uint8_t p_code;

    parameter_access_error(uint8_t p):
        std::logic_error("error: param rw op acess error"),
        p_code(p)
    {}
};

///////////////////////////////////////////////////////////
//
//                   Common protocol
//
///////////////////////////////////////////////////////////

namespace common_protocol
{
struct header_key;
struct body_key;

struct marker_key;
struct group_key;
struct type_key;
struct message_num_key;
struct data_size_key;

template <uint16_t C>
using uint16_constant = std::integral_constant<uint16_t, C>;

using message_header =
std::tuple
<
    pair<marker_key     , uint16_constant<0xA5D7>>,
    pair<group_key      , uint16_t>,
    pair<type_key       , uint16_t>,
    pair<message_num_key, uint32_t>,
    pair<data_size_key  , uint32_t>
>;

using service_group_key     = uint16_constant<0x0>;
using config_group_key      = uint16_constant<0x1>;
using data_access_group_key = uint16_constant<0x2>;

///////////////// service group ///////////////////////////

using protocol_version_key = uint16_constant<0x0>;
using protocol_version = uint16_t;
//
using active_connections_request_key = uint16_constant<0x1>;
using active_connections_request = std::tuple<>;
//
using active_connections_info_key = uint16_constant<0x2>;

struct num_of_control_slots_key;
struct num_of_free_control_slots_key;
struct max_control_prior_key;
struct min_control_prior_key;
struct num_of_monitor_slots_key;
struct num_of_free_monitor_slots_key;
struct max_monitor_prior_key;
struct min_monitor_prior_key;

using active_connections_info =
std::tuple
<
    pair<num_of_control_slots_key     , uint8_t>,
    pair<num_of_free_control_slots_key, uint8_t>,
    pair<max_control_prior_key        , uint8_t>,
    pair<min_control_prior_key        , uint8_t>,
    pair<num_of_monitor_slots_key     , uint8_t>,
    pair<num_of_free_monitor_slots_key, uint8_t>,
    pair<max_monitor_prior_key        , uint8_t>,
    pair<min_monitor_prior_key        , uint8_t>
>;
//
using control_level_up_request_key = uint16_constant<0x3>;
using control_level_up_request = std::tuple<>;
//
using control_level_activation_request_key = uint16_constant<0x4>;
using control_level_activation_request = std::tuple<>;
//
using control_level_deactivation_request_key = uint16_constant<0x5>;
using control_level_deactivation_request = std::tuple<>;
//
using disconnect_request_key = uint16_constant<0x6>;
using disconnect_request = std::tuple<>;
//
using disconnect_code_key = uint16_constant<0x7>;
using disconnect_code = uint16_t;
//
using command_return_code_key = uint16_constant<0x8>;

struct command_num_key;
struct return_code_key;

using command_return_code =
std::tuple
<
    pair<command_num_key, uint32_t>,
    pair<return_code_key, uint16_t>
>;

///////////////// config group ////////////////////////////

// function map utils
struct f_code_key;
struct f_number_key;

using function_id_t =
std::tuple
<
    uint16_t, // code
    uint16_t  // number
>;
//
using config_version_request_key = uint16_constant<0x0>;
using config_version_request = std::tuple<>;
//
using config_version_key = uint16_constant<0x1>;
using config_version = uint64_t;
//
using function_list_request_key = uint16_constant<0x2>;
using function_list_request = std::tuple<>;
//
using diff_function_list_request_key = uint16_constant<0x3>;
using diff_function_list_request = uint64_t;
//
using function_list_key = uint16_constant<0x4>;
using function_list = repeat<uint32_t, function_id_t>;
//
using function_config_request_key = uint16_constant<0x5>;
using function_config_request = function_id_t;
//
using function_diff_config_request_key = uint16_constant<0x6>;
using function_diff_config_request = std::tuple<uint64_t, function_id_t>;
//
using function_config_key = uint16_constant<0x7>;
using function_config = std::tuple<function_id_t, repeat<uint8_t, any>>;

///////////////// data access /////////////////////////////
//
using function_value_read_request_key = uint16_constant<0x0>;
using function_value_read_request =
std::tuple
<
    function_id_t, 
    repeat
    <
        uint8_t,
        std::tuple
        <
            uint8_t,
            uint8_t
        >
    >
>;
//
using function_value_read_on_update_1_time_request_key = uint16_constant<0x1>;
using function_value_read_on_update_1_time_request =
function_value_read_request;
//
using function_value_read_on_update_request_key = uint16_constant<0x2>;
using function_value_read_on_update_request = function_value_read_request;
//
using function_value_read_periodical_request_key = uint16_constant<0x3>;
using function_value_read_periodical_request =
std::tuple
<
    function_id_t, 
    repeat<uint8_t, std::tuple<uint8_t, uint8_t>>,
    uint32_t
>;
//
using function_value_update_cancel_key = uint16_constant<0x4>;
using function_value_update_cancel =
std::tuple
<
    function_id_t,
    repeat<uint8_t, uint8_t>
>;
//
using function_value_periodical_update_cancel_key = uint16_constant<0x5>;
using function_value_periodical_update_cancel = function_value_update_cancel;
//
using function_value_read_key = uint16_constant<0x6>;
using function_value_read =
std::tuple
<
    function_id_t,
    repeat
    <
        uint8_t,
        std::tuple
        <
            uint8_t,
            std::tuple
            <
                any,
                uint8_t
            >
        >
    >
>;
//
using function_value_read_denied_key = uint16_constant<0x7>;
using function_value_read_denied =
repeat
<
    uint8_t,
    std::tuple
    <
        uint8_t,
        uint8_t
    >
>;
//
using function_value_write_key = uint16_constant<0x8>;
using function_value_write =
std::tuple
<
    function_id_t,
    repeat
    <
        uint8_t,
        std::tuple
        <
            uint8_t,
            any
        >
    >
>;
//
using labels_format_request_key = uint16_constant<0x9>;
using labels_format_request = std::tuple<>;
//
using labels_format_key = uint16_constant<0xA>;
using labels_format = std::tuple<any>;

#define MSG_TYPE(NAME) pair<NAME##_key, NAME>

using service_group =
std::tuple
<
    MSG_TYPE(protocol_version),
    MSG_TYPE(active_connections_request),
    MSG_TYPE(active_connections_info),
    MSG_TYPE(control_level_up_request),
    MSG_TYPE(control_level_activation_request),
    MSG_TYPE(control_level_deactivation_request),
    MSG_TYPE(disconnect_request),
    MSG_TYPE(disconnect_code),
    MSG_TYPE(command_return_code)
>;

using config_group =
std::tuple
<
    MSG_TYPE(config_version_request),
    MSG_TYPE(config_version), 
    MSG_TYPE(function_list_request),
    MSG_TYPE(diff_function_list_request),
    MSG_TYPE(function_list),
    MSG_TYPE(function_config_request),
    MSG_TYPE(function_diff_config_request),
    MSG_TYPE(function_config)
>;

using data_access_group =
std::tuple
<
    MSG_TYPE(function_value_read_request),
    MSG_TYPE(function_value_read_on_update_1_time_request),
    MSG_TYPE(function_value_read_on_update_request),
    MSG_TYPE(function_value_read_periodical_request),
    MSG_TYPE(function_value_update_cancel),
    MSG_TYPE(function_value_periodical_update_cancel),
    MSG_TYPE(function_value_read),
    MSG_TYPE(function_value_read_denied),
    MSG_TYPE(function_value_write),
    MSG_TYPE(labels_format_request),
    MSG_TYPE(labels_format)
>;

#undef MSG_TYPE

using common_protocol_table =
std::tuple
<
    pair<service_group_key    , service_group>,
    pair<config_group_key     , config_group >,
    pair<data_access_group_key, data_access_group>
>;

template <typename Group, typename Type>
using message_body =
at_key
<
    Type,
    at_key
    <
        Group,
        common_protocol_table
    >
>;

template <typename Group, typename Type>
using message =
std::tuple
<
    pair<header_key, message_header>,
    pair<body_key  , message_body<Group, Type>>
>;

}

///////////////////////////////////////////////////////////
//
//              Parameter config data types
//
///////////////////////////////////////////////////////////

namespace details
{
struct type_key;
struct code_key;
struct read_acess_level;
struct write_acess_level;
}

constexpr uint8_t NA_PARAM   = 0;
constexpr uint8_t READ_FLAG  = 1;
constexpr uint8_t WRITE_FLAG = 1 << 1;

using parameter_access_config =
std::tuple
<
    pair<details::type_key, uint8_t>,
    pair<details::code_key, uint8_t>
>;

using parameter_access_level_config =
std::tuple
<
    pair<details::read_acess_level , uint8_t>,
    pair<details::write_acess_level, uint8_t>
>;

namespace details
{
struct field_count_key;
struct field_size_key;
struct field_format_key;
}

using parameter_value_type_config =
std::tuple
<
    pair<details::field_count_key , uint8_t>,
    pair<details::field_size_key  , uint8_t>,
    pair<details::field_format_key, uint8_t>
>;

namespace details
{
struct value_max_key;
struct value_min_key;
struct value_step_key;
}

template <typename T>
using parameter_value_limits_config =
std::tuple
<
    pair<details::value_max_key , T>,
    pair<details::value_min_key , T>,
    pair<details::value_step_key, T>
>;

namespace details
{
struct access_config_key;
struct access_level_config_key;
struct value_type_config_key;
struct value_limits_config_key;
struct dimension_key;
}

template <typename T>
using parameter_config =
std::tuple
<
    pair<details::access_config_key      , parameter_access_config>,
    pair<details::access_level_config_key, parameter_access_level_config>,
    pair<details::value_type_config_key  , parameter_value_type_config>,
    pair<details::value_limits_config_key, parameter_value_limits_config<T>>,
    pair<details::dimension_key          , repeat<uint8_t, uint8_t>>
>;

///////////////////////////////////////////////////////////
//
//                      Parameter
//
///////////////////////////////////////////////////////////

class parameter_base
{
    any invalid_ret() const { access_error(); return make_storage(0); }
protected:
    using f_t = std::function<void()>;

    void access_error() const { throw parameter_access_error(get_p_code()); }
public:
    virtual any get_config() const { return invalid_ret(); }
    virtual any get_value_writer() { return invalid_ret(); }

    virtual std::tuple<any, uint8_t> get_value_reader()
    {
        return std::tuple<any, uint8_t>(invalid_ret(), 0);
    }

    virtual void on_read()  { access_error(); }
    virtual void on_write() { access_error(); }

    virtual void add_read_action (const f_t&, size_t p = 0) { access_error(); }
    virtual void add_write_action(const f_t&, size_t p = 0) { access_error(); }
    
    virtual void set_read()  { access_error(); }    
    virtual void set_write() { access_error(); }

    virtual uint8_t get_p_code() const = 0; // HACK or not?
};

template <uint8_t PARAMETER_TYPE, typename V>
class parameter: public parameter_base
{
    parameter_config<V> config;
    std::vector<V> value;

    class rw_action
    {
        bool ready = false;
        boost::signals2::signal<void()> actions;
    public:
        void operator()()
        {
            if(ready) {
                actions();
                ready = false;
            }
        }

        void set_ready() { ready = true; }
        void add(const f_t& f) { actions.connect(f); }
    };

    // actions after read and write
    rw_action read_actions; // actions after value read
    rw_action write_actions; // actions after value write

    // check access for read/write
    template <uint8_t FLAG>
    void check_flag() const
    {
        if((PARAMETER_TYPE & FLAG) == 0) // no such flag in parameter type
            access_error();
    }

    void set_field_count()
    {
        using namespace details;

        size_t field_count =
        get<field_count_key>(get<value_type_config_key>(config));

        value.resize(field_count);
    }
public:
    parameter(const parameter_config<V>& conf) :
        config(conf)
    {
        set_field_count();
    }

    std::vector<V>& val_ref() { return value; } // TODO ugly hack
    uint8_t get_p_code() const
    {
        using namespace details;
        return get<code_key>(get<access_config_key>(config));
    } // HACK ?

    any get_config() const {  return make_storage(config); }

    // rw interface
    std::tuple<any, uint8_t> get_value_reader()
    {
        check_flag<READ_FLAG>();
        return std::tuple<any, uint8_t>(make_storage_ref(value), 0);
    }

    any get_value_writer()
    {
        check_flag<WRITE_FLAG>();
        return make_storage_ref(value);
    }

    // add actions
    void add_read_action(const parameter_base::f_t& f)
    {
        check_flag<READ_FLAG>();
        read_actions.add(f);
    }

    void add_write_action(const parameter_base::f_t& f)
    {
        check_flag<WRITE_FLAG>();
        write_actions.add(f);
    }

    void on_read()  { check_flag<READ_FLAG >(); read_actions();  }
    void on_write() { check_flag<WRITE_FLAG>(); write_actions(); }

    void set_read()  { check_flag<READ_FLAG >(); read_actions.set_ready();  }
    void set_write() { check_flag<WRITE_FLAG>(); write_actions.set_ready(); }
};

template <>
class parameter<NA_PARAM, std::tuple<>> : public parameter_base
{
    parameter_access_config config;

public:
    parameter(const uint8_t& pnum)
    {
        get<details::type_key>(config) = NA_PARAM;
        get<details::code_key>(config) = pnum;
    }

    any get_config() const { return make_storage(config); }
    uint8_t get_p_code() const { return get<details::code_key>(config); }
};

using na_parameter = parameter<NA_PARAM, std::tuple<>>;

///////////////////////////////////////////////////////////
//
//              Parameter Serialization
//
///////////////////////////////////////////////////////////

template <uint8_t P_TYPE, typename T, typename IStream>
inline std::shared_ptr<parameter_base>
make_parameter_from_config
(
    IStream& is,
    const parameter_access_config& p_access_conf,
    const parameter_access_level_config& p_access_level_conf,
    const parameter_value_type_config& p_value_conf
)
{
    using namespace details;

    parameter_value_limits_config<T> p_value_limits;
    is >> p_value_limits;

    repeat<uint8_t, uint8_t> p_dimension;
    is >> p_dimension;

    parameter_config<T> conf;
    
    get<access_config_key      >(conf) = p_access_conf;
    get<access_level_config_key>(conf) = p_access_level_conf;
    get<value_type_config_key  >(conf) = p_value_conf;
    get<value_limits_config_key>(conf) = p_value_limits;
    get<dimension_key          >(conf) = p_dimension;

    return std::make_shared<parameter<P_TYPE, T>>(conf);
}

template <uint8_t P_TYPE, typename IStream>
inline std::shared_ptr<parameter_base>
make_parameter_from_config(IStream& is, uint8_t p_code)
{
    parameter_access_config p_access_conf;

    get<details::type_key>(p_access_conf) = P_TYPE;
    get<details::code_key>(p_access_conf) = p_code;

    parameter_access_level_config p_access_level_conf;
    is >> p_access_level_conf;

    parameter_value_type_config p_value_conf;
    is >> p_value_conf;

    uint8_t p_size = get<details::field_size_key>(p_value_conf);
    uint8_t p_format_flags = get<details::field_format_key>(p_value_conf);

    bool is_floating_point = p_format_flags & 1;
    bool is_signed = p_format_flags & (1 << 1);

    #define __P_MAKE(T)\
    make_parameter_from_config<P_TYPE, T>\
    (\
        is,\
        p_access_conf,\
        p_access_level_conf,\
        p_value_conf\
    )

    if(is_floating_point) {
        switch(p_size)
        {
        case 2: return __P_MAKE(float);
        case 3: return __P_MAKE(double);
        case 4: return __P_MAKE(long double);
        default: throw parameter_access_error(p_code); // TODO other exc
        }
    }
    else {
        switch(p_size)
        {
        case 0: return is_signed ? __P_MAKE(int8_t ) : __P_MAKE(uint8_t );
        case 1: return is_signed ? __P_MAKE(int16_t) : __P_MAKE(uint16_t);
        case 2: return is_signed ? __P_MAKE(int32_t) : __P_MAKE(uint32_t);
        case 3: return is_signed ? __P_MAKE(int64_t) : __P_MAKE(uint64_t);
        default: throw parameter_access_error(p_code); // TODO other exc
        }
    }

    #undef __P_MAKE
}

template <typename IStream>
inline std::shared_ptr<parameter_base> make_parameter_from_config(IStream& is)
{
    uint8_t p_type, p_code;
    is >> p_type >> p_code;

    switch(p_type) {
    case READ_FLAG:
        return make_parameter_from_config<READ_FLAG>(is, p_code);
    case WRITE_FLAG:
        return make_parameter_from_config<WRITE_FLAG>(is, p_code);
    case READ_FLAG | WRITE_FLAG:
        return make_parameter_from_config<READ_FLAG | WRITE_FLAG>(is, p_code);
    case NA_PARAM:
        return std::make_shared<na_parameter>(na_parameter(p_code));
    default:
        throw parameter_access_error(p_code); // TODO other exc
    }

    return std::make_shared<na_parameter>(na_parameter(p_code));
}

///////////////////////////////////////////////////////////
//
//                 Common Protocol Function
//
///////////////////////////////////////////////////////////

enum class FunctionCodes : uint16_t
{
    UNSPECIFIED_DEVICE = 0,
    MOTION_CONTROL = 1,
    SENSOR_1D = 2
};

using function_base = std::map<uint8_t, std::shared_ptr<parameter_base>>;

template <FunctionCodes F_CODE, uint8_t P_COUNT>
class function : public function_base
{
public:
    function()
    {
        for(uint8_t i = 0; i < P_COUNT; i++) {
            auto empty_p = std::make_shared<na_parameter>(na_parameter(i));
            insert(std::make_pair(i, empty_p));
        }
    }
};

using move_control_function = function<FunctionCodes::MOTION_CONTROL, 0x1B>;
using sensor_1D_function    = function<FunctionCodes::SENSOR_1D     , 0x0B>;

///////////////////////////////////////////////////////////
//
//                 Robot Function Interface
//
///////////////////////////////////////////////////////////

class robot_state
{
protected:
    using same_type_function_group_t = std::map<uint16_t, function_base>;
    using function_map_t = std::map<uint16_t, same_type_function_group_t>;

    function_map_t function_map;
public:
    function_base& get_function_ref(uint16_t f_code, uint16_t f_number)
    {
        // TODO check index
        return function_map[f_code][f_number];
    }

    std::shared_ptr<parameter_base>& parameter_ref
    (
        uint16_t f_code,
        uint16_t f_number,
        uint8_t p_code
    )
    {
        // TODO check index
        return function_map[f_code][f_number][p_code];
    }
    // service information
    
    // function list
    common_protocol::function_list get_f_list() const
    {
        using namespace common_protocol;
        function_list res;

        for(auto& p : function_map)
            for(auto& f: p.second)
                res.push_back(function_id_t(p.first, f.first));

        return res;
    }

    template <typename IStream>
    void update_function_list(IStream& is)
    {
        uint32_t num_of_functions;
        is >> num_of_functions;

        for(size_t i = 0; i < num_of_functions; i++) {
            uint16_t f_code, f_num;
            is >> f_code >> f_num;
            function_map[f_code][f_num];
        }
    }

    // config
    template <typename IStream>
    common_protocol::function_config get_function_config(IStream& is)
    {
        using namespace common_protocol;

        uint16_t f_code, f_number;
        is >> f_code >> f_number;

        common_protocol::function_config res;

        get<0>(res) = function_id_t(f_code, f_number);

        for(auto& p : function_map[f_code][f_number])
            get<1>(res).push_back(p.second->get_config());

        return res;
    }

    template <typename IStream>
    void update_function_config(IStream& is)
    {
        uint16_t f_code, f_number;
        is >> f_code >> f_number;

        uint8_t num_of_params;
        is >> num_of_params;

        for(size_t i = 0; i < num_of_params; i++) {
            auto new_param = make_parameter_from_config(is);
            uint8_t p_code = new_param->get_p_code();
            function_map[f_code][f_number][p_code] = new_param;
        }
    }

    // value read
    template <typename IStream>
    common_protocol::function_value_read get_read_values(IStream& is)
    {
        using namespace common_protocol;

        uint16_t f_code, f_number;
        is >> f_code >> f_number;

        uint8_t num_of_params;
        is >> num_of_params;

        function_value_read res;

        get<0>(res) = function_id_t(f_code, f_number);

        for(size_t i = 0; i < num_of_params; i++) {
            // TODO check_index, exc
            uint8_t p_code, p_flags;
            is >> p_code >> p_flags; // TODO flags
            std::tuple<uint8_t, std::tuple<any, uint8_t>> v
            (
                p_code,
                function_map[f_code][f_number][p_code]->get_value_reader()
            );
            get<1>(res).push_back(v);
        }

        return res;
    }

    template <typename IStream>
    void update_function_read_values(IStream& is)
    {
        uint16_t f_code, f_number;
        is >> f_code >> f_number;

        uint8_t num_of_params;
        is >> num_of_params;

        for(size_t i = 0; i < num_of_params; i++) {
            uint8_t p_code;
            is >> p_code;

            auto reader =
            function_map[f_code][f_number][p_code]->get_value_reader();
            is >> reader;
        }
    }

    // value write
    template <typename IStream>
    common_protocol::function_value_write get_write_values(IStream& is)
    {
        using namespace common_protocol;

        uint16_t f_code, f_number;
        is >> f_code >> f_number;

        uint8_t num_of_params;
        is >> num_of_params;

        function_value_write res;
        using p_wr_t = std::tuple_element<1, function_value_write>::type::value_type;

        get<0>(res) = function_id_t(f_code, f_number);

        for(size_t i = 0; i < num_of_params; i++) {
            // TODO check_index, exc
            uint8_t p_code;
            is >> p_code;

            any v = function_map[f_code][f_number][p_code]->get_value_writer();
            get<1>(res).push_back(p_wr_t(p_code, v));
        }

        return res;
    }

    template <typename IStream>
    void write_function_values(IStream& is)
    {
        uint16_t f_code, f_number;
        is >> f_code >> f_number;

        uint8_t num_of_params;
        is >> num_of_params;

        for(size_t i = 0; i < num_of_params; i++) {
            uint8_t p_code;
            is >> p_code;

            auto& p = function_map[f_code][f_number][p_code];
            auto writer = p->get_value_writer();
            is >> writer;
            p->set_write();
            p->on_write();
        }

    }
    // labels
};

class client_server_base
{
protected:
    template <typename Group, typename Type>
    common_protocol::message<Group, Type>
    make_message
    (
        const common_protocol::message_body<Group, Type>& m,
        uint32_t msg_num = 0
    )
    {
        using namespace common_protocol;

        message<Group, Type> ret;

        auto& header = get<header_key>(ret);
        
        get<group_key      >(header) = Group::value;
        get<type_key       >(header) = Type::value;
        get<message_num_key>(header) = msg_num;
        get<data_size_key  >(header) = calc_size(m);

        auto& body = get<body_key>(ret);

        body = m;

        return ret;
    };

    connection io;
    robot_state r;

    template <typename Group, typename Type>
    void send_message
    (
        const common_protocol::message_body<Group, Type>& m,
        uint32_t msg_num = 0
    )
    {
        io.write(make_message<Group, Type>(m, msg_num));
    }

    template <typename T>
    client_server_base(const T& t): io(t) {}
public:

    std::shared_ptr<parameter_base>& parameter_ref
    (
        uint16_t f_code,
        uint16_t f_number,
        uint8_t p_code
    )
    {
        return r.parameter_ref(f_code, f_number, p_code);
    }

    function_base& get_function_ref(uint16_t f_code, uint16_t f_number)
    {
        return r.get_function_ref(f_code, f_number);
    }
};

class server : public client_server_base
{
public:
    template <typename T>
    server(const T& t): client_server_base(t) {}

    void server_package_parse()
    {
        using namespace common_protocol;

        message_header header;
        io.read(header);

        uint16_t msg_group, msg_type;
        uint32_t msg_size;

        msg_group = get<group_key>(header);
        msg_type = get<type_key>(header);
        msg_size = get<data_size_key>(header);

        binary_buffer tmp_buf = io.read_buffer(msg_size);
        binary_istream is(tmp_buf);

        switch(msg_group) {
            case service_group_key::value:
                switch(msg_type) {
                case protocol_version_key::value:
                    break;
                case active_connections_request_key::value:
                    break;
                case active_connections_info_key::value:
                    break;
                case control_level_up_request_key::value:
                    break;
                case control_level_activation_request_key::value:
                    break;
                case control_level_deactivation_request_key::value:
                    break;
                case disconnect_request_key::value:
                    break;
                case disconnect_code_key::value:
                    break;
                case command_return_code_key::value:
                    break;
                default: break;// TODO send error msg
                }
                break;
            case config_group_key::value:
                switch(msg_type) {
                case config_version_request_key::value:
                    break;
                case config_version_key::value:
                    break;
                case function_list_request_key::value:
                    send_message
                    <
                        config_group_key,
                        function_list_key
                    >(r.get_f_list());
                    break;
                case diff_function_list_request_key::value:
                    break;
                case function_list_key::value:
                    break;
                case function_config_request_key::value:
                    send_message
                    <
                        config_group_key,
                        function_config_key
                    >(r.get_function_config(is));
                    break;
                case function_diff_config_request_key::value:
                    break;
                case function_config_key::value:
                    break;
                default: break;// TODO send error msg
                }
                    break;
            case data_access_group_key::value:
                switch(msg_type) {
                case function_value_read_request_key::value:
                    send_message
                    <
                        data_access_group_key,
                        function_value_read_key
                    >(r.get_read_values(is));
                    break;
                case function_value_read_on_update_1_time_request_key::value:
                    break;
                case function_value_read_on_update_request_key::value:
                    break;
                case function_value_read_periodical_request_key::value:
                    break;
                case function_value_update_cancel_key::value:
                    break;
                case function_value_periodical_update_cancel_key::value:
                    break;
                case function_value_read_key::value:
                    break;
                case function_value_read_denied_key::value:
                    break;
                case function_value_write_key::value:
                    r.write_function_values(is);
                    break;
                case labels_format_request_key::value:
                    break;
                case labels_format_key::value:
                    break;
                default: break;// TODO send error msg
                }
                    break;
            default:; // TODO send error msg
                    break;
        }
    }
};

class client : public client_server_base
{
public:
    template <typename T>
    client(const T& t): client_server_base(t) {}

    void update_config()
    {
        using namespace common_protocol;

        // function list
        send_message<config_group_key, function_list_request_key>(std::tuple<>());
        client_package_parse();

        for(auto& f: r.get_f_list()) {
            send_message<config_group_key, function_config_request_key>(f);
            client_package_parse();
        }
    }

    template <typename IStream>
    void write_parameter_values(IStream& is)
    {
        using namespace common_protocol;
        send_message
        <
            data_access_group_key,
            function_value_write_key
        >(r.get_write_values(is));
    }

    template <typename IStream>
    void set_parameter_values(IStream& is)
    {
        r.write_function_values(is);
    }

    template <typename IStream>
    void read_parameter_values(IStream& is)
    {
        using namespace common_protocol;

        function_value_read_request req;
        is >> req;

        send_message
        <
            data_access_group_key,
            function_value_read_request_key
        >(req);
    }

    void client_package_parse()
    {
        using namespace common_protocol;

        message_header header;
        io.read(header);

        uint16_t msg_group, msg_type;
        uint32_t msg_size;

        msg_group = get<group_key>(header);
        msg_type = get<type_key>(header);
        msg_size = get<data_size_key>(header);

        binary_buffer tmp_buf = io.read_buffer(msg_size);
        binary_istream is(tmp_buf);

        switch(msg_group) {
            case service_group_key::value:
                switch(msg_type) {
                case protocol_version_key::value:
                    break;
                case active_connections_request_key::value:
                    break;
                case active_connections_info_key::value:
                    break;
                case control_level_up_request_key::value:
                    break;
                case control_level_activation_request_key::value:
                    break;
                case control_level_deactivation_request_key::value:
                    break;
                case disconnect_request_key::value:
                    break;
                case disconnect_code_key::value:
                    break;
                case command_return_code_key::value:
                    break;
                default: break;// TODO send error msg
                }
                break;
            case config_group_key::value:
                switch(msg_type) {
                case config_version_request_key::value:
                    break;
                case config_version_key::value:
                    break;
                case function_list_request_key::value:
                    break;
                case diff_function_list_request_key::value:
                    break;
                case function_list_key::value:
                    r.update_function_list(is);
                    break;
                case function_config_request_key::value:
                    break;
                case function_diff_config_request_key::value:
                    break;
                case function_config_key::value:
                    r.update_function_config(is);
                    break;
                default: break;// TODO send error msg
                }
                break;
            case data_access_group_key::value:
                switch(msg_type) {
                case function_value_read_request_key::value:
                    break;
                case function_value_read_on_update_1_time_request_key::value:
                    break;
                case function_value_read_on_update_request_key::value:
                    break;
                case function_value_read_periodical_request_key::value:
                    break;
                case function_value_update_cancel_key::value:
                    break;
                case function_value_periodical_update_cancel_key::value:
                    break;
                case function_value_read_key::value:
                    r.update_function_read_values(is);
                    break;
                case function_value_read_denied_key::value:
                    break;
                case function_value_write_key::value:
                    r.write_function_values(is);
                    // TODO make & send write package
                    break;
                case labels_format_request_key::value:
                    break;
                case labels_format_key::value:
                    break;
                default:
                    break;// TODO send error msg
                }
                break;
            default:; // TODO send error msg
                break;
        }
    }
};
}

#endif // __FUNCTION_H__
