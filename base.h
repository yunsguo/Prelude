/*
*	prelude.h:
*   emulating the Haskell prelude
*   Language: C++, Visual Studio 2017
*   Platform: Windows 10 Pro
*   Application: recreational
*   Author: Yunsheng Guo, yguo125@syr.edu
*/

/*
*
*   Package Operations:
*	defines some convenient aliases
*	includes type class implenmetations, Maybe type and its type traits implenmentations
*	some static operators for type classes
*	pattern matching snytax
*
*   Public Interface:
*	check Haskell prelude for futher info
*	Eq, Ord, Show all have their operator implenmeted
*	Functor fmap is implenmented
*	Applicative has the same partial apply operator as function type
*	Monad has A unique operator >>= for ap and operator >> for compose
*	due to the order needed to achieve do Haskell notation
*	Monad is no longer an Applicative
*
*   Build Process:
*   requires function.h and variant.h
*
*   Maintenance History:
*   June 6
*	first draft contains typeclass definitions and function application definition
*   August 12
*   refactor. move Maybe type into this for clarity and move all type class and traits declaration to meta.h
*	change pattern matching syntax and let pattern matching use Maybe
*	August 25
*	final review for publish
*
*
*/

#ifndef BASE_H
#define BASE_H

#include <variant>
#include <string>

#include "tmp.h"
#include "function.h"

namespace fcl
{

#ifdef PATTERN_MATCH_BY_DATA_TRAITS

    constexpr const auto data_npos = std::variant_npos;

    template <typename... As>
    using data = std::variant<As...>;

    template <typename A, typename Enable = void>
    struct data_traits : public not_possess_traits
    {
        using type = pack<>;

        constexpr static size_t elem_index(const A &);

        template <typename T>
        constexpr static T as(A);
    };

    template <typename A>
    using has_data_traits = std::bool_constant<data_traits<A>::possess>;

    template <typename A>
    constexpr size_t elem_index(const A &a)
    {
        static_assert(has_data_traits<A>::value);
        return data_traits<A>::elem_index(a);
    }

    template <typename T, typename A>
    constexpr T as(A a)
    {
        static_assert(has_data_traits<A>::value);
        return data_traits<A>::template as<T>(a);
    }

    template <typename... As>
    struct data_traits<data<As...>> : public possess_traits
    {
        using possess = std::false_type;

        using type = pack<As...>;

        constexpr static size_t elem_index(const data<As...> &data) { return data.index(); }

        template <typename T>
        constexpr static T as(data<As...> data)
        {
            static_assert(tmp::elem<T, type>::value);
            return data;
        }
    };

#endif

    template <typename T, typename Tag>
    struct new_type
    {
        using V = T;

        constexpr new_type() : value() {}

        template <typename G,
                  typename = std::enable_if_t<is_memberwise_assignable<V, G>::value>>
        constexpr new_type(G value) : value(std::forward<G>(value)) {}

        constexpr new_type(const new_type &other) = default;
        constexpr new_type(new_type &&other) = default;

        friend void swap(new_type &one, new_type &other) noexcept
        {
            using std::swap;

            swap(one.value, other.value);
        }

        new_type &operator=(new_type other)
        {
            swap(*this, other);
            return *this;
        }

        V &operator++(int) { return value; }

        const V &operator++(int) const { return value; }

        operator V() { return value; }

    private:
        V value;
    };

    // forward declaration

    template <typename, typename>
    struct Eq;

    template <typename, typename>
    struct Ord;

    template <typename, typename>
    struct Show;

    template <typename, typename>
    struct Semigroup;

    template <typename, typename>
    struct Monoid;

    template <typename, typename>
    struct Functor;

    template <typename, typename>
    struct Applicative;

    template <typename, typename>
    struct Monad;

    struct pertaining_type_class
    {
        //pertaining to a type class or not
        constexpr static const bool pertain = true;
    };

    struct not_pertaining_type_class
    {
        //pertaining to a type class or not
        constexpr static const bool pertain = false;
    };

    template <typename... Ts>
    struct __conjuct__pertain;

    template <typename T, typename... Ts>
    struct __conjuct__pertain<T, Ts...> : public std::bool_constant<T::pertain && __conjuct__pertain<Ts...>::value>
    {
    };

    template <>
    struct __conjuct__pertain<> : public std::true_type
    {
    };

    template <typename... Ts>
    using enable_type_class_t = std::enable_if_t<__conjuct__pertain<Ts...>::value>;

    template <template <typename...> typename Typeclass, typename A, typename B, typename Enable = void>
    struct is_of_same_type_class_instance : public std::false_type
    {
    };

    template <template <typename...> typename Typeclass, typename A, typename B>
    struct is_of_same_type_class_instance<Typeclass, A, B, std::enable_if_t<Typeclass<A>::pertain && Typeclass<B>::pertain && std::is_same<typename Typeclass<A>::template permutated<typename Typeclass<B>::parameter>, B>::value && std::is_same<typename Typeclass<B>::template permutated<typename Typeclass<A>::parameter>, A>::value>> : public std::true_type
    {
    };

    //The Eq class defines equality (==) and inequality (/=).
    template <typename A, typename Enable = void>
    struct Eq : public not_pertaining_type_class
    {
        /** REQUIRED **/
        constexpr static bool equals(const A &one, const A &other);
    };

    template <typename T, typename Enable = void>
    struct __has_builtin_equality_operator : public std::false_type
    {
    };

    template <typename... Args>
    void ____eval(Args...);

    template <typename T>
    struct __has_builtin_equality_operator<T, decltype(____eval<bool>(std::declval<T>() == std::declval<T>()))> : public std::true_type
    {
    };

    template <typename A>
    struct Eq<A, std::enable_if_t<__has_builtin_equality_operator<A>::value>> : public pertaining_type_class
    {
        constexpr static bool equals(const A &one, const A &other) { return one == other; }
    };

    template <typename A, typename = std::enable_if_t<Eq<A>::pertain && !__has_builtin_equality_operator<A>::value>>
    constexpr bool operator==(const A &one, const A &other) { return Eq<A>::equals(one, other); }

    template <typename A, typename = std::enable_if_t<Eq<A>::pertain && !__has_builtin_equality_operator<A>::value>>
    constexpr bool operator!=(const A &one, const A &other) { return !Eq<A>::equals(one, other); }

    enum Ordering
    {
        LT,
        EQ,
        GT
    };

    //The Ord class is used for totally ordered datatypes.
    template <typename A, typename Enable = void>
    struct Ord : public not_pertaining_type_class
    {
        // static_assert(Eq<A>::pertain);
        /** REQUIRED **/
        constexpr static Ordering compare(const A &one, const A &other);
    };

    template <typename T, typename Enable = void>
    struct __has_builtin_comparison_operator : public std::false_type
    {
    };

    template <typename T>
    struct __has_builtin_comparison_operator<T, decltype(____eval<bool, bool, bool, bool>(
                                                    std::declval<T>() > std::declval<T>(), std::declval<T>() < std::declval<T>(), std::declval<T>() >= std::declval<T>(), std::declval<T>() <= std::declval<T>()))> : public std::true_type
    {
    };

    template <typename A>
    struct Ord<A, std::enable_if_t<__has_builtin_comparison_operator<A>::value>> : public pertaining_type_class
    {
        constexpr static Ordering compare(const A &one, const A &other) { return one == other ? Ordering::EQ : one > other ? Ordering::GT
                                                                                                                           : Ordering::LT; }
    };

    template <typename A, typename = std::enable_if_t<Ord<A>::pertain && !__has_builtin_comparison_operator<A>::value>>
    constexpr bool operator<=(const A &one, const A &other) { return Ord<A>::compare(one, other) != Ordering::GT; }

    template <typename A, typename = std::enable_if_t<Ord<A>::pertain && !__has_builtin_comparison_operator<A>::value>>
    constexpr bool operator>=(const A &one, const A &other) { return Ord<A>::compare(one, other) != Ordering::LT; }

    template <typename A, typename = std::enable_if_t<Ord<A>::pertain && !__has_builtin_comparison_operator<A>::value>>
    constexpr bool operator<(const A &one, const A &other) { return Ord<A>::compare(one, other) == Ordering::LT; }

    template <typename A, typename = std::enable_if_t<Ord<A>::pertain && !__has_builtin_comparison_operator<A>::value>>
    constexpr bool operator>(const A &one, const A &other) { return Ord<A>::compare(one, other) == Ordering::GT; }

    using std::string;

    // Conversion of values to readable Strings.
    template <typename A, typename Enable = void>
    struct Show : public not_pertaining_type_class
    {
        /** REQUIRED **/
        constexpr static string show(const A &value);
    };

    template <>
    struct Show<string> : public pertaining_type_class
    {
        static string show(const string &value) { return value; }
    };

#ifdef _GLIBCXX_IOSTREAM

    template <typename T, typename Enable = void>
    struct __has_builtin_insersion_operator : public std::false_type
    {
    };

    void __ostream(std::ostream &);
    template <typename T>
    struct __has_builtin_insersion_operator<T,
                                            decltype(__ostream(std::declval<std::ostream &>() << std::declval<T>()))> : public std::true_type
    {
    };

#ifdef _GLIBCXX_SSTREAM

    template <typename A>
    struct Show<A, std::enable_if_t<__has_builtin_insersion_operator<A>::value && !std::is_convertible<A, string>::value>> : public pertaining_type_class
    {
        constexpr static string show(const A &value)
        {
            std::ostringstream sstr;
            sstr << value;
            return sstr.str();
        }
    };

#elif NATIVE_SHOW_BY_TO_STRING

    template <typename T, typename Enable = void>
    struct __has_to_string_implementation : public std::false_type
    {
    };

    void __string(string);

    template <typename T>
    struct __has_to_string_implementation<T,
                                          decltype(__string(std::to_string(std::declval<T>())))> : public std::true_type
    {
    };

    template <typename A>
    struct Show<A, std::enable_if_t<__has_to_string_implementation<A>::value && !std::is_convertible<A, string>::value>> : public pertaining_type_class
    {
        constexpr static string show(const A &value)
        {
            return std::to_string(value);
        }
    };

#endif
    template <typename A, typename = std::enable_if_t<Show<A>::pertain && !__has_builtin_insersion_operator<A>::value>>
    constexpr std::ostream &operator<<(std::ostream &out, const A &value)
    {
        return out << Show<A>::show(value);
    }
#endif

    // The class of semigroups (types with an associative binary operation).
    template <typename A, typename Enable = void>
    struct Semigroup : public not_pertaining_type_class
    {
        // An associative operation.
        /** REQUIRED **/
        constexpr static A op(A lhs, const A &rhs);
    };

    template <typename T, typename Enable = void>
    struct __has_builtin_semigroup_operator : public std::false_type
    {
    };

    template <typename T>
    struct __has_builtin_semigroup_operator<T, decltype(____eval<T>(std::declval<T>() + std::declval<T>()))> : public std::true_type
    {
    };

    template <typename A>
    struct Semigroup<A, std::enable_if_t<__has_builtin_semigroup_operator<A>::value>> : public pertaining_type_class
    {
        constexpr static A op(A lhs, const A &rhs) { return lhs + rhs; }
    };

    // An associative operation.
    template <typename A, typename = std::enable_if_t<Semigroup<A>::pertain && !__has_builtin_semigroup_operator<A>::value>>
    constexpr A operator+(A lhs, const A &rhs) { return Semigroup<A>::op(lhs, rhs); }

    // The class of monoids (types with an associative binary operation that has an identity).
    template <typename A, typename Enable = void>
    struct Monoid : public not_pertaining_type_class
    {
        // static_assert(Semigroup<A>::pertain);

        /** REQUIRED **/
        // Identity of mappend
        constexpr static const A mempty = A();
    };

    template <typename T, typename Enable = void>
    struct __can_be_assigned_by_zero : public std::false_type
    {
    };

    template <typename T>
    struct __can_be_assigned_by_zero<T, decltype(____eval<T>(std::declval<T &>() = 0))> : public std::true_type
    {
    };

    template <typename T, typename Enable = void>
    struct __can_be_assigned_by_empty_string : public std::false_type
    {
    };

    template <typename T>
    struct __can_be_assigned_by_empty_string<T, decltype(____eval<T>(std::declval<T &>() = ""))> : public std::true_type
    {
    };

    template <typename A>
    struct Monoid<A, std::enable_if_t<__has_builtin_semigroup_operator<A>::value && __can_be_assigned_by_zero<A>::value>> : public pertaining_type_class
    {
        constexpr static const A mempty = 0;
    };

    template <typename A>
    struct Monoid<A, std::enable_if_t<__has_builtin_semigroup_operator<A>::value && __can_be_assigned_by_empty_string<A>::value>> : public pertaining_type_class
    {
        constexpr static const A mempty = "";
    };

    // template <typename A>
    // struct Monoid<A, std::enable_if_t<__can_be_assigned_by<A, "">::value>> : public pertaining_type_class
    // {
    //     constexpr static const A mempty = "";
    // };

    // A type f is a Functor if it provides a function fmap which, given any types a and b lets you apply any function from (a -> b) to turn an f a into an f b, preserving the structure of f.
    template <typename FA, typename Enable = void>
    struct Functor : public not_pertaining_type_class
    {
        /** REQUIRED **/

        template <typename A>
        using permutated = pack<A>;

        /** REQUIRED **/
        // enclosed type parameter
        using parameter = undefined;

        /** REQUIRED **/
        template <typename Func>
        constexpr static permutated<partial_applied_t<Func>> fmap(Func, FA)
        {
            static_assert(is_of_same_type_class_instance<Functor, FA, first_variable_t<Func>>::value);
        }

        /** REQUIRED **/
        // Replace all locations in the input with the same value. The default definition is fmap . const, but this may be overridden with a more efficient version.
        template <typename B>
        static permutated<B> replace(B, FA);
    };

    template <typename Func, typename FA, typename Enable = void>
    struct __is_fmapable : public std::false_type
    {
    };

    template <typename Func, typename FA>
    struct __is_fmapable<Func, FA,
                         std::enable_if_t<Functor<FA>::pertain && std::is_same<FA, typename Functor<FA>::template permutated<first_variable_t<Func>>>::value>> : public std::true_type
    {
    };

    template <typename Func, typename FA, typename = std::enable_if_t<__is_fmapable<Func, FA>::value>>
    typename Functor<FA>::template permutated<partial_applied_t<Func>> operator-(Func f, FA fa) { return Functor<FA>::fmap(f, fa); }

    template <typename B, typename FA, typename = enable_type_class_t<Functor<FA>>>
    typename Functor<FA>::template permutated<B> operator%(B b, FA fa) { return Functor<FA>::template replace<B>(b, fa); }

    // A functor with application, providing operations to embed pure expressions (pure), and ap computations and combine their results (<*> and liftA2).
    template <typename FA, typename Enable = void>
    struct Applicative : public not_pertaining_type_class
    {
        // static_assert(Functor<MA>::pertain);

        /** REQUIRED **/

        template <typename A>
        using permutated = pack<A>;

        /** REQUIRED **/
        // enclosed type parameter
        using parameter = undefined;

        /** REQUIRED **/
        // Lift a value.
        constexpr static FA pure(parameter);

        /** REQUIRED **/
        // Sequential application.
        // A few functors support an implementation of <*> that is more efficient than the default one.
        template <typename FFunc>
        constexpr static permutated<partial_applied_t<typename Applicative<FFunc>::parameter>> ap(FFunc, FA)
        {
            static_assert(is_of_same_type_class_instance<Applicative, FFunc, FA>::value);
            using Func = typename Applicative<FFunc>::parameter;
            static_assert(is_function_like<Func>::value);
            using A = typename Applicative<FA>::parameter;
            static_assert(std::is_same<first_variable_t<Func>, A>::value);
        }

        // Lift a binary function to actions.
        // Some functors support an implementation of liftA2 that is more efficient than the default one. In particular, if fmap is an expensive operation, it is likely better to use liftA2 than to fmap over the structure and then use <*>.
        // template <typename Func, typename = std::enable_if_t<function_traits<Func>::Arity >= 2>>
        // constexpr static L<partial_applied_t<partial_applied_t<Func>>> liftA2(Func, L<first_variable_t<Func>>, L<index_t<1, Func>>);

        // Sequence actions, discarding the value of the first argument.
        template <typename FB>
        constexpr static FA right_tie(FB, FA)
        {
            static_assert(is_of_same_type_class_instance<Applicative, FB, FA>::value);
        }

        // Sequence actions, discarding the value of the second argument.
        template <typename FB>
        constexpr static FA left_tie(FA, FB)
        {
            static_assert(is_of_same_type_class_instance<Applicative, FB, FA>::value);
        }
    };

    template <typename FFunc, typename FA, typename Enable = void>
    struct __is_apable : public std::false_type
    {
    };

    template <typename FFunc, typename FA>
    struct __is_apable<FFunc, FA,
                       std::enable_if_t<
                           is_of_same_type_class_instance<Applicative, FFunc, FA>::value &&
                           is_function_like<typename Applicative<FFunc>::parameter>::value &&
                           std::is_same<first_variable_t<typename Applicative<FFunc>::parameter>, typename Applicative<FA>::parameter>::value>> : public std::true_type
    {
    };

    template <typename FFunc, typename FA, typename = std::enable_if_t<__is_apable<FFunc, FA>::value>>
    typename Applicative<FFunc>::template permutated<partial_applied_t<typename Applicative<FFunc>::parameter>> operator*(FFunc ff, FA fa) { return Applicative<FA>::ap(ff, fa); }

    // A monoid on applicative functors.
    template <typename A, typename Enable = void>
    struct Alternative : public not_pertaining_type_class
    {
        // static_assert(Applicative<MA>::pertain);

        /** REQUIRED **/
        constexpr static const A empty = A();

        /** REQUIRED **/
        constexpr static A alter(A, const A &);
    };

    template <typename A, typename = std::enable_if_t<Alternative<A>::pertain>>
    constexpr A operator|(A lhs, const A &rhs) { return Alternative<A>::alter(lhs, rhs); }

    // The Monad class defines the basic operations over a monad, a concept from a branch of mathematics known as category theory. From the perspective of a Haskell programmer, however, it is best to think of a monad as an abstract datatype of actions. Haskell's do expressions provide a convenient syntax for writing monadic expressions.
    template <typename MA, typename Enable = void>
    struct Monad : public not_pertaining_type_class
    {
        // static_assert(Applicative<MA>::pertain);

        /** REQUIRED **/

        template <typename A>
        using permutated = pack<A>;

        /** REQUIRED **/
        // enclosed type parameter
        using parameter = undefined;

        /** REQUIRED **/
        // Lift a value.
        constexpr static MA pure(parameter);

        /** REQUIRED **/
        template <typename Func>
        constexpr static partial_applied_t<Func> bind(MA, Func);

        /** REQUIRED **/
        template <typename MB>
        constexpr static MB compose(MA, MB);
    };

    template <typename FA, typename FB, typename = std::enable_if_t<is_of_same_type_class_instance<Applicative, FA, FB>::value>, typename = std::enable_if_t<!is_of_same_type_class_instance<Monad, FA, FB>::value>>
    FB operator<<(FA fa, FB fb) { return Applicative<FA>::template right_tie<FB>(fa, fb); }

    template <typename FA, typename FB, typename = std::enable_if_t<is_of_same_type_class_instance<Applicative, FA, FB>::value>>
    FA operator>>(FA fa, FB fb) { return Applicative<FA>::template left_tie<FB>(fa, fb); }

    template <typename Func, typename MA, typename Enable = void>
    struct is_monadically_bindable : public std::false_type
    {
    };

    template <typename Func, typename MA>
    struct is_monadically_bindable<Func,
                                   MA,
                                   std::enable_if_t<Monad<MA>::pertain &&
                                                    conjunct<
                                                        is_function_like<Func>,
                                                        std::is_same<typename Monad<return_type_t<Func>>::template permutated<typename Monad<MA>::parameter>, MA>, std::is_same<typename Monad<MA>::template permutated<typename Monad<return_type_t<Func>>::parameter>, return_type_t<Func>>, std::is_same<MA, typename Monad<MA>::template permutated<first_variable_t<Func>>>>::value>> : public std::true_type
    {
    };

    template <typename Func, typename MA, typename = std::enable_if_t<is_monadically_bindable<Func, MA>::value>>
    partial_applied_t<Func> operator<=(Func f, MA ma) { return Monad<MA>::bind(ma, f); }

    template <typename MA, typename MB, typename = std::enable_if_t<is_of_same_type_class_instance<Monad, MA, MB>::value>>
    MB operator<<(MA ma, MB mb) { return Monad<MA>::template compose<MB>(ma, mb); }

    // When a value is bound in do-notation, the pattern on the left hand side of <- might not match. In this case, this class provides a function to recover.
    template <typename MA, typename Enable = void>
    struct MonadFail : public not_pertaining_type_class
    {
        // static_assert(Monad<MA>::pertain);

        constexpr static MA fail(string);
    };

    template <typename F>
    using is_unary_function = std::bool_constant<is_function_like<F>::value && length<F>::value == 2>;

    template <typename F>
    using second_variable_t = tmp::index_t<1, F>;

    template <typename F>
    using is_right_reducer_function = std::bool_constant<is_function_like<F>::value && length<F>::value == 3 && std::is_same<second_variable_t<F>, return_type_t<F>>::value>;

    // The Foldable class represents data structures that can be reduced to a summary value one element at a time. Strict left-associative folds are a good fit for space-efficient reduction, while lazy right-associative folds are a good fit for corecursive iteration, or for folds that short-circuit after processing an initial subsequence of the structure's elements.
    template <typename TA>
    struct Foldable : public not_pertaining_type_class
    {

        template <typename A>
        using permutated = pack<A>;

        using parameter = undefined;

        // Map each element of the structure into a monoid, and combine the results with (<>). This fold is right-associative and lazy in the accumulator. For strict left-associative folds consider foldMap` instead.
        template <typename Func>
        constexpr static return_type_t<Func> foldMap(Func, TA)
        {
            static_assert(is_unary_function<Func>::value);
            static_assert(std::is_same<first_variable_t<Func>, parameter>::value);
            static_assert(Monoid<return_type_t<Func>>::pertain);
        }

        // Right-associative fold of a structure, lazy in the accumulator.
        template <typename Func>
        constexpr static return_type_t<Func> foldr(Func &&, return_type_t<Func>, TA)
        {
            static_assert(is_right_reducer_function<Func>::value);
            static_assert(std::is_same<first_variable_t<Func>, parameter>::value);
        }
    };

    // Functors representing data structures that can be traversed from left to right, performing an action on each element.
    template <typename TA>
    struct Traversable : public not_pertaining_type_class
    {
        // static_assert(Functor<TA>::pertain);

        // static_assert(Foldable<TA>::pertain);

        template <typename A>
        using permutated = pack<A>;

        using parameter = undefined;

        //Map each element of a structure to an action, evaluate these actions from left to right, and collect the results.
        template <typename F>
        constexpr static typename Applicative<return_type_t<F>>::template permutated<permutated<typename Applicative<return_type_t<F>>::parameter>> traverse(F, TA)
        {
            static_assert(is_unary_function<F>::value);
            static_assert(std::is_same<first_variable_t<F>, parameter>::value);
            static_assert(Applicative<return_type_t<F>>::pertain);
        }
    };

    // template <typename... Ts>
    // struct __conjuct__pertain;

    // template <typename T, typename... Ts>
    // struct __conjuct__pertain<T, Ts...> : public std::bool_constant<T::pertain && __conjuct__pertain<Ts...>::value>
    // {
    // };

    // template <>
    // struct __conjuct__pertain<> : public std::true_type
    // {
    // };

    // template <typename... Ts>
    // using enable_type_class_t = std::enable_if_t<__conjuct__pertain<Ts...>::value>;

    // template <template <typename...> typename Typeclass, typename A, typename B, typename Enable = void>
    // struct is_of_same_type_class_instance : public std::false_type
    // {
    // };

    // template <template <typename...> typename Typeclass, typename A, typename B>
    // struct is_of_same_type_class_instance<Typeclass, A, B, std::enable_if_t<Typeclass<A>::pertain && Typeclass<B>::pertain && std::is_same<typename Typeclass<A>::template permutated<typename Typeclass<B>::parameter>, B>::value && std::is_same<typename Typeclass<B>::template permutated<typename Typeclass<A>::parameter>, A>::value>> : public std::true_type
    // {
    // };
}
#endif