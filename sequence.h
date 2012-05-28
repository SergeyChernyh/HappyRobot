#ifndef __CONTAINER__
#define __CONTAINER__

#include "metaprogramming/element_access.h"
#include "metaprogramming/type_traits.h"

namespace robot { namespace metaprogramming
{
    namespace details
    {
        template <typename T>
        class element_value
        {
            T value;

        public:
            const T& get() const { return value; }
                  T& get()       { return value; }
        };

        template <typename T>
        struct nothing
        {
            T get() const { return T(); };
        };

        template <typename T>
        using element_storage =
        at_key
        <
            is_const<T>,
            pair<std::true_type , nothing<T>>,
            pair<std::false_type, element_value<T>>
        >;

        template <typename ...>
        struct element;

        template <typename T, typename ...Signature>
        struct element<T, Signature...>: public element_storage<T>
        {};

        template <typename Key, typename T, typename ...Signature>
        struct element<pair<Key, T>, Signature...>:
            public element<T, Signature...>
        {};

        template <>
        struct element<>
        {};
    }

    template <>
    struct sequence<> {};

    template <typename Head, typename ...Tail>
    struct sequence<Head, Tail...>:
        public sequence<Tail...>,
        public details::element<Head, Tail...>
    {};
}

namespace sequence_access
{
    namespace element_access
    {
        namespace m = metaprogramming;

        template <typename, typename>
        struct access;

        template <typename ...ElementSignature, typename Head, typename ...Tail>
        struct access<m::sequence<ElementSignature...>, m::sequence<Head, Tail...>>:
            public access<m::sequence<ElementSignature...>, m::sequence<Tail...>>
        {};

        template <typename Head, typename ...Tail>
        struct access<m::sequence<Tail...>, m::sequence<Head, Tail...>>
        {
            using type = m::details::element<Head, Tail...>;
        };

        template <typename ...ElementSignature>
        struct access<m::sequence<ElementSignature...>, m::sequence<>>
        {
            static_assert
            (
                true,
                "robot::container::element_access::access: element not found"
            );
        };

        template <typename, typename>
        struct accessor;

        template <typename ...ElementSignature, typename ...Args>
        struct accessor<m::sequence<ElementSignature...>, m::sequence<Args...>>
        {
            using type =
            typename
            access
            <
                m::sequence<ElementSignature...>,
                m::sequence<Args...>
            >::type;
        };

        template <size_t n, typename ...Args> struct signature_at_c;
     
        template <typename Head, typename ...Tail>
        struct signature_at_c<0, Head, Tail...>
        {
            using type = m::sequence<Head, Tail...>;
        };
         
        template <size_t n, typename Head, typename ...Tail>
        struct signature_at_c<n, Head, Tail...>: public signature_at_c<n - 1, Tail...>
        {
            static_assert
            (
                n <= sizeof...(Tail),
                "robot::container::element_access::signature_at_c: out of range"
            );
        };

        template <size_t n, typename ...Args>
        struct signature_at_c<n, m::sequence<Args...>>: public signature_at_c<n, Args...> {};

        template <typename ...Args>
        struct signature_at_c<0, m::sequence<Args...>>: public signature_at_c<0, Args...> {};

        template <typename Key, typename ...Args> struct signature_at_key;

        template <typename Head, typename ...Tail>
        struct signature_at_key<typename m::convert_to_pair<Head>::first, Head, Tail...>
        {
            using type = m::sequence<Head, Tail...>;
        };

        template <typename Key, typename Head, typename ...Tail>
        struct signature_at_key<Key, Head, Tail...>: public signature_at_key<Key, Tail...> {};

        template <typename Key, typename... Args>
        struct signature_at_key<Key, m::sequence<Args...>>: public signature_at_key<Key, Args...> {};

        template <typename>
        struct element_;

        template <typename ...Signature>
        struct element_<m::sequence<Signature...>>
        {
            using type = m::details::element<Signature...>;
        };

        template <typename T>
        using element = typename element_<T>::type;

        template <size_t C, typename ...Args>
        using at_c = m::value_type<m::at_c<C, Args...>>;

        template <typename Key, typename ...Args>
        using at_key = m::value_type<m::at_key<Key, Args...>>;
    }
    
    template <size_t C, typename ...Args>
    element_access::at_c<C, Args...>& at_c(metaprogramming::sequence<Args...>& p)
    {
        using namespace element_access;
        element<typename signature_at_c<C, Args...>::type> *r = &p;
        return r->get();
    }

    template <size_t C, typename ...Args>
    const element_access::at_c<C, Args...>& at_c(const metaprogramming::sequence<Args...>& p)
    {
        using namespace element_access;
        const element<typename signature_at_c<C, Args...>::type> *r = &p;
        return r->get();
    }

    template <typename Key, typename ...Args>
    element_access::at_key<Key, Args...>& at_key(metaprogramming::sequence<Args...>& p)
    {
        using namespace element_access;
        element<typename signature_at_key<Key, Args...>::type> *r = &p;
        return r->get();
    }
    
    template <typename Key, typename ...Args>
    const element_access::at_key<Key, Args...>& at_key(const metaprogramming::sequence<Args...>& p)
    {
        using namespace element_access;
        const element<typename signature_at_key<Key, Args...>::type> *r = &p;
        return r->get();
    }

    template <size_t C, typename ...Args>
    element_access::at_c<C, Args...> value_at_c(const metaprogramming::sequence<Args...>& p)
    {
        using namespace element_access;
        const element<typename signature_at_c<C, Args...>::type> *r = &p;
        return r->get();
    }
    
    template <typename Key, typename ...Args>
    element_access::at_key<Key, Args...> value_at_key(const metaprogramming::sequence<Args...>& p)
    {
        using namespace element_access;
        const element<typename signature_at_key<Key, Args...>::type> *r = &p;
        return r->get();
    }
}

namespace algorithm
{
    namespace m = metaprogramming;

    template <bool, typename>
    struct for_each_argument;

    template <typename T>
    struct for_each_argument<true, T>
    {
        template <typename S>
        static m::value_type<T> get(const S& s) { return m::value_type<T>(); }
    };

    template <typename T>
    struct for_each_argument<false, T>
    {
        template <typename S>
        static m::value_type<T>& get(S& s) { return sequence_access::at_c<0>(s); }

        template <typename S>
        static const m::value_type<T>& get(const S& s) { return sequence_access::at_c<0>(s); }
    };

    template <typename F, typename T>
    struct for_each_struct;

    template <typename F>
    struct for_each_struct<F, m::sequence<>>
    {
        static void do_(F& f, m::sequence<>& s) {}
        static void do_(F& f, const m::sequence<>& s) {}
    };

    template <typename F, typename Head, typename ...Args>
    struct for_each_struct<F, m::sequence<Head, Args...>>
    {
        static void do_(F& f, m::sequence<Head, Args...>& s)
        {
            f(for_each_argument<m::is_const<Head>::value, Head>::get(s));
            for_each_struct<F, m::sequence<Args...>>::do_(f, s);
        }

        static void do_(F& f, const m::sequence<Head, Args...>& s)
        {
            f(for_each_argument<m::is_const<Head>::value, Head>::get(s));
            for_each_struct<F, m::sequence<Args...>>::do_(f, s);
        }
    };

    template <typename F, typename T>
    void for_each(F& f, T& t) { for_each_struct<F, T>::do_(f, t); }

    template <typename F, typename T>
    void for_each(F& f, const T& t) { for_each_struct<F, T>::do_(f, t); }
}

}

#endif //__CONTAINER__
