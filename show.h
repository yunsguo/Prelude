#ifndef SHOW_H
#define SHOW_H

#include <iostream>
#include <string>
#include "base.h"

namespace fcl
{

    template <typename T, typename Enable = void>
    struct __has_builtin_insersion_operator : public std::false_type
    {
    };

    template <typename T>
    struct __has_builtin_insersion_operator<T,
                                            decltype(____eval<std::ostream &>(std::declval<std::ostream &>() << std::declval<T>()))> : public std::true_type
    {
    };

    template <typename A>
    struct Show
    {
        // pertaining to a type class or not
        constexpr static const bool pertain = __has_builtin_insersion_operator<A>::value;
    };
}
#endif