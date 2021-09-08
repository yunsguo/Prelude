#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <variant>
#include <type_traits>
#include <tuple>
#include <memory>

#include "base.h"

namespace fcl
{
    using size_t = std::size_t;

    /**
        data FingerTree a = Empty
                            | Single a
                            | Deep (Digit a) (FingerTree (Node a)) (Digit a)
        data Digit a = One a | Two a a | Three a a a | Four a a a a
        data Node a = Node2 a a | Node3 a a a
     * **/

    //compile-time array
    template <typename A, size_t N>
    struct cta;
}

namespace detail
{

    template <size_t I>
    struct __cta_traits
    {
        template <typename A, size_t N>
        constexpr static A &get(fcl::cta<A, N> &c) { return __cta_traits<I - 1>::get(c.tail); }

        template <typename A, size_t N>
        constexpr static A &&get(fcl::cta<A, N> &&c) { return __cta_traits<I - 1>::get(c.tail); }

        template <typename A, size_t N>
        constexpr static const A &get(const fcl::cta<A, N> &c) { return __cta_traits<I - 1>::get(c.tail); }

        template <typename A, size_t N>
        constexpr static const A &&get(const fcl::cta<A, N> &&c) { return __cta_traits<I - 1>::get(c.tail); }

        template <typename A>
        cta<std::shared_ptr<node<A>>, I> __fmap(cta<node<A>, I> &&c)
        {
            return cta<std::shared_ptr<node<A>>, I>()
        }
    };

    template <>
    struct __cta_traits<1>
    {

        template <typename A, size_t N>
        constexpr static A &get(fcl::cta<A, N> &c) { return c.a; }

        template <typename A, size_t N>
        constexpr static A &&get(fcl::cta<A, N> &&c) { return c.a; }

        template <typename A, size_t N>
        constexpr static const A &get(const fcl::cta<A, N> &c) { return c.a; }

        template <typename A, size_t N>
        constexpr static const A &&get(const fcl::cta<A, N> &&c) { return c.a; }

        template <typename A>
        cta<std::shared_ptr<node<A>>, 1> __fmap(cta<node<A>, 1> &&c)
        {
        }
    };

} // namespace detail

namespace fcl
{
    template <typename A, size_t N>
    struct cta
    {
        template <typename T, typename... Ts>
        constexpr cta(T head, Ts ...tail) : a(head), tail(tail...)
        {
            static_assert(std::is_convertible<T, A>::value);
            static_assert(sizeof...(Ts) + 1 == N);
        }

        template <size_t I, size_t J>
        constexpr cta(cta<A, I> &&head, cta<A, J> &&tail) : a(std::move(head.a)), tail(std::move(head.tail), std::move(tail))
        {
            static_assert(I + J == N);
        }

        constexpr cta(cta<A, 1> &&head, cta<A, N - 1> &&tail) : a(std::move(head.a)), tail(std::move(tail)) {}

        template <size_t>
        friend struct detail::__cta_traits;

        template <typename, size_t>
        friend struct cta;

        friend struct Show<cta, void>;

        template <typename>
        friend struct sequence_traits;

    private:
        A a;
        cta<A, N - 1> tail;
    };

    template <typename A>
    struct cta<A, 1>
    {
        template <typename T>
        constexpr cta(T head) : a(head)
        {
            static_assert(std::is_convertible<T, A>::value);
        }

        template <size_t>
        friend struct detail::__cta_traits;

        template <typename, size_t>
        friend struct cta;

        friend struct Show<cta, void>;

        template <typename>
        friend struct sequence_traits;

    private:
        A a;
    };
}
namespace std
{

    template <typename A, size_t N>
    struct tuple_size<fcl::cta<A, N>>
        : std::integral_constant<std::size_t, N>
    {
    };

    template <std::size_t I, typename A, size_t N>
    constexpr A &get(fcl::cta<A, N> &a) noexcept
    {
        static_assert(I < N);
        return detail::__cta_traits<I>::get(a);
    }

    template <std::size_t I, typename A, size_t N>
    constexpr A const &get(const fcl::cta<A, N> &a) noexcept
    {
        static_assert(I < N);
        return detail::__cta_traits<I>::get(a);
    }

    template <std::size_t I, typename A, size_t N>
    constexpr A &&get(fcl::cta<A, N> &&a) noexcept
    {
        static_assert(I < N);
        return detail::__cta_traits<I>::get(a);
    }

    template <std::size_t I, typename A, size_t N>
    constexpr A const &&get(const fcl::cta<A, N> &&a) noexcept
    {
        static_assert(I < N);
        return detail::__cta_traits<I>::get(a);
    }

    // template <typename... Ts>
    // constexpr std::enable_if_t<all<detail::is_shared_tuple, type_list<Ts...>>::value, detail::inferred_t<concat_t<Ts...>>> tuple_cat(Ts &&...ts) noexcept
    // {
    //     return detail::tuple_cat(std::forward<Ts>(ts)...);
    // }

}

namespace fcl
{

    template <typename A>
    struct node;

    template <typename A, size_t N>
    using numbered_node = std::variant<cta<std::shared_ptr<node<A>>, N>, std::shared_ptr<cta<A, N>>>;

    template <typename A, size_t N>
    cta<std::shared_ptr<node<A>>, N> __fmap(cta<node<A>, N> &&c)
    {
    }

    template <typename A>
    struct node
    {
        template <size_t N>
        node(cta<A, N> &&a) : _depth(0), a(std::make_shared<cta<A, N>>(std::move(a))) { static_assert(N == 2 || N == 3); }

        template <size_t N>
        node(const cta<A, N> &a) : _depth(0), a(numbered_node<A, N>(std::make_shared<cta<A, N>>(a))) { static_assert(N == 2 || N == 3); }

        template <size_t N>
        node(cta<node<A>, N> a) : _depth(std::get<0>(a).depth() + 1), a(detail::__cta_traits<N>::fmap(std::make_shared<node<A>>, std::move(a))) { static_assert(N == 2 || N == 3); }

        unsigned char depth() const { return _depth; }

        const unsigned char _depth;
        std::variant<numbered_node<A, 2>, numbered_node<A, 3>> a;
    };

    struct empty
    {
    };

    constexpr const empty empty_v = empty();

    template <typename A>
    struct single
    {
        template <typename T>
        single(T a) : a(a) { static_assert(std::is_convertible<T, A>::value); }
        A a;
    };

    template <typename A>
    using digit = std::variant<cta<A, 1>, cta<A, 2>, cta<A, 3>, cta<A, 4>>;

    template <typename A>
    constexpr digit<A> __push_front_digit(cta<A, 1> a, digit<A> b)
    {
        return b.index() == 0 ? digit<A>(cta<A, 2>(a, std::get<0>(b)))
               : b.index() == 1
                   ? digit<A>(cta<A, 3>(a, std::get<1>(b)))
                   : digit<A>(cta<A, 4>(a, std::get<2>(b)));
    }

    template <typename A>
    struct deep;

    template <typename A>
    using finger_tree = std::variant<empty, single<A>, deep<A>>;

    template <typename A>
    struct deep
    {
        using F = finger_tree<node<A>>;

        template <typename X, typename Y>
        deep(X a, F &&b, Y c) : a(a), b(std::make_shared<F>(std::move(b))), c(c)
        {
            static_assert(std::is_convertible<X, digit<A>>::value);
            static_assert(std::is_convertible<Y, digit<A>>::value);
        }
        unsigned char depth() const { return 0; }

        digit<A> a;
        std::shared_ptr<F> b;
        digit<A> c;
    };

    template <typename A>
    struct deep<node<A>>
    {
        using F = finger_tree<node<A>>;

        template <typename X, typename Y>
        deep(X a, F &&b, Y c) : a(a), b(std::make_shared<F>(std::move(b))), c(c)
        {
            static_assert(std::is_convertible<X, digit<node<A>>>::value);
            static_assert(std::is_convertible<Y, digit<node<A>>>::value);
        }
        unsigned char depth() const { return a.index() == 0 ? std::get<0>(std::get<0>(a)).depth() : a.index() == 1 ? std::get<0>(std::get<1>(a)).depth()
                                                                                                : a.index() == 2   ? std::get<0>(std::get<2>(a)).depth()
                                                                                                                   : std::get<0>(std::get<3>(a)).depth(); }
        digit<node<A>> a;
        std::shared_ptr<F> b;
        digit<node<A>> c;
    };

    template <typename A>
    struct sequence_traits;

    template <>
    struct sequence_traits<empty>
    {
        template <typename A>
        constexpr static finger_tree<A> push_front(A a, empty) { return single<A>(a); }
    };

    template <typename A>
    struct sequence_traits<single<A>>
    {
        constexpr static finger_tree<A> push_front(A a, const single<A> &b) { return deep<A>(cta<A, 1>(a), empty_v, cta<A, 1>(b.a)); }
        constexpr static finger_tree<A> push_front(A a, single<A> &&b) { return deep<A>(cta<A, 1>(a), empty_v, cta<A, 1>(std::move(b.a))); }
    };

    template <typename A>
    struct sequence_traits<deep<A>>
    {
        constexpr static finger_tree<A> push_front(A a, const deep<A> &d) { return d.a.index() == 3
                                                                                       ? deep<A>(cta<A, 2>(a, std::get<3>(d.a).a), sequence_traits<finger_tree<node<A>>>::push_front(node<A>(std::get<3>(d.a).tail), *d.b), d.c)
                                                                                       : deep<A>(__push_front_digit(cta<A, 1>(a), d.a), d.b, d.c); }
    };

    template <typename A>
    struct sequence_traits<deep<node<A>>>
    {
        constexpr static finger_tree<node<A>> push_front(node<A> a, const deep<node<A>> &d)
        {
            return a.depth() == d.depth()
                       ? d.a.index() == 3
                             ? deep<node<A>>(cta<node<A>, 2>(a, std::get<3>(d.a).a), sequence_traits<finger_tree<node<A>>>::push_front(node<A>(std::get<3>(d.a).tail), *d.b), d.c)
                             : deep<node<A>>(__push_front_digit(cta<node<A>, 1>(a), d.a), d.b, d.c)
                       : deep<node<A>>(d.a, sequence_traits<finger_tree<node<A>>>::push_front(a, *d.b), d.c);
        }
    };

    template <typename A>
    struct sequence_traits<finger_tree<A>>
    {
        constexpr static finger_tree<A> push_front(A a, const finger_tree<A> &b) { return b.index() == 0
                                                                                              ? sequence_traits<empty>::push_front(a, std::get<0>(b))
                                                                                          : b.index() == 1 ? sequence_traits<single<A>>::push_front(a, std::get<1>(b))
                                                                                                           : sequence_traits<deep<A>>::push_front(a, std::get<2>(b)); }
    };

    template <typename A, typename B, typename = decltype(sequence_traits<B>::push_front(std::declval<A>(), std::declval<B>()))>
    constexpr auto operator+=(A a, B b) -> decltype(sequence_traits<B>::push_front(std::declval<A>(), std::declval<B>()))
    {
        return sequence_traits<B>::push_front(std::forward<A>(a), std::forward<B>(b));
    }

    // template <typename A>
    // using node = std::variant<std::pair<A, A>, std::tuple<A, A, A>>;

    // template <typename A>
    // using digit = std::variant<A, std::pair<A, A>, std::tuple<A, A, A>, std::tuple<A, A, A, A>>;

    // struct empty
    // {
    // };

    // constexpr const empty empty_v = empty();

    // template <typename, typename>
    // struct finger_tree;

    // template <typename A>
    // void ____eval(A);

    // template <typename A>
    // struct finger_tree
    // {
    //     using V = std::variant<empty, A, std::tuple<digit<A>, finger_tree<node<A>>, digit<A>>>;

    //     template <typename... As, typename = decltype(____eval<V>(V(std::declval<As>()...)))>
    //     finger_tree(As... args) : val(std::forward<As>(args)...) {}

    //     V val;
    // };

    // template <typename A>
    // finger_tree<A> add(A a, finger_tree<A> ft)
    // {
    //     switch (ft.val.index())
    //     {
    //     case 0:
    //         return finger_tree<A>(std::in_place_index<1>, a);
    //     case 1:
    //         return finger_tree<A>(std::forward_as_tuple<digit<A>(a), finger_tree<A>(empty_v)>(), digit<A>(std::get<1>(ft.val)));
    //     default:
    //         return empty_v;
    //     }
    // }

}
#endif