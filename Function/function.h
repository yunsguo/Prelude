/*
*   meta_traits.h:
*   forward declaration header plus some miscellaneous type inference stuff
*   Language: C++, Visual Studio 2017
*   Platform: Windows 10 Pro
*   Application: recreational
*   Author: Yunsheng Guo, yguo125@syr.edu
*/

/*
*
*   Package Operations:
*	None, almost all definition will be implenmented or specilized later
*	forward types such that the FP library does not depends on variant or function implenmentation
*
*   Public Interface:
*	None
*
*   Build Process:
*   requires fcl.h
*
*   Maintenance History:
*   June 6
*   contain and separate FP library from fcl.h such that
*	fcl.h can be used for other projects without any problem
*   August 12
*   refactor into A forward declaration header with some useful stuff
*	August 25
*	final review for publish
*	July 2021
*	cmake conversion
*	August 2021
*	major refactor
*
*
*/

#ifndef FUNCTION_H
#define FUNCTION_H

#include <type_traits>
#include <variant>

#include "tmp.h"
#include "tuple.h"

namespace fcl
{

    using tmp::pack;

    using tmp::undefined;

    using tmp::possess_traits;

    using tmp::not_possess_traits;

    // At most 256 arguments are allowed in a given function
    using byte = unsigned char;

    // The function_traits class defines the basic operations over a type considered to be a function. Minimal complete definition includes Arity, type, partial_applied, and partial_apply method.
    template <typename F, typename Enable = void>
    struct function_traits : public not_possess_traits
    {
        //Arity is the number of arguments or operands taken by A function
        constexpr static const byte Arity = 0;

        // pack corrisponding type
        using type = undefined;

        using partial_applied = undefined;

        // optional
        using first_variable = undefined;

        // optional
        using return_type = undefined;

        //apply the first parameter and return an partially applied function
        constexpr static partial_applied partial_apply(F, first_variable);
    };

    template <typename F>
    using is_function_like = std::bool_constant<function_traits<F>::possess>;

}

namespace tmp
{

    template <typename A>
    struct pack_traits<A, std::enable_if_t<fcl::is_function_like<A>::value>> : public possess_traits
    {
        using type = typename fcl::function_traits<A>::type;
    };

}

namespace fcl
{
    template <typename F>
    using partial_applied_t = typename function_traits<F>::partial_applied;

    template <typename F>
    using first_variable_t = tmp::head_t<F>;

    template <typename F>
    using return_type_t = tmp::last_t<F>;

    // template <typename Assignee, typename Assignor>
    // using is_memberwise_assignable = std::is_assignable<std::add_lvalue_reference_t<Assignee>, std::decay_t<Assignor>>;

    template <typename Target, typename Value>
    using is_memberwise_assignable = disjunct<std::is_convertible<Value, Target>, std::is_assignable<std::add_lvalue_reference_t<Target>, std::decay_t<Value>>>;

    template <typename F, typename A, typename = std::enable_if_t<is_function_like<F>::value && is_memberwise_assignable<first_variable_t<F>, A>::value>>
    constexpr partial_applied_t<F> operator<(F &&f, A &&arg) noexcept
    {
        return function_traits<F>::partial_apply(std::forward<F>(f), std::forward<first_variable_t<F>>(arg));
    }

    template <typename T>
    struct __is_native_function : public std::false_type
    {
    };

    template <typename R, typename... Args>
    struct __is_native_function<R (*)(Args...)> : public std::true_type
    {
    };

    template <typename R, typename... Args>
    struct __is_native_function<R (*&)(Args...)> : public std::true_type
    {
    };

    template <typename R, typename... Args>
    struct __is_native_function<R(Args...)> : public std::true_type
    {
    };

    static_assert(__is_native_function<int(int, int)>::value);
    static_assert(__is_native_function<int (*)(int, int)>::value);
    static_assert(__is_native_function<int (*&)(int, int)>::value);

    template <typename A, typename B>
    using __is_not = std::bool_constant<!std::is_same_v<A, B>>;

    template <typename A>
    using __is_not_void = std::bool_constant<!std::is_same_v<A, void>>;

    template <typename R, typename Arg>
    struct function_traits<R (*)(Arg), std::enable_if_t<all<__is_not_void, pack<R, Arg>>::value>> : public possess_traits
    {
        //original type
        using F = R (*)(Arg);

        constexpr const static byte Arity = 1;

        using type = pack<Arg, R>;

        using partial_applied = R;

        constexpr static R partial_apply(F f, Arg arg) { return f(std::forward<Arg>(arg)); }
    };

    template <byte Arity, typename F, typename Enable = void>
    struct partial_function;

    template <typename R, typename Arg1, typename Arg2, typename... Args>
    struct function_traits<R (*)(Arg1, Arg2, Args...), std::enable_if_t<all<__is_not_void, pack<R, Arg1, Arg2, Args...>>::value>> : public possess_traits
    {
        //original type
        using F = R (*)(Arg1, Arg2, Args...);

        constexpr const static byte Arity = 2 + sizeof...(Args);

        using type = pack<Arg1, Arg2, Args..., R>;

        using partial_applied = partial_function<Arity - 1, F>;

        constexpr static partial_applied partial_apply(F f, Arg1 arg) noexcept { return partial_applied(std::forward<F>(f), std::forward<Arg1>(arg)); }
    };

    template <typename A>
    using __native_value_type_t = std::remove_reference_t<std::decay_t<A>>;

    template <typename F>
    struct function_traits<F,
                           std::enable_if_t<is_function_like<__native_value_type_t<F>>::value && is_memberwise_assignable<__native_value_type_t<F>, F>::value>>
        : public possess_traits
    {
        using base = function_traits<__native_value_type_t<F>>;

        constexpr const static byte Arity = base::Arity;

        using type = typename base::type;

        using partial_applied = typename base::partial_applied;

        using first_variable = tmp::head_t<type>;

        constexpr static partial_applied partial_apply(F f, first_variable arg) noexcept { return base::partial_apply(f, std::forward<first_variable>(arg)); }
    };

    template <typename F, typename Enable = void>
    struct boxed_function;

    template <typename F>
    struct boxed_function<F>
    {
        using value_type = __native_value_type_t<F>;

        static_assert(is_function_like<value_type>::value && __is_native_function<value_type>::value, "can only box a function-like type.");

        template <typename G, typename = std::enable_if_t<is_memberwise_assignable<value_type, G>::value>>
        constexpr boxed_function(G &&f) : _f(f) {}
        constexpr boxed_function(const boxed_function &other) = default;
        constexpr boxed_function(boxed_function &&other) = default;

        friend void swap(boxed_function &one, boxed_function &other) noexcept
        {
            using std::swap;

            swap(one._f, other._f);
        }

        boxed_function &operator=(boxed_function other)
        {
            swap(*this, other);
            return *this;
        }

        friend struct function_traits<boxed_function>;

    private:
        value_type _f;
    };

    template <typename F>
    constexpr boxed_function<F> boxed(F f)
    {
        return boxed_function<F>(std::forward<F>(f));
    }

    template <typename F>
    struct function_traits<boxed_function<F>> : public function_traits<typename boxed_function<F>::value_type>
    {
        using base = function_traits<typename boxed_function<F>::value_type>;

        constexpr const static byte Arity = base::Arity;

        using type = typename base::type;

        using partial_applied = typename base::partial_applied;

        using first_variable = tmp::head_t<type>;

        constexpr static partial_applied partial_apply(boxed_function<F> f, first_variable arg) { return base::partial_apply(std::move(f._f), std::forward<first_variable>(arg)); }
    };

    template <typename A>
    using __uncurried_is_memberwise_assignable = uncurry<is_memberwise_assignable, A>;

    template <typename A, typename B>
    using are_equivalent_parameters = tmp::conjunct<std::bool_constant<length<A>::value == length<B>::value>, tmp::all<__uncurried_is_memberwise_assignable, tmp::zip_t<A, B>>>;

    template <byte Arity, typename F>
    struct partial_function<Arity, F, std::enable_if_t<Arity + 2 == length<F>::value>>
    {
        static_assert(is_function_like<F>::value);
        static_assert(length<F>::value > 2);

        //Rank is the number of binding arguments contained by the partial function
        constexpr const static byte Rank = 1;

        using binding = tmp::inferred_from_t<tuple, tmp::take_t<Rank, F>>;

        using params = tmp::inferred_from_t<tuple, tmp::drop_t<Rank, tmp::init_t<F>>>;

        using parent_variable = tmp::head_t<F>;

        using R = tmp::last_t<F>;

        using binding_variant = std::variant<binding, R>;

        template <typename P, typename A, typename = std::enable_if_t<is_memberwise_assignable<F, P>::value && is_memberwise_assignable<parent_variable, A>::value>>
        constexpr partial_function(P &&f, A &&arg) : _f(std::forward<P>(f)),
                                                     _v(std::in_place_index<0>, forward_as_tuple<parent_variable>(std::forward<parent_variable>(arg))){};

        template <typename A, typename = std::enable_if_t<is_memberwise_assignable<R, A>::value>>
        constexpr partial_function(A &&arg) : _f(nullptr),
                                              _v(std::in_place_index<1>, std::forward<R>(arg)) {}

        constexpr partial_function(const partial_function &other) = default;
        constexpr partial_function(partial_function &&other) = default;

        friend void swap(partial_function &one, partial_function &other) noexcept
        {
            using std::swap;

            swap(one._f, other._f);
            swap(one._v, other._v);
        }

        partial_function &operator=(partial_function other)
        {
            swap(*this, other);
            return *this;
        }

        template <typename... As, typename = std::enable_if_t<are_equivalent_parameters<pack<As...>, params>::value>>
        constexpr R operator()(As &&...args) const
        {
            return _v.index() == 0 ? std::apply(_f, std::tuple_cat(std::get<0>(_v), std::forward_as_tuple<As...>(std::forward<As>(args)...))) : std::get<1>(_v);
        }

        template <byte, typename, typename>
        friend struct partial_function;

        friend struct function_traits<partial_function>;

    private:
        F _f;
        binding_variant _v;
    };

    template <byte Arity, typename F>
    struct partial_function<Arity, F, std::enable_if_t<Arity != 0 && Arity + 2 < length<F>::value>>
    {
        static_assert(is_function_like<F>::value);
        static_assert(length<F>::value > 2);

        //Rank is the number of binding arguments contained by the partial function
        constexpr const static byte Rank = length<F>::value - 1 - Arity;

        using binding = tmp::inferred_from_t<tuple, tmp::take_t<Rank, F>>;

        using params = tmp::inferred_from_t<tuple, tmp::drop_t<Rank, tmp::init_t<F>>>;

        using parent_variable = tmp::index_t<Rank - 1, F>;

        using R = tmp::last_t<F>;

        using binding_variant = std::variant<binding, R>;

        template <typename A,
                  typename = std::enable_if_t<is_memberwise_assignable<parent_variable, A>::value>>
        constexpr partial_function(partial_function<Arity + 1, F> parent, A &&arg) : _f(parent._f),
                                                                                     _v(parent._v.index() == 0
                                                                                            ? binding_variant(std::in_place_index<0>, std::tuple_cat(std::move(std::get<0>(parent._v)), forward_as_tuple<parent_variable>(std::forward<parent_variable>(arg))))
                                                                                            : binding_variant(std::in_place_index<1>, std::get<1>(parent._v))) {}

        template <typename A, typename = std::enable_if_t<is_memberwise_assignable<R, A>::value>>
        constexpr partial_function(A &&arg) : _f(nullptr),
                                              _v(std::in_place_index<1>, std::forward<R>(arg)) {}

        constexpr partial_function(const partial_function &other) = default;
        constexpr partial_function(partial_function &&other) = default;

        friend void swap(partial_function &one, partial_function &other) noexcept
        {
            using std::swap;

            swap(one._f, other._f);
            swap(one._v, other._v);
        }

        partial_function &operator=(partial_function other)
        {
            swap(*this, other);
            return *this;
        }

        template <typename... As, typename = std::enable_if_t<are_equivalent_parameters<pack<As...>, params>::value>>
        constexpr R operator()(As &&...args) const
        {
            return _v.index() == 0 ? std::apply(_f, std::tuple_cat(std::get<0>(_v), std::forward_as_tuple<As...>(std::forward<As>(args)...))) : std::get<1>(_v);
        }

        template <byte, typename, typename>
        friend struct partial_function;

        friend struct function_traits<partial_function>;

    private:
        F _f;
        binding_variant _v;
    };

    template <byte N, typename FP>
    struct function_traits<partial_function<N, FP>,
                           std::enable_if_t<N != 1>> : public function_traits<FP>
    {
        using F = partial_function<N, FP>;

        using base = function_traits<FP>;

        constexpr const static byte Arity = N;

        using type = tmp::drop_t<F::Rank, typename base::type>;

        using partial_applied = partial_function<Arity - 1, FP>;

        using first_variable = tmp::index_t<F::Rank, FP>;

        constexpr static partial_applied partial_apply(F f, first_variable arg)
        {
            return partial_applied(std::forward<F>(f), std::forward<first_variable>(arg));
        }
    };

    template <typename FP>
    struct function_traits<partial_function<1, FP>> : public function_traits<FP>
    {
        using F = partial_function<1, FP>;

        constexpr const static byte Arity = 1;

        using type = tmp::drop_t<F::Rank, FP>;

        using partial_applied = tmp::last_t<FP>;

        using first_variable = tmp::index_t<F::Rank, FP>;

        using R = tmp::last_t<FP>;

        constexpr static R partial_apply(F f, first_variable arg)
        {
            return f(std::forward<first_variable>(arg));
        }
    };

    template <typename L1, typename L2>
    struct can_be_composited
        : public std::conditional_t<tmp::all<is_function_like, pack<L1, L2>>::value, can_be_composited<tmp::pack_traits_t<L1>, tmp::pack_traits_t<L2>>, std::false_type>
    {
    };

    template <typename T, typename A, typename B, typename C, typename... Cs>
    struct can_be_composited<pack<T, C, Cs...>, pack<A, B>>
        : public std::bool_constant<std::is_same<tmp::inferred_pack_by_traits_t<B>, tmp::inferred_pack_by_traits_t<T>>::value>
    {
    };

    template <typename A, typename B1, typename B2, typename... Bs, typename T, typename C, typename... Cs>
    struct can_be_composited<pack<T, C, Cs...>, pack<A, B1, B2, Bs...>>
        : public std::bool_constant<std::is_same<tmp::inferred_pack_by_traits_t<pack<B1, B2, Bs...>>, tmp::inferred_pack_by_traits_t<T>>::value>
    {
    };

    static_assert(can_be_composited<pack<int, float>, pack<double, int>>::value);

    static_assert(can_be_composited<pack<pack<int, int>, int, int, int>, pack<int, int, int>>::value);

    static_assert(!can_be_composited<pack<pack<int, int>, int, int, int>, pack<int, int, int, int>>::value);

    template <typename F, typename G>
    struct composite_function
    {

        // assume F and G is function-like

        static_assert(tmp::all<is_function_like, pack<F, G>>::value);

        static_assert(can_be_composited<F, G>::value);

        using Fp = __native_value_type_t<F>;

        using Gp = __native_value_type_t<G>;

        constexpr composite_function(Fp f, Gp g) : _f(f), _g(g) {}

        template <typename H, typename I,
                  typename = std::enable_if_t<is_memberwise_assignable<Fp, H>::value && is_memberwise_assignable<Gp, I>::value>>
        constexpr composite_function(H f, I g) : _f(std::forward<H>(f)), _g(std::forward<I>(g)) {}

        constexpr composite_function(const composite_function &other) = default;
        constexpr composite_function(composite_function &&other) = default;

        friend void swap(composite_function &one, composite_function &other) noexcept
        {
            using std::swap;

            swap(one._f, other._f);
            swap(one._g, other._g);
        }

        composite_function &operator=(composite_function other)
        {
            swap(*this, other);
            return *this;
        }

        using params_pack = tmp::cons_t<tmp::head_t<G>, tmp::tail_t<tmp::init_t<G>>>;

        template <typename... As,
                  typename = std::enable_if_t<are_equivalent_parameters<params_pack, pack<As...>>::value>>
        constexpr return_type_t<F> operator()(first_variable_t<G> arg, As &&...args) const
        {
            return _f(function_traits<G>::partial_apply(_g, std::forward<first_variable_t<G>>(arg)), std::forward<As>(args)...);
        }

        friend struct function_traits<composite_function>;

    private:
        Fp _f;
        Gp _g;
    };

    template <typename F, typename G>
    struct function_traits<composite_function<F, G>, std::enable_if_t<can_be_composited<F, G>::value>> : public possess_traits
    {
        using X = composite_function<F, G>;

        using type = tmp::cons_t<tmp::head_t<G>, tmp::tail_t<F>>;

        constexpr const static byte Arity = tmp::length<type>::value - 1;

        using partial_applied = partial_applied_t<F>;

        using first_variable = tmp::head_t<type>;

        using return_type = tmp::last_t<type>;

        constexpr static partial_applied partial_apply(X f, first_variable arg)
        {
            return function_traits<decltype(f._f)>::partial_apply(f._f, function_traits<decltype(f._g)>::partial_apply(f._g, std::forward<first_variable>(arg)));
        }
    };

    template <typename F, typename G, typename = std::enable_if_t<can_be_composited<F, G>::value>>
    constexpr composite_function<F, G> operator*(F &&f, G &&g) noexcept
    {
        return composite_function<F, G>(std::forward<F>(f), std::forward<G>(g));
    }

    template <typename F>
    struct flip_function
    {

        // assume F and G is function-like

        static_assert(is_function_like<F>::value);

        static_assert(length<F>::value > 2);

        using Fp = __native_value_type_t<F>;

        template <typename G,
                  typename = std::enable_if_t<is_memberwise_assignable<Fp, G>::value>>
        constexpr flip_function(G f) : _f(std::forward<G>(f)) {}

        constexpr flip_function(const flip_function &other) = default;
        constexpr flip_function(flip_function &&other) = default;

        friend void swap(flip_function &one, flip_function &other) noexcept
        {
            using std::swap;

            swap(one._f, other._f);
        }

        flip_function &operator=(flip_function other)
        {
            swap(*this, other);
            return *this;
        }

        using params_pack = tmp::init_t<tmp::flip_t<F>>;

        template <typename A, typename B, typename... Cs,
                  typename = std::enable_if_t<are_equivalent_parameters<params_pack, pack<A, B, Cs...>>::value>>
        constexpr return_type_t<F> operator()(A &&a, B &&b, Cs &&...args) const
        {
            return _f(std::forward<B>(b), std::forward<A>(a), std::forward<Cs>(args)...);
        }

        friend struct function_traits<flip_function>;

    private:
        Fp _f;
    };

    template <typename F>
    struct function_traits<flip_function<F>, std::enable_if_t<(length<F>::value > 2)>> : public function_traits<F>
    {
        using X = flip_function<F>;

        using type = tmp::flip_t<F>;

        constexpr const static byte Arity = tmp::length<F>::value - 1;

        using partial_applied = partial_function<Arity - 1, X>;

        using first_variable = tmp::head_t<type>;

        using return_type = tmp::last_t<type>;

        constexpr static partial_applied partial_apply(X f, first_variable arg)
        {
            return partial_applied(std::forward<X>(f), std::forward<first_variable>(arg));
        }
    };

    template <typename Func>
    partial_applied_t<Func> flip(Func f)
    {
        static_assert(is_function_like<Func>::value);
        return flip_function(f);
    }
}

#endif