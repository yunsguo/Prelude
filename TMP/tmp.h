/*
*   tmp.h:
*   A template meta_traits programming package in Haskell style for general use
*   Language: C++, Visual Studio 2017
*   Platform: Windows 10 Pro
*   Application: recreational
*   Author: Yunsheng Guo, yguo125@syr.edu
*/

/*
*
*   Package Operations:
*	A lot of type pack operations
*	converting freely between std::tuple and tmp::pack and tmp::Cons
*	has A list_trait struct for automatic conversion from any specilized type to tmp::Cons
*	naming convensions follows Haskell, capitals are types and all lower cases are meta_traits functions
*
*   Public Interface:
*	technically none.
*
*   Build Process:
*
*   Maintenance History:
*   June 6
*   it all starts from here
*	August 25
*	final review for publish, has not changed much due to the bed rock position in dependency hierarchy
*
*
*/

#ifndef TMP_H
#define TMP_H

#include <type_traits>

namespace tmp
{

    struct undefined
    {
    };

    template <bool predicate, typename T>
    struct else_undefined;

    template <typename T>
    struct else_undefined<true, T>
    {
        using type = T;
    };

    template <typename T>
    struct else_undefined<false, T>
    {
        using type = undefined;
    };

    template <bool predicate, typename T>
    using else_undefined_t = typename else_undefined<predicate, T>::type;

    //A sequencial container for heterogeneous type
    template <typename... As>
    struct pack
    {
    };

    struct possess_traits
    {
        //possess trait or not
        constexpr static const bool possess = true;
    };

    struct not_possess_traits
    {
        //possess trait or not
        constexpr static const bool possess = false;
    };

    template <typename A, typename Enable = void>
    struct pack_traits : public not_possess_traits
    {
        using type = undefined;
    };

    template <typename... As>
    struct pack_traits<pack<As...>> : public possess_traits
    {
        using type = pack<As...>;
    };

    template <typename A>
    using is_pack_like = std::bool_constant<pack_traits<A>::possess>;

    template <typename A>
    using pack_traits_t = typename pack_traits<A>::type;

    template <typename A>
    struct inferred_pack_by_traits
    {
        using type = std::conditional_t<is_pack_like<A>::value, pack_traits_t<A>, A>;
    };

    template <typename A>
    using inferred_pack_by_traits_t = typename inferred_pack_by_traits<A>::type;

    template <template <typename...> typename TypeClass, typename A>
    struct inferred_from
    {
        using type = else_undefined_t<is_pack_like<A>::value, typename inferred_from<TypeClass, pack_traits_t<A>>::type>;
    };

    template <template <typename...> typename TypeClass, typename A>
    using inferred_from_t = typename inferred_from<TypeClass, A>::type;

    template <typename... As>
    struct inferred_from<pack, pack<As...>>
    {
        using type = pack<As...>;
    };

    //meta_traits function for constructing A pack
    template <typename A, typename B>
    struct cons
    {
        using type = else_undefined_t<is_pack_like<B>::value, typename cons<A, pack_traits_t<B>>::type>;
    };

    template <typename A, typename B>
    using cons_t = typename cons<A, B>::type;

    template <typename A, typename... As>
    struct cons<A, pack<As...>>
    {
        using type = pack<A, As...>;
    };

    template <template <typename...> typename AB, typename FA>
    struct fmap;

    template <template <typename...> typename AB, typename FA>
    using fmap_t = typename fmap<AB, FA>::type;

    template <template <typename...> typename AB>
    struct fmap<AB, pack<>>
    {
        using type = pack<>;
    };

    template <template <typename...> typename AB, typename A, typename... As>
    struct fmap<AB, pack<A, As...>>
    {
        using type = cons_t<typename AB<A>::type, fmap_t<AB, pack<As...>>>;
    };

    template <typename...>
    struct conjunct;

    template <typename... As, typename... Bs>
    struct conjunct<pack<As...>, Bs...> : public conjunct<As..., Bs...>
    {
    };

    template <>
    struct conjunct<> : public std::true_type
    {
    };

    template <typename A, typename... As>
    struct conjunct<A, As...> : public std::integral_constant<bool, A::value && conjunct<As...>::value>
    {
    };

    template <typename...>
    struct disjunct;

    template <typename... As, typename... Bs>
    struct disjunct<pack<As...>, Bs...> : public disjunct<As..., Bs...>
    {
    };

    template <>
    struct disjunct<> : public std::false_type
    {
    };

    template <typename A, typename... As>
    struct disjunct<A, As...> : public std::integral_constant<bool, A::value || disjunct<As...>::value>
    {
    };

    template <template <typename> typename P, typename L>
    struct all : public else_undefined_t<is_pack_like<L>::value, all<P, pack_traits_t<L>>>
    {
    };

    template <template <typename> typename P, typename A, typename... As>
    struct all<P, pack<A, As...>> : public conjunct<P<A>, all<P, pack<As...>>>
    {
    };

    template <template <typename> typename P>
    struct all<P, pack<>> : public std::true_type
    {
    };

    //meta_traits function concatenating two pack
    template <typename... As>
    struct concat
    {
        using type = else_undefined_t<all<is_pack_like, pack<As...>>::value, typename concat<pack_traits_t<As>...>::type>;
    };

    template <typename... As>
    using concat_t = typename concat<As...>::type;

    template <typename... As, typename... Bs>
    struct concat<pack<As...>, pack<Bs...>>
    {
        using type = pack<As..., Bs...>;
    };

    //Extract the first element of A pack, which must be non-empty.
    template <typename A>
    struct head
    {
        using type = else_undefined_t<is_pack_like<A>::value, typename head<pack_traits_t<A>>::type>;
    };

    template <typename A>
    using head_t = typename head<A>::type;

    template <typename A, typename... As>
    struct head<pack<A, As...>>
    {
        using type = A;
    };

    template <>
    struct head<undefined>
    {
        using type = undefined;
    };

    //Extract the elements after the head of A pack, which must be non-empty.
    template <typename A>
    struct tail
    {
        using type = else_undefined_t<is_pack_like<A>::value, typename tail<pack_traits_t<A>>::type>;
    };

    template <typename A>
    using tail_t = typename tail<A>::type;

    template <typename A, typename... As>
    struct tail<pack<A, As...>>
    {
        using type = pack<As...>;
    };

    template <>
    struct tail<undefined>
    {
        using type = undefined;
    };

    //Return all the elements of A pack except the last one. The pack must be non-empty.
    template <typename A>
    struct init
    {
        using type = else_undefined_t<is_pack_like<A>::value, typename init<pack_traits_t<A>>::type>;
    };

    template <typename A>
    using init_t = typename init<A>::type;

    template <typename A>
    struct init<pack<A>>
    {
        using type = pack<>;
    };

    template <typename A, typename B>
    struct init<pack<A, B>>
    {
        using type = pack<A>;
    };

    template <typename A, typename B, typename... Rest>
    struct init<pack<A, B, Rest...>>
    {
        using type = cons_t<A, typename init<pack<B, Rest...>>::type>;
    };

    //Extract the last element of A pack, which must be finite and non-empty.
    template <typename A>
    struct last
    {
        using type = else_undefined_t<is_pack_like<A>::value, typename last<pack_traits_t<A>>::type>;
    };

    template <typename A>
    using last_t = typename last<A>::type;

    template <typename A>
    struct last<pack<A>>
    {
        using type = A;
    };

    template <typename A, typename B, typename... Rest>
    struct last<pack<A, B, Rest...>>
    {
        using type = typename last<pack<B, Rest...>>::type;
    };

    template <>
    struct last<undefined>
    {
        using type = undefined;
    };

    using size_t = std::size_t;

    // returns the prefix of xs of length n, or xs itself if n > length xs
    template <size_t I, typename A>
    struct take
    {
        using type = else_undefined_t<is_pack_like<A>::value, typename take<I, pack_traits_t<A>>::type>;
    };

    template <size_t I, typename A>
    using take_t = typename take<I, A>::type;

    template <size_t I>
    struct take<I, pack<>>
    {
        using type = pack<>;
    };

    template <typename A, typename... As>
    struct take<0, pack<A, As...>>
    {
        using type = pack<>;
    };

    template <size_t I, typename A, typename... As>
    struct take<I, pack<A, As...>>
    {
        using type = cons_t<A, take_t<I - 1, pack<As...>>>;
    };

    //returns the suffix of xs after the first n elements, or [] if n > length xs
    template <size_t I, typename A>
    struct drop
    {
        using type = else_undefined_t<is_pack_like<A>::value, typename drop<I, pack_traits_t<A>>::type>;
    };

    template <size_t I, typename A>
    using drop_t = typename drop<I, A>::type;

    template <size_t I>
    struct drop<I, pack<>>
    {
        using type = pack<>;
    };

    template <typename A, typename... As>
    struct drop<0, pack<A, As...>>
    {
        using type = pack<A, As...>;
    };

    template <size_t I, typename A, typename... As>
    struct drop<I, pack<A, As...>>
    {
        using type = drop_t<I - 1, pack<As...>>;
    };

    //Returns the size/length of A finite structure As an Int.
    template <typename A>
    struct length : public std::conditional_t<
                        is_pack_like<A>::value,
                        std::integral_constant<size_t, length<pack_traits_t<A>>::value>,
                        std::integral_constant<size_t, -1>>
    {
    };

    template <typename... As>
    struct length<pack<As...>> : std::integral_constant<size_t, sizeof...(As)>
    {
    };

    template <typename A, typename List>
    struct elem : public else_undefined_t<is_pack_like<List>::value, elem<A, pack_traits_t<List>>>
    {
        static_assert(is_pack_like<List>::value);
    };

    template <typename A, typename... As>
    struct elem<A, pack<A, As...>> : public std::true_type
    {
    };

    template <typename A>
    struct elem<A, pack<>> : public std::false_type
    {
    };

    template <typename A, typename B, typename... Bs>
    struct elem<A, pack<B, Bs...>> : public elem<A, pack<Bs...>>
    {
    };

    //pack indexing meta_traits function
    template <size_t I, typename A>
    struct index
    {
        using type = typename index<I - 1, tail_t<A>>::type;
    };

    template <size_t I, typename A>
    using index_t = typename index<I, A>::type;

    template <typename A>
    struct index<0, A>
    {
        using type = head_t<A>;
    };

    template <typename A, typename List>
    struct elem_index : public else_undefined_t<is_pack_like<List>::value, elem_index<A, pack_traits_t<List>>>
    {
        static_assert(is_pack_like<List>::value);
    };

    template <typename A, typename... As>
    struct elem_index<A, pack<A, As...>> : public std::integral_constant<size_t, 0>
    {
    };

    template <typename A>
    struct elem_index<A, pack<>> : public std::integral_constant<size_t, -1>
    {
    };

    template <typename A, typename B, typename... Bs>
    struct elem_index<A, pack<B, Bs...>> : public std::integral_constant<size_t, elem_index<A, pack<Bs...>>::value == -1 ? -1 : 1 + elem_index<A, pack<Bs...>>::value>
    {
    };

    namespace
    {
        using namespace tmp;

        template <typename L1, typename L2>
        struct rev
        {
            using type = typename rev<tail_t<L1>, cons_t<head_t<L1>, L2>>::type;
        };

        template <typename A, typename L2>
        struct rev<pack<A>, L2>
        {
            using type = cons_t<A, L2>;
        };

        template <typename A>
        struct rev<pack<>, A>
        {
            using type = A;
        };
    }

    //reverse A returns the elements of A in reverse order.
    template <typename A>
    struct reverse
    {
        using type = typename rev<A, pack<>>::type;
    };

    template <typename A>
    using reverse_t = typename reverse<A>::type;

    template <template <typename...> typename F, typename L>
    struct uncurry : public else_undefined_t<is_pack_like<L>::value, typename uncurry<F, pack_traits_t<L>>::type>
    {
        using type = typename uncurry<F, pack_traits_t<L>>::type;
    };

    template <template <typename...> typename F, typename L>
    using uncurry_t = typename uncurry<F, L>::type;

    template <template <typename...> typename F, typename... As>
    struct uncurry<F, pack<As...>> : public F<As...>
    {
    };

    template <typename L1, typename L2>
    struct zip
    {
        using type = else_undefined_t<is_pack_like<L1>::value && is_pack_like<L2>::value, typename zip<pack_traits_t<L1>, pack_traits_t<L2>>::type>;
    };

    template <typename L1, typename L2>
    using zip_t = typename zip<L1, L2>::type;

    template <typename A, typename... As, typename B, typename... Bs>
    struct zip<pack<A, As...>, pack<B, Bs...>>
    {
        using type = cons_t<pack<A, B>, zip_t<pack<As...>, pack<Bs...>>>;
    };

    template <typename A>
    struct zip<pack<>, A>
    {
        using type = pack<>;
    };

    template <typename A>
    struct zip<A, pack<>>
    {
        using type = pack<>;
    };

    template <>
    struct zip<pack<>, pack<>>
    {
        using type = pack<>;
    };

    template <typename P>
    struct flip
    {
        using type = else_undefined_t<is_pack_like<P>::value, typename flip<pack_traits_t<P>>::type>;
    };

    template <typename P>
    using flip_t = typename flip<P>::type;

    template <typename A, typename B, typename... Cs>
    struct flip<pack<A, B, Cs...>>
    {
        using type = pack<B, A, Cs...>;
    };

    template <typename A>
    struct flip<pack<A>>
    {
        using type = undefined;
    };

    template <>
    struct flip<pack<>>
    {
        using type = undefined;
    };

}

namespace
{

    using namespace tmp;

    using lst1 = pack<int, int, int, int, int>;
    using lst2 = pack<double, double, double, double, double>;
    using lst3 = pack<int, double, float, char>;
    using lst4 = pack<size_t>;
    using lst5 = pack<int, char>;
    using lst6 = pack<int, size_t, char, double, float>;

    template <typename... As>
    struct Dummy;
    template <typename... As>
    struct Dummy2;
}

namespace tmp
{

    template <typename... As>
    struct pack_traits<Dummy<As...>> : public possess_traits
    {
        using type = fmap_t<inferred_pack_by_traits, pack<As...>>;
    };
}

namespace
{
    using namespace tmp;

    static_assert(is_pack_like<lst1>::value);

    static_assert(is_pack_like<Dummy<int>>::value);

    static_assert(is_pack_like<Dummy<int>>::value);

    static_assert(std::is_same<pack_traits_t<Dummy<Dummy<Dummy<int, int>, int>, int>>, pack<pack<pack<int, int>, int>, int>>::value);

    static_assert(std::is_same<pack_traits_t<Dummy2<int, int>>, undefined>::value);

    static_assert(std::is_same<inferred_from_t<pack, lst4>, lst4>::value);

    static_assert(std::is_same<cons_t<int, pack<double, float, char>>, lst3>::value);

    static_assert(std::is_same<fmap_t<head, pack<lst1, lst2, lst3, lst4, lst5, lst6>>, pack<int, double, int, size_t, int, int>>::value);

    static_assert(!conjunct<pack<std::true_type, std::true_type, std::false_type>>::value);

    static_assert(conjunct<std::true_type, std::true_type, std::true_type>::value);

    static_assert(disjunct<pack<std::true_type, std::true_type, std::false_type>>::value);

    static_assert(!disjunct<std::false_type, std::false_type, std::false_type>::value);

    template <typename A>
    using is_int = std::is_same<A, int>;

    static_assert(all<is_int, pack<int, int, int, int>>::value);

    static_assert(std::is_same<concat_t<pack<int, int, int>, pack<int, int>>, lst1>::value);

    static_assert(std::is_same<head_t<lst5>, int>::value);

    static_assert(std::is_same<last_t<lst5>, char>::value);

    static_assert(std::is_same<tail_t<lst1>, pack<int, int, int, int>>::value);

    static_assert(std::is_same<init_t<lst3>, pack<int, double, float>>::value);

    static_assert(std::is_same<take_t<2, lst3>, pack<int, double>>::value);

    static_assert(std::is_same<drop_t<2, lst3>, pack<float, char>>::value);

    static_assert(length<lst1>::value == 5);

    static_assert(std::is_same<typename index<2, lst6>::type, char>::value);

    static_assert(elem<int, lst1>::value);

    static_assert(!elem<float, lst1>::value);

    static_assert(tmp::elem_index<float, lst1>::value == -1);

    static_assert(tmp::elem_index<int, lst1>::value == 0);

    static_assert(std::is_same<typename reverse<lst6>::type, pack<float, double, char, size_t, int>>::value);

    static_assert(std::is_same<uncurry_t<cons, pack<int, pack<double, float, char>>>, lst3>::value);

    static_assert(std::is_same<zip_t<lst3, lst6>, pack<pack<int, int>, pack<double, size_t>, pack<float, char>, pack<char, double>>>::value);

    static_assert(uncurry<std::is_convertible, pack<int, double>>::value);

    template <typename A>
    using uncurried_is_convertible = uncurry<std::is_convertible, A>;

    static_assert(uncurried_is_convertible<pack<int, double>>::value);

    static_assert(std::is_same<fmap_t<uncurried_is_convertible, zip_t<lst1, lst2>>, pack<std::true_type, std::true_type, std::true_type, std::true_type, std::true_type>>::value);

    static_assert(conjunct<fmap_t<uncurried_is_convertible, zip_t<lst1, lst2>>>::value);

    static_assert(std::is_same<pack<int, double, float>, flip_t<pack<double, int, float>>>::value);
}
#endif
