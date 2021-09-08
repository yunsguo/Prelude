#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <variant>
#include <type_traits>
#include <memory>

namespace fcl
{

    template <size_t>
    struct __cta__impl_traits;

    template <typename T>
    using __remove_cvref_t = std::remove_reference_t<std::remove_cv_t<T>>;

    template <typename From, typename To>
    using __is_forwardable = std::bool_constant<std::is_convertible<__remove_cvref_t<From>, To>::value>;

    template <typename From, typename To>
    using __is_perfectly_forwardable = std::bool_constant<std::is_convertible<__remove_cvref_t<From>, To>::value && std::is_same<__remove_cvref_t<From>, __remove_cvref_t<To>>::value>;

    //compile-time array
    template <typename A, size_t N>
    struct CTA
    {
        template <typename Head, typename Tail, typename = std::enable_if_t<__is_forwardable<Head, A>::value && __is_forwardable<Tail, CTA<A, N - 1>>::value>>
        constexpr CTA(Head &&head, Tail &&tail) : head{std::forward<Head>(head)}, tail{std::forward<Tail>(tail)} {}

        template <size_t>
        friend struct __cta__impl_traits;

        template <typename, size_t>
        friend struct CTA;

        template <typename, typename>
        friend struct Show;

        template <typename>
        friend struct sequence_traits;

    private:
        A head;
        CTA<A, N - 1> tail;
    };

    template <typename A>
    struct CTA<A, 1>
    {
        template <typename Head, typename = std::enable_if_t<__is_forwardable<Head, A>::value>>
        constexpr CTA(Head &&head) : head{std::forward<Head>(head)} {}

        template <size_t>
        friend struct __cta__impl_traits;

        template <typename, size_t>
        friend struct CTA;

        template <typename, typename>
        friend struct Show;

        template <typename>
        friend struct sequence_traits;

    private:
        A head;
    };

    template <typename A>
    constexpr CTA<A, 1> make_cta(A &&arg)
    {
        return CTA<A, 1>(std::forward<A>(arg));
    }

    template <typename A, typename... As>
    constexpr CTA<A, sizeof...(As) + 1> make_cta(A &&arg, As &&...args)
    {
        return CTA<A, sizeof...(As) + 1>(std::forward<A>(arg), make_cta<As...>(std::forward<As>(args)...));
    }

    template <>
    struct __cta__impl_traits<1>
    {

        template <typename A, size_t N>
        constexpr static A &get(fcl::CTA<A, N> &c) { return c.head; }

        template <typename A, size_t N>
        constexpr static A &&get(fcl::CTA<A, N> &&c) { return std::move(c.head); }

        template <typename A, size_t N>
        constexpr static const A &get(const fcl::CTA<A, N> &c) { return c.head; }

        template <typename A, size_t N>
        constexpr static const A &&get(const fcl::CTA<A, N> &&c) { return c.head; }

        template <typename A>
        constexpr static CTA<std::shared_ptr<A>, 1> fmap(const CTA<A, 1> &c)
        {
            return CTA<std::shared_ptr<A>, 1>(std::make_shared<A>(c.head));
        }

        template <typename A>
        constexpr static CTA<std::shared_ptr<A>, 1> &&fmap(CTA<A, 1> &&c)
        {
            return CTA<std::shared_ptr<A>, 1>(std::make_shared<A>(std::move(c.head)));
        }
    };

    template <size_t I>
    struct __cta__impl_traits
    {
        template <typename A, size_t N>
        constexpr static A &get(fcl::CTA<A, N> &c) { return __cta__impl_traits<I - 1>::get(c.tail); }

        template <typename A, size_t N>
        constexpr static A &&get(fcl::CTA<A, N> &&c) { return __cta__impl_traits<I - 1>::get(std::move(c.tail)); }

        template <typename A, size_t N>
        constexpr static const A &get(const fcl::CTA<A, N> &c) { return __cta__impl_traits<I - 1>::get(c.tail); }

        template <typename A, size_t N>
        constexpr static const A &&get(const fcl::CTA<A, N> &&c) { return __cta__impl_traits<I - 1>::get(std::move(c.tail)); }

        template <typename A>
        constexpr static CTA<std::shared_ptr<A>, I> fmap(const CTA<A, I> &c)
        {
            return CTA<std::shared_ptr<A>, I>(std::make_shared<A>(c.head), __cta__impl_traits<I - 1>::fmap(c.tail));
        }

        template <typename A>
        constexpr static CTA<std::shared_ptr<A>, I> &&fmap(CTA<A, I> &&c)
        {
            return CTA<std::shared_ptr<A>, I>(std::make_shared<A>(std::move(c.head)), __cta__impl_traits<I - 1>::fmap(std::move(c.tail)));
        }
    };
}
namespace std
{

    template <std::size_t I, typename A, size_t N>
    constexpr A &get(fcl::CTA<A, N> &a) noexcept
    {
        static_assert(I < N);
        return fcl::__cta__impl_traits<I + 1>::get(a);
    }

    template <std::size_t I, typename A, size_t N>
    constexpr A const &get(const fcl::CTA<A, N> &a) noexcept
    {
        static_assert(I < N);
        return fcl::__cta__impl_traits<I + 1>::get(a);
    }

    template <std::size_t I, typename A, size_t N>
    constexpr A &&get(fcl::CTA<A, N> &&a) noexcept
    {
        static_assert(I < N);
        return fcl::__cta__impl_traits<I + 1>::get(std::move(a));
    }

    template <std::size_t I, typename A, size_t N>
    constexpr A const &&get(const fcl::CTA<A, N> &&a) noexcept
    {
        static_assert(I < N);
        return fcl::__cta__impl_traits<I + 1>::get(std::move(a));
    }

}

namespace fcl
{

    // data Node a = Node2 a a | Node3 a a a
    template <typename A>
    struct node;

    template <typename A, size_t N>
    using numbered_node = std::variant<CTA<std::shared_ptr<node<A>>, N>, std::shared_ptr<CTA<A, N>>>;

    template <typename A>
    struct node
    {
        template <size_t N>
        constexpr node(CTA<A, N> &&a) : a(std::make_shared<CTA<A, N>>(std::move(a))) { static_assert(N == 2 || N == 3); }

        template <size_t N>
        constexpr node(const CTA<A, N> &a) : a(numbered_node<A, N>(std::make_shared<CTA<A, N>>(a))) { static_assert(N == 2 || N == 3); }

        template <size_t N>
        constexpr node(const CTA<node<A>, N> &a) : a(fcl::__cta__impl_traits<N>::fmap(a)) { static_assert(N == 2 || N == 3); }

        template <size_t N>
        constexpr node(CTA<node<A>, N> &&a) : a(fcl::__cta__impl_traits<N>::fmap(std::move(a))) { static_assert(N == 2 || N == 3); }

        std::variant<numbered_node<A, 2>, numbered_node<A, 3>> a;
    };

    struct empty
    {
    };

    constexpr const empty empty_v = empty();

    template <typename A>
    struct single
    {
        template <typename T, typename = std::enable_if_t<__is_forwardable<T, A>::value>>
        constexpr single(T &&a) : a{std::forward<T>(a)} {}

        A a;
    };

    // data Digit a = One a | Two a a | Three a a a | Four a a a a
    template <typename A>
    using digit = std::variant<CTA<A, 1>, CTA<A, 2>, CTA<A, 3>, CTA<A, 4>>;

    template <typename A>
    constexpr digit<A> __push_front_digit(CTA<A, 1> &&a, digit<A> &&b)
    {
        return b.index() == 0 ? digit<A>(CTA<A, 2>(std::get<0>(std::move(a)), std::get<0>(std::move(b))))
               : b.index() == 1
                   ? digit<A>(CTA<A, 3>(std::get<0>(std::move(a)), std::get<1>(std::move(b))))
                   : digit<A>(CTA<A, 4>(std::get<0>(std::move(a)), std::get<2>(std::move(b))));
    }

    template <typename A>
    constexpr digit<A> __push_front_digit(CTA<A, 1> &&a, const digit<A> &b)
    {
        return b.index() == 0 ? digit<A>(CTA<A, 2>(std::get<0>(std::move(a)), std::get<0>(b)))
               : b.index() == 1
                   ? digit<A>(CTA<A, 3>(std::get<0>(std::move(a)), std::get<1>(b)))
                   : digit<A>(CTA<A, 4>(std::get<0>(std::move(a)), std::get<2>(b)));
    }

    template <typename A>
    struct deep;

    // data FingerTree a = Empty | Single a | Deep (Digit a) (FingerTree (Node a)) (Digit a)
    template <typename A>
    struct finger_tree
    {
        using V = std::variant<empty, single<A>, deep<A>>;

        template <typename T, typename = std::enable_if_t<__is_forwardable<T, V>::value>>
        constexpr finger_tree(T &&a) : a{std::forward<T>(a)} {}

        V a;
    };

    template <typename A>
    struct deep
    {
        using F = finger_tree<node<A>>;

        template <typename X, typename Y, typename Z,
                  std::enable_if_t<__is_forwardable<X, digit<A>>::value && __is_forwardable<Y, std::shared_ptr<F>>::value && __is_forwardable<Z, digit<A>>::value, int> = 0>
        constexpr deep(X &&a, Y &&b, Z &&c) : a(std::forward<X>(a)), b(std::forward<Y>(b)), c(std::forward<Z>(c)) {}

        template <typename X, typename Y, typename Z,
                  std::enable_if_t<__is_forwardable<X, digit<A>>::value && __is_forwardable<Y, empty>::value && __is_forwardable<Z, digit<A>>::value, int *> = nullptr>
        constexpr deep(X &&a, Y &&, Z &&c) : a(std::forward<X>(a)),
                                             b(nullptr), c(std::forward<Z>(c)) {}

        template <typename X, typename Y, typename Z,
                  std::enable_if_t<__is_forwardable<X, digit<A>>::value && __is_forwardable<Y, F>::value && !__is_forwardable<Y, empty>::value && __is_forwardable<Z, digit<A>>::value, int **> = nullptr>
        constexpr deep(X &&a, Y &&b, Z &&c) : deep(std::forward<X>(a), std::make_shared<F>(std::forward<Y>(b)), std::forward<Z>(c)) {}

        digit<A> a;
        std::shared_ptr<F> b;
        digit<A> c;
    };

    template <typename A>
    struct deep<node<A>>
    {
        using F = finger_tree<node<A>>;

        template <typename X, typename Y, typename Z,
                  std::enable_if_t<__is_forwardable<X, digit<node<A>>>::value && __is_forwardable<Y, std::shared_ptr<F>>::value && __is_forwardable<Z, digit<node<A>>>::value, int> = 0>
        constexpr deep(X &&a, Y &&b, Z &&c) : a(std::forward<X>(a)), b(std::forward<Y>(b)), c(std::forward<Z>(c)) {}

        template <typename X, typename Y, typename Z,
                  std::enable_if_t<__is_forwardable<X, digit<node<A>>>::value && __is_forwardable<Y, empty>::value && __is_forwardable<Z, digit<node<A>>>::value, int *> = nullptr>
        constexpr deep(X &&a, Y &&, Z &&c) : a(std::forward<X>(a)),
                                             b(nullptr), c(std::forward<Z>(c)) {}

        template <typename X, typename Y, typename Z,
                  std::enable_if_t<__is_forwardable<X, digit<node<A>>>::value && __is_forwardable<Y, F>::value && !__is_forwardable<Y, empty>::value && __is_forwardable<Z, digit<node<A>>>::value, int **> = nullptr>
        constexpr deep(X &&a, Y &&b, Z &&c) : deep(std::forward<X>(a), std::make_shared<F>(std::forward<Y>(b)), std::forward<Z>(c)) {}

        digit<node<A>> a;
        std::shared_ptr<F> b;
        digit<node<A>> c;
    };

    template <typename A>
    struct sequence_traits;

    template <>
    struct sequence_traits<empty>
    {
        // a <| Empty = Single a
        template <typename A, typename X, typename = std::enable_if_t<__is_forwardable<X, empty>::value>>
        constexpr static finger_tree<__remove_cvref_t<A>> push_front(A &&a, X &&)
        {
            return single<__remove_cvref_t<A>>(std::forward<A>(a));
        }
    };

    template <typename A>
    struct sequence_traits<single<A>>
    {

        // a <| Single b = Deep [a ] Empty [b ]
        template <typename X>
        constexpr static finger_tree<A> push_front(X &&a, single<A> &&b)
        {
            return deep<A>(digit<A>(CTA<A, 1>(std::forward<X>(a))), empty_v, digit<A>(CTA<A, 1>(std::move(b.a))));
        }
        template <typename X>
        constexpr static finger_tree<A> push_front(X &&a, const single<A> &b)
        {
            return deep<A>(digit<A>(CTA<A, 1>(std::forward<X>(a))), empty_v, digit<A>(CTA<A, 1>(b.a)));
        }
    };

    template <typename A>
    struct sequence_traits<deep<A>>
    {
        // a <| Deep [b, c, d, e ] m sf = Deep [a, b ] (Node3 c d e <| m) sf
        // a <| Deep pr m sf = Deep ([a ] + pr ) m sf
        template <typename X, typename = std::enable_if_t<__is_perfectly_forwardable<X, A>::value>>
        constexpr static finger_tree<A> push_front(X &&a, deep<A> &&d)
        {
            return d.a.index() == 3
                       ? deep<A>(CTA<A, 2>(std::forward<X>(a), std::move(std::get<0>(std::get<3>(d.a)))), sequence_traits<std::shared_ptr<finger_tree<node<A>>>>::push_front(node<A>(std::move(std::get<3>(d.a).tail)), std::move(d.b)), std::move(d.c))
                       : deep<A>(__push_front_digit(CTA<A, 1>(a), std::move(d.a)), std::move(d.b), std::move(d.c));
        }
        template <typename X, typename = std::enable_if_t<__is_perfectly_forwardable<X, A>::value>>
        constexpr static finger_tree<A> push_front(X &&a, const deep<A> &d)
        {
            return d.a.index() == 3
                       ? deep<A>(CTA<A, 2>(std::forward<X>(a), std::get<0>(std::get<3>(d.a))), sequence_traits<std::shared_ptr<finger_tree<node<A>>>>::push_front(node<A>(std::get<3>(d.a).tail), d.b), d.c)
                       : deep<A>(__push_front_digit(CTA<A, 1>(a), d.a), d.b, d.c);
        }
    };

    template <typename A>
    struct sequence_traits<deep<node<A>>>
    {
        // a <| Deep [b, c, d, e ] m sf = Deep [a, b ] (Node3 c d e <| m) sf
        // a <| Deep pr m sf = Deep ([a ] + pr ) m sf
        template <typename X, typename = std::enable_if_t<__is_perfectly_forwardable<X, node<A>>::value>>
        constexpr static finger_tree<node<A>> push_front(X &&a, deep<node<A>> &&d)
        {
            return d.a.index() == 3
                       ? deep<node<A>>(CTA<node<A>, 2>(std::forward<X>(a), std::move(std::get<0>(std::get<3>(d.a)))), sequence_traits<std::shared_ptr<finger_tree<node<A>>>>::push_front(node<A>(std::move(std::get<3>(d.a).tail)), std::move(d.b)), std::move(d.c))
                       : deep<node<A>>(__push_front_digit(CTA<node<A>, 1>(a), std::move(d.a)), std::move(d.b), std::move(d.c));
        }
        template <typename X, typename = std::enable_if_t<__is_perfectly_forwardable<X, node<A>>::value>>
        constexpr static finger_tree<node<A>> push_front(X &&a, const deep<node<A>> &d)
        {
            return d.a.index() == 3
                       ? deep<node<A>>(CTA<node<A>, 2>(std::forward<X>(a), std::get<0>(std::get<3>(d.a))), sequence_traits<std::shared_ptr<finger_tree<node<A>>>>::push_front(node<A>(std::get<3>(d.a).tail), d.b), d.c)
                       : deep<node<A>>(__push_front_digit(CTA<node<A>, 1>(a), d.a), d.b, d.c);
        }
    };

    template <typename A>
    struct sequence_traits<finger_tree<A>>
    {
        // (<|) :: a → FingerTree a → FingerTree a
        template <typename X, typename = std::enable_if_t<__is_perfectly_forwardable<X, A>::value>>
        constexpr static finger_tree<A> push_front(X &&a, finger_tree<A> &&b)
        {
            return b.a.index() == 0
                       ? sequence_traits<empty>::push_front(std::forward<X>(a), empty_v)
                   : b.a.index() == 1 ? sequence_traits<single<A>>::push_front(std::forward<X>(a), std::move(std::get<1>(b.a)))
                                      : sequence_traits<deep<A>>::push_front(std::forward<X>(a), std::move(std::get<2>(b.a)));
        }
        template <typename X, typename = std::enable_if_t<__is_perfectly_forwardable<X, A>::value>>
        constexpr static finger_tree<A> push_front(X &&a, const finger_tree<A> &b)
        {
            return b.a.index() == 0
                       ? sequence_traits<empty>::push_front(std::forward<X>(a), empty_v)
                   : b.a.index() == 1 ? sequence_traits<single<A>>::push_front(std::forward<X>(a), std::get<1>(b.a))
                                      : sequence_traits<deep<A>>::push_front(std::forward<X>(a), std::get<2>(b.a));
        }
    };

    template <typename A>
    struct sequence_traits<std::shared_ptr<finger_tree<A>>>
    {
        // (<|) :: a → FingerTree a → FingerTree a
        template <typename X, typename = std::enable_if_t<__is_perfectly_forwardable<X, A>::value>>
        constexpr static finger_tree<A> push_front(X &&a, std::shared_ptr<finger_tree<A>> &&b)
        {
            return b ? (b->index() == 1 ? sequence_traits<single<A>>::push_front(std::forward<X>(a), std::get<1>(*b))
                                        : sequence_traits<deep<A>>::push_front(std::forward<X>(a), std::get<2>(*b)))
                     : sequence_traits<empty>::push_front(std::forward<X>(a), empty_v);
        }
        template <typename X, typename = std::enable_if_t<__is_perfectly_forwardable<X, A>::value>>
        constexpr static finger_tree<A> push_front(X &&a, const std::shared_ptr<finger_tree<A>> &b)
        {
            return b ? (b->a.index() == 1 ? sequence_traits<single<A>>::push_front(std::forward<X>(a), std::get<1>((*b).a))
                                          : sequence_traits<deep<A>>::push_front(std::forward<X>(a), std::get<2>((*b).a)))
                     : sequence_traits<empty>::push_front(std::forward<X>(a), empty_v);
        }
    };

    template <typename A, typename B, typename = decltype(sequence_traits<__remove_cvref_t<B>>::push_front(std::declval<A>(), std::declval<B>()))>
    constexpr auto operator+=(A &&a, B &&b) -> decltype(sequence_traits<__remove_cvref_t<B>>::push_front(std::declval<A>(), std::declval<B>()))
    {
        return sequence_traits<__remove_cvref_t<B>>::push_front(std::forward<A>(a), std::forward<B>(b));
    }
}
#endif