#include <iostream>

#include "sequence.h"
#include "base.h"
#include "show.h"

using namespace fcl;

namespace fcl
{
    template <size_t I, typename... Ts>
    struct __insert
    {
        static std::ostream &apply(std::ostream &os, const std::variant<Ts...> &v)
        {
            return v.index() == I ? os << std::get<I>(v) : __insert<I - 1, Ts...>::apply(os, v);
        }
    };

    template <typename... Ts>
    struct __insert<1, Ts...>
    {
        static std::ostream &apply(std::ostream &os, const std::variant<Ts...> &v)
        {
            return v.index() == 1 ? os << std::get<1>(v) : os << std::get<0>(v);
        }
    };

    template <typename T, typename... Ts, typename = enable_all_type_class_t<Show, T, Ts...>>
    std::ostream &operator<<(std::ostream &os, const std::variant<T, Ts...> &v)
    {
        return __insert<sizeof...(Ts), T, Ts...>::apply(os, v);
    }

    template <typename T, typename = enable_type_class_t<Show<T>>>
    std::ostream &operator<<(std::ostream &os, const std::variant<T> &v)
    {
        return os << std ::get<0>(v);
    }

    template <typename A, size_t N>
    std::ostream &__show(std::ostream &os, const CTA<A, N> &c)
    {
        auto cons = fcl::__cta__impl<N>::view(c);
        os << cons.first << ", " ;
        __show<A>(os,cons.second);
        return os;
    }

    template <typename A>
    std::ostream &__show(std::ostream &os, const CTA<A, 1> &c)
    {
        return os << std::get<0>(c);
    }

    template <typename A, size_t N, typename = enable_type_class_t<Show<A>>>
    std::ostream &operator<<(std::ostream &os, const CTA<A, N> &c)
    {
        os<<'(';
        __show<A>(os,c);
        return os<<')';
    }

    template <typename A,typename = enable_type_class_t<Show<A>>>
    int get_depth(const node<A> &n)
    {
        return n.a.index() == 1 ? 1 : n.a.index() == 3 ? 1
                                  : n.a.index() == 2   ? 1 + get_depth(*std::get<0>(std::get<2>(n.a)))
                                                       : 1 + get_depth(*std::get<0>(std::get<0>(n.a)));
    }

    template <typename A,typename = enable_type_class_t<Show<A>>>
    std::ostream &operator<<(std::ostream &os, const std::shared_ptr<A> &p)
    {
        return p ? os <<"->"<< *p : os << "nullptr";
    }

    template<typename A,typename = enable_type_class_t<Show<A>>>
         std::ostream & operator<<(std::ostream &os, const node<A> &n)
        {
            return get_depth(n) == 0 ? os<<"node" << (n.a.index() + 2) <<" " << n.a
                                               : os<<"^" <<get_depth(n)<<  " " << n.a;
        }

        std::ostream & operator<<(std::ostream &os, const empty &)
        {
            return os<<"empty";
        }

    template<typename A,typename = enable_type_class_t<Show<A>>>
        std::ostream& operator<<(std::ostream& os,const single<A> &s)
        {
            return os<<"single "<<s.a;
        }

    template<typename A,typename = enable_type_class_t<Show<A>>>
        std::ostream& operator<<(std::ostream& os,const deep<A> &d)
        {
             return os<<"deep (digit " << d.a << ", "
             << (d.b? *d.b:empty_v)
             <<", digit "<<d.c<<")";
        }
        

    template <typename A,typename = enable_type_class_t<Show<A>>>
    std::ostream& operator<<(std::ostream& os, const finger_tree<A>&f)
    {
        return os<<f.a;
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