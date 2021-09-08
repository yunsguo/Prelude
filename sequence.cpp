#include <iostream>

#include "sequence.h"
#include "base.h"

using namespace fcl;

namespace fcl
{
    template <typename A, typename B>
    struct Show<std::variant<A, B>, enable_type_class_t<Show<A>, Show<B>>> : public pertaining_type_class
    {
        constexpr static string show(const std::variant<A, B> &v)
        {
            return v.index() == 0 ? Show<A>::show(std::get<0>(v)) : Show<B>::show(std::get<1>(v));
        }
    };

    template <typename A, typename B, typename C>
    struct Show<std::variant<A, B, C>, enable_type_class_t<Show<A>, Show<B>, Show<C>>> : public pertaining_type_class
    {
        constexpr static string show(const std::variant<A, B, C> &v)
        {
            return v.index() == 0 ? Show<A>::show(std::get<0>(v)) : v.index() == 1 ? Show<B>::show(std::get<1>(v))
                                                                                   : Show<C>::show(std::get<2>(v));
        }
    };

    template <typename A, typename B, typename C, typename D>
    struct Show<std::variant<A, B, C, D>, enable_type_class_t<Show<A>, Show<B>, Show<C>, Show<D>>> : public pertaining_type_class
    {
        constexpr static string show(const std::variant<A, B, C, D> &v)
        {
            return v.index() == 0 ? Show<A>::show(std::get<0>(v)) : v.index() == 1 ? Show<B>::show(std::get<1>(v))
                                                                : v.index() == 2   ? Show<C>::show(std::get<2>(v))
                                                                                   : Show<D>::show(std::get<3>(v));
        }
    };

    template <typename A, size_t N>
    struct Show<CTA<A, N>, enable_type_class_t<Show<A>>> : public pertaining_type_class
    {
        constexpr static string show(const CTA<A, N> &c)
        {
            return '(' + Show<A>::show(std::get<0>(c)) + ", " + Show<CTA<A, N - 1>>::show(c.tail).substr(1);
        }
    };

    template <typename A>
    struct Show<CTA<A, 1>, enable_type_class_t<Show<A>>> : public pertaining_type_class
    {
        constexpr static string show(const CTA<A, 1> &c)
        {
            return '(' + Show<A>::show(std::get<0>(c)) + ')';
        }
    };

    template <typename A>
    struct Show<node<A>, enable_type_class_t<Show<A>>> : public pertaining_type_class
    {

        constexpr static int get_depth(const node<A> &n)
        {
            return n.a.index() == 1 ? 1 : n.a.index() == 3 ? 1
                                      : n.a.index() == 2   ? 1 + get_depth(*std::get<0>(std::get<2>(n.a)))
                                                           : 1 + get_depth(*std::get<0>(std::get<0>(n.a)));
        }

        template <size_t N>
        constexpr static std::string show(const std::shared_ptr<CTA<A, N>> &p)
        {
            return p ? Show<CTA<A, N>>::show(*p) : std::string("nullptr");
        }

        constexpr static std::string show(const CTA<std::shared_ptr<node<A>>, 1> &c)
        {
            return std::get<0>(c) ? show(*std::get<0>(c)) : std::string("nullptr");
        }

        template <size_t N>
        constexpr static std::string show(const CTA<std::shared_ptr<node<A>>, N> &c)
        {
            return (std::get<0>(c) ? show(*std::get<0>(c)) : std::string("nullptr")) + ", " + show(c.tail);
        }

        constexpr static std::string show(const std::variant<CTA<std::shared_ptr<node<A>>, 2>, std::shared_ptr<CTA<A, 2>>, CTA<std::shared_ptr<node<A>>, 3>, std::shared_ptr<CTA<A, 3>>> &v)
        {
            return v.index() >= 2 ? v.index() == 2 ? show(std::get<2>(v)) : show(std::get<3>(v)) : v.index() == 0 ? show(std::get<0>(v))
                                                                                                                  : show(std::get<1>(v));
        }

        constexpr static string show(const node<A> &n)
        {
            return "node" + (get_depth(n) == 0 ? std::to_string(n.a.index() + 2) + " " + show(n.a)
                                               : "^" + std::to_string(get_depth(n)) + " " + show(n.a));
        }
    };

    template <>
    struct Show<empty> : public pertaining_type_class
    {
        static string show(const empty &)
        {
            return "empty";
        }
    };

    template <typename A>
    struct Show<single<A>, enable_type_class_t<Show<A>>> : public pertaining_type_class
    {
        constexpr static string show(const single<A> &s)
        {
            return "single " + Show<A>::show(s.a);
        }
    };

    template <typename A>
    struct Show<deep<A>, enable_type_class_t<Show<A>>> : public pertaining_type_class
    {
        constexpr static string show(const deep<A> &d)
        {
            return "deep (digit " + Show<decltype(d.a)>::show(d.a) + ", " + (d.b ? Show<decltype(*d.b)>::show(*d.b) : Show<empty>::show(empty_v)) + ", digit " + Show<decltype(d.c)>::show(d.c) + ")";
        }
    };

    template <typename A>
    struct Show<finger_tree<A>, enable_type_class_t<Show<A>>> : public pertaining_type_class
    {
        constexpr static string show(const finger_tree<A> &f)
        {
            return Show<decltype(f.a)>::show(f.a);
        }
    };
}

finger_tree<int> get_large_left(int n, int i = 1, finger_tree<int> identity = empty_v)
{
    if (n <= i)
        return identity;
    auto k = i += identity;
    return get_large_left(n, i + 1, k);
}

finger_tree<int> get_large_right(int n, int i = 1, finger_tree<int> identity = empty_v)
{
    if (n <= i)
        return identity;
    auto k = identity << i;
    return get_large_right(n, i + 1, k);
}

int main()
{
    std::cout << get_large_left(100) << std::endl;

    std::cout << get_large_right(100) << std::endl;
}