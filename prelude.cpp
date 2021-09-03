/*
* prelude.cpp
* test stub for prelude.h
* no actual implenmetaion included
* Yunsheng Guo yguo125@syr.edu
*/

#include <iostream>
#include <string>
#include <cassert>

#include "prelude.h"

using namespace fcl;

struct test_struct
{
    test_struct() {}
    test_struct(size_t i) : index(i) {}

    size_t value() const { return index; }

    operator size_t() const { return index; }

private:
    size_t index;
};

template <>
struct fcl::Eq<test_struct> : public pertaining_type_class
{
    static bool equals(const test_struct &one, const test_struct &other)
    {
        return fcl::Eq<size_t>::equals(one, other);
    }
};

template <>
struct fcl::Ord<test_struct> : public pertaining_type_class
{
    static Ordering compare(const test_struct &one, const test_struct &other)
    {
        return fcl::Ord<size_t>::compare(one, other);
    }
};

template <>
struct fcl::Monoid<test_struct> : public pertaining_type_class
{
    static const test_struct mempty;
    static test_struct mappend(test_struct lhs, const test_struct &rhs) { return lhs.value() + rhs.value(); }
};

// bool k = __has_to_string_implementation<test_struct>::value;

const test_struct fcl::Monoid<test_struct>::mempty = 0;

int increment(int a)
{
    return a + 1;
}

int decrement(int a) { return a - 1; }

int op1(int a, bool b) { return b ? a + 1 : a - 1; }

bool is_nothing(maybe<int> a)
{
    return !a.has_value();
}

maybe<int> maybe_add(maybe<int> a, maybe<int> b)
{
    if (is_nothing(a) || is_nothing(b))
        return nothing;
    return a.value() + b.value();
}

maybe<int> monadic_add(int a, int b)
{
    auto r = a + b;
    return r < 0 ? maybe<int>(nothing) : maybe<int>(r);
}

unsigned int k = 0;

int main()
{
    std::cout
        << "start operation. " << std::endl;
    assert(increment - maybe<int>(1) == maybe<int>(2));
    assert(increment - maybe<int>(nothing) == maybe<int>(nothing));
    assert(increment - 5 % maybe<int>(1) == maybe<int>(6));
    auto minc = maybe<std::decay_t<decltype(increment)>>(increment);
    assert(minc * maybe<int>(1) == maybe<int>(2));
    auto mop1 = maybe<std::decay_t<decltype(op1)>>(op1);
    decltype(mop1 * maybe<int>(1) << maybe<int>(3)) k;
    assert(mop1 * (maybe<int>(1) << maybe<int>(3)) * maybe<bool>(true) == maybe<int>(4));
    assert(mop1 * (maybe<int>(1) >> maybe<int>(3)) * maybe<bool>(true) == maybe<int>(2));
    assert((mop1 * maybe<int>(1) << maybe<int>(3)) == maybe<int>(3));
    assert((mop1 * maybe<int>(1) >> maybe<int>(3)) * maybe<bool>(true) == maybe<int>(2));

    assert(mop1 * maybe<int>(1) * maybe<bool>(false) >> maybe<bool>(true) == maybe<int>(0));
    assert(mop1 * maybe<int>(1) * maybe<bool>(false) << maybe<bool>(true) == maybe<bool>(true));
    assert(mop1 * maybe<int>(1) * maybe<bool>(false) >> maybe<bool>(true) == maybe<bool>(false));
    assert(mop1 * maybe<int>(1) * (maybe<int>(3) << maybe<bool>(true)) == maybe<int>(2));
    assert(maybe_apply(-1, increment, maybe<int>(2)) == 3);
    assert(maybe_apply(-1, increment, nothing) == -1);
    test_struct ts1(1);
    test_struct ts2(2);
    std::cout << "test_struct is Eq: " << Eq<test_struct>::pertain << std::endl;
    std::cout << "ts1 == ts2: " << (ts1 == ts2) << std::endl;
    std::cout << "ts1 >= ts2: " << (ts1 >= ts2) << std::endl;
    std::cout << "ts1 < ts2: " << (ts1 < ts2) << std::endl;
    std::cout << "5 to_string: " << std::to_string(5) << std::endl;
    std::cout << "blank: " << std::endl;
    std::cout << "ts1 to_string: " << std::to_string(ts1) << std::endl;
    std::cout << "show ts1: " << ts1 << std::endl;
    std::cout << "semigroup op: " << (ts1 + ts2 + ts1 + ts2) << std::endl;
    std::cout << "semigroup op: " << (1 == 2) << std::endl;
    std::cout << "5 by show: " << Show<int>::show(5) << std::endl;

    auto f = increment;
    maybe<int> m(5);
    maybe<int> m1;
    maybe<int> m3(7);
    maybe<decltype(f)> mf = f;

    maybe<__native_value_type_t<decltype(increment)>> m2 = increment;
    static_assert(fcl::Show<int>::pertain);
    static_assert(fcl::Show<maybe<int>>::pertain);
    std::cout << "maybe Int: " << maybe<int>(3) << std::endl;
    std::cout << "nothing < maybe 5: " << (m1 < m) << std::endl;
    std::cout << "5 by show: " << Show<int>::show(5) << std::endl;
    std::cout << "nothing | maybe 5: " << (maybe<int>() | maybe<int>(5)) << std::endl;
    std::cout << "monadic_add <= maybe 5 <= maybe 6 << nothing: " << (monadic_add <= maybe<int>(5) <= maybe<int>(6) << maybe<int>(nothing)) << std::endl;
    std::cout << "monadic_add <= maybe -5 <= maybe -6: " << (monadic_add <= maybe<int>(-5) <= maybe<int>(-6)) << std::endl;

    std::cout << "monadic_add <= nothing <= maybe 6 : " << (monadic_add <= maybe<int>(nothing) <= maybe<int>(6)) << std::endl;
    std::cout
        << "end operation. " << std::endl;
}
