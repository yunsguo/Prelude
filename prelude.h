
#ifndef PRELUDE_H
#define PRELUDE_H

#include <optional>

#include "base.h"
#include "show.h"

namespace fcl
{
    template <typename A, typename B = A>
    constexpr B id(A a)
    {
        static_assert(std::is_convertible<A, B>::value);
        return a;
    }

    template <typename A, typename B, typename C = B>
    constexpr C right(A, B b)
    {
        static_assert(std::is_convertible<B, C>::value);
        return b;
    }

    template <typename A, typename B, typename C = A>
    constexpr C left(A a, B)
    {
        static_assert(std::is_convertible<A, C>::value);
        return a;
    }

    template <typename A>
    using maybe = std::optional<A>;

    constexpr const auto nothing = std::nullopt;

    template <typename Func>
    constexpr partial_applied_t<Func> maybe_apply(partial_applied_t<Func> defaulted, Func f, maybe<first_variable_t<Func>> ma)
    {
        static_assert(is_function_like<Func>::value);
        return ma.has_value() ? function_traits<Func>::partial_apply(f, ma.value()) : defaulted;
    }

    // The Eq class defines equality (==) and inequality (/=).
    // Disabble due to optional native implementation
    //  template <typename A>
    //  struct Eq<maybe<A>, enable_type_class_t<Eq<A>>> : public pertaining_type_class
    //  {
    //      constexpr static bool equals(const maybe<A> &one, const maybe<A> &other)
    //      {
    //          return one.has_value()     ? other.has_value() ? one.value() == other.value() : false
    //                 : other.has_value() ? false
    //                                     : true;
    //      }
    //  };

    // The Ord class is used for totally ordered datatypes.
    // Disabble due to optional native implementation
    //  template <typename A>
    //  struct Ord<maybe<A>, enable_type_class_t<Ord<A>>> : public pertaining_type_class
    //  {
    //      static_assert(Eq<maybe<A>>::pertain);
    //      constexpr static Ordering compare(const maybe<A> &one, const maybe<A> &other)
    //      {
    //          return one.has_value()     ? other.has_value() ? Ord<A>::compare(one.value(), other.value()) : Ordering::GT
    //                 : other.has_value() ? Ordering::LT
    //                                     : Ordering::EQ;
    //      }
    //  };

    template <typename A, typename = enable_type_class_t<Show<A>>>
    std::ostream &operator<<(std::ostream &os, const maybe<A> &value)
    {
        return value.has_value() ? os << "just " << value.value() : os << "nothing";
    }

    // The class of semigroups (types with an associative binary operation).
    template <typename A>
    struct Semigroup<maybe<A>, enable_type_class_t<Semigroup<A>>> : public pertaining_type_class
    {
        // An associative operation.
        constexpr static maybe<A> op(maybe<A> lhs, const maybe<A> &rhs)
        {
            return lhs.has_value() ? rhs.has_value() ? Semigroup<A>::op(lhs.value(), rhs.value()) : nothing
                                   : nothing;
        }
    };

    // The class of monoids (types with an associative binary operation that has an identity).
    template <typename A>
    struct Monoid<maybe<A>> : public Semigroup<A>
    {
        static_assert(Semigroup<maybe<A>>::pertain);
        constexpr static const maybe<A> mempty = nothing;
        constexpr static auto mappend = Semigroup<maybe<A>>::op;
    };

    // A type f is a Functor if it provides a function fmap which, given any types a and b lets you apply any function from (a -> b) to turn an f a into an f b, preserving the structure of f.
    template <typename A>
    struct Functor<maybe<A>> : public pertaining_type_class
    {
        using FA = maybe<A>;

        template <typename B>
        using permutated = maybe<B>;

        using parameter = A;

        template <typename Func>
        constexpr static permutated<partial_applied_t<Func>> fmap(Func f, FA ma)
        {
            static_assert(is_function_like<Func>::value);
            static_assert(std::is_same<FA, permutated<first_variable_t<Func>>>::value);

            using FB = permutated<partial_applied_t<Func>>;

            return ma.has_value() ? FB(function_traits<Func>::partial_apply(f, ma.value())) : FB(nothing);
        }

        template <typename B>
        constexpr static auto replace = left<B, FA, permutated<B>>;
    };

    template <typename A>
    struct Applicative<maybe<A>> : public pertaining_type_class
    {
        using FA = maybe<A>;

        static_assert(Functor<FA>::pertain);

        template <typename B>
        using permutated = maybe<B>;

        using parameter = A;

        constexpr static auto pure = id<parameter, FA>;

        template <typename FFunc>
        constexpr static permutated<partial_applied_t<typename Applicative<FFunc>::parameter>> ap(FFunc mf, FA ma)
        {
            static_assert(is_of_same_type_class_instance<Applicative, FFunc, FA>::value);
            static_assert(Applicative<FFunc>::pertain);
            using Func = typename Applicative<FFunc>::parameter;
            static_assert(is_function_like<Func>::value);
            using FB = permutated<partial_applied_t<typename Applicative<FFunc>::parameter>>;

            return mf.has_value() ? ma.has_value() ? function_traits<Func>::partial_apply(mf.value(), ma.value()) : FB(nothing) : FB(nothing);
        }
        template <typename FB>
        constexpr static auto right_tie = right<FA, FB>;

        template <typename FB>
        constexpr static auto left_tie = left<FA, FB>;
    };

    // A monoid on applicative functors.
    template <typename A>
    struct Alternative<maybe<A>> : public pertaining_type_class
    {

        static_assert(Applicative<maybe<A>>::pertain);

        constexpr static const maybe<A> empty = nothing;

        constexpr static maybe<A> alter(maybe<A> lhs, const maybe<A> &rhs)
        {
            return lhs.has_value() ? lhs : rhs;
        }
    };

    template <typename A>
    struct Monad<maybe<A>> : public pertaining_type_class
    {
        using MA = maybe<A>;

        static_assert(Applicative<MA>::pertain);

        template <typename B>
        using permutated = maybe<B>;

        using parameter = A;

        constexpr static auto pure = Applicative<MA>::pure;

        template <typename Func>
        constexpr static partial_applied_t<Func> bind(MA ma, Func f)
        {
            static_assert(is_monadically_bindable<Func, MA>::value);
            return ma.has_value() ? function_traits<Func>::partial_apply(f, ma.value()) : partial_applied_t<Func>(nothing);
        }

        template <typename MB>
        constexpr static auto compose = Applicative<MA>::template right_tie<MB>;
    };

    // When a value is bound in do-notation, the pattern on the left hand side of <- might not match. In this case, this class provides a function to recover.
    // template <typename A>
    // struct MonadFail<maybe<A>> : public pertaining_type_class
    // {
    //     static_assert(Monad<maybe<A>>::pertain);
    //     static maybe<A> fail(const ) { return nothing; };
    // };

    template <typename L, typename R>
    using either = std::variant<L, R>;

    template <typename Func1, typename Func2>
    partial_applied_t<Func1> either_apply(Func1 f1, Func2 f2, either<first_variable_t<Func1>, first_variable_t<Func2>> eab)
    {
        static_assert(is_function_like<Func1>::value && is_function_like<Func2>::value);
        static_assert(std::is_same<partial_applied_t<Func1>, partial_applied_t<Func2>>::value);
        return eab.index() == 0 ? function_traits<Func1>::partial_apply(f1, std::get<0>(eab)) : function_traits<Func2>::partial_apply(f2, std::get<1>(eab));
    }

    template <typename L, typename R, typename = enable_type_class_t<Show<L>, Show<R>>>
    std::ostream &operator<<(std::ostream &os, const either<L, R> &e)
    {
        return e.index() == 0 ? os << "left " << std::get<0>(e) : os << "right " << std::get<1>(e);
    }

    template <typename L, typename R>
    struct Semigroup<either<L, R>> : public pertaining_type_class
    {
        // An associative operation.
        constexpr static either<L, R> op(either<L, R> lhs, const either<L, R> &rhs)
        {
            return lhs.index() == 0 ? rhs : lhs;
        }
    };

    // A type f is a Functor if it provides a function fmap which, given any types a and b lets you apply any function from (a -> b) to turn an f a into an f b, preserving the structure of f.
    template <typename L, typename R>
    struct Functor<either<L, R>> : public pertaining_type_class
    {

        using FA = either<L, R>;

        template <typename B>
        using permutated = either<L, B>;

        using parameter = R;

        template <typename Func>
        constexpr static permutated<partial_applied_t<Func>> fmap(Func f, FA ma)
        {
            static_assert(is_function_like<Func>::value);
            static_assert(std::is_same<FA, permutated<first_variable_t<Func>>>::value);

            return ma.index() == 0 ? permutated<partial_applied_t<Func>>(std::in_place_index<0>, std::get<0>(ma)) : permutated<partial_applied_t<Func>>(std::in_place_index<1>, function_traits<Func>::partial_apply(f, std::get<1>(ma)));
        }

        template <typename B>
        constexpr static auto replace = left<B, FA, permutated<B>>;
    };

    template <typename L, typename R>
    struct Applicative<either<L, R>> : public pertaining_type_class
    {
        using FA = either<L, R>;

        static_assert(Functor<FA>::pertain);

        template <typename B>
        using permutated = either<L, B>;

        using parameter = R;

        constexpr static FA pure(parameter a) { return FA(std::in_place_index<1>, a); }

        template <typename FFunc>
        constexpr static permutated<partial_applied_t<typename Applicative<FFunc>::parameter>> ap(FFunc mf, FA ma)
        {
            static_assert(is_of_same_type_class_instance<Applicative, FFunc, FA>::value);
            static_assert(Applicative<FFunc>::pertain);
            using Func = typename Applicative<FFunc>::parameter;
            static_assert(is_function_like<Func>::value);
            using FB = permutated<partial_applied_t<typename Applicative<FFunc>::parameter>>;

            return mf.index() == 1 ? ma.index() == 1 ? FB(std::in_place_index<1>, function_traits<Func>::partial_apply(std::get<1>(mf), std::get<1>(ma)))
                                                     : FB(std::in_place_index<0>, std::get<0>(ma))
                                   : FB(std::in_place_index<0>, std::get<0>(mf));
        }
        template <typename FB>
        constexpr static auto right_tie = right<FA, FB>;

        template <typename FB>
        constexpr static auto left_tie = left<FA, FB>;
    };

    template <typename L, typename R>
    struct Monad<either<L, R>> : public pertaining_type_class
    {
        using MA = either<L, R>;

        static_assert(Applicative<MA>::pertain);

        template <typename B>
        using permutated = either<L, B>;

        using parameter = L;

        using E = L;

        constexpr static auto pure = Applicative<MA>::pure;

        template <typename Func>
        constexpr static partial_applied_t<Func> bind(MA ma, Func f)
        {
            static_assert(is_monadically_bindable<Func, MA>::value);
            return ma.index() == 0 ? partial_applied_t<Func>(return_type_t<Func>(std::in_place_index<0>, std::get<0>(ma))) : function_traits<Func>::partial_apply(f, std::get<1>(ma));
        }

        template <typename MB>
        constexpr static auto compose = right<MA, MB>;
    };

}

#endif