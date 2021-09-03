/*
*   tuple.h:
*   A container template for sharing partially applied parameters
*   Language: C++, Visual Studio 2017
*   Platform: Windows 10 Pro
*   Application: recreational
*   Author: Yunsheng Guo, yguo125@syr.edu
*/

/*
*
*   Package Operations:
*	A heterogenerous container that shares the object it contains during copying
*
*   Public Interface:
*   Build Process:
*
*   Maintenance History:
*   August 6
*   first draft
*	August 10
*	removed from project
*	August 30
*	add back to the tuple branch
*
*
*/

#ifndef TUPLE_H
#define TUPLE_H

#include <type_traits>
#include <memory>
#ifndef USE_SHAREDTUPLE
#include <tuple>
#endif

#include "tmp.h"

#ifdef USE_SHAREDTUPLE
#include "shared_tuple.h"
#endif

namespace fcl
{
#ifdef USE_SHAREDTUPLE
	template <typename... As>
	using tuple = detail::shared_tuple<As...>;
#else
	template <typename... Elements>
	using tuple = std::tuple<Elements...>;

	template <typename... Elements>
	constexpr auto make_tuple = std::make_tuple<Elements...>;

	template <typename... Elements>
	constexpr auto forward_as_tuple = std::forward_as_tuple<Elements...>;
}
namespace tmp
{

	template <typename... As>
	struct inferred_from<fcl::tuple, pack<As...>>
	{
		using type = fcl::tuple<As...>;
	};

	template <typename... As>
	struct pack_traits<fcl::tuple<As...>> : public possess_traits
	{
		using type = pack<As...>;
	};
}

namespace fcl
{
	namespace
	{
		template <std::size_t... Ns, typename... Ts>
		inferred_from_t<tuple, take_t<sizeof...(Ns), std::tuple<Ts...>>> take_impl(std::index_sequence<Ns...>, std::tuple<Ts...> t)
		{
			return std::forward_as_tuple(std::forward<index_t<Ns, pack<Ts...>>>(std::get<Ns>(t))...);
		}

		template <std::size_t N, std::size_t... Ns, typename... Ts>
		inferred_from_t<tuple, drop_t<N, std::tuple<Ts...>>> drop_impl(std::index_sequence<Ns...>, std::tuple<Ts...> t)
		{
			return std::forward_as_tuple(std::forward<index_t<Ns + N, pack<Ts...>>>(std::get<Ns + N>(t))...);
		}
	}

	template <size_t N, typename... As>
	tuple<inferred_from_t<tuple, take_t<N, pack<As...>>>, inferred_from_t<tuple, drop_t<N, pack<As...>>>> split_at(tuple<As...> tuple)
	{
		static_assert(N <= sizeof...(As));
		return std::forward_as_tuple(take_impl(std::make_index_sequence<N>{}, tuple), drop_impl<N>(std::make_index_sequence<sizeof...(As) - N>{}, tuple));
	}

}

#endif
#endif