/*
*   shared_tuple.h:
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
*	add back to the shared_tuple branch
*
*
*/

#ifndef SHARED_TUPLE_H
#define SHARED_TUPLE_H

#include <type_traits>
#include <memory>
#include <utility>

#include "tmp.h"

namespace detail
{

	template <typename A>
	struct is_trivial : public std::bool_constant<
							std::is_trivially_constructible<A>::value &&
							std::is_trivially_copy_constructible<A>::value &&
							std::is_trivially_destructible<A>::value &&
							std::is_trivially_copy_assignable<A>::value>
	{
	};

	template <typename A>
	struct trivial_wrapper
	{
		using is_trivial = is_trivial<A>;
		using wrapper_type = std::conditional_t<is_trivial::value, A, std::shared_ptr<A>>;

		trivial_wrapper() = default;

		template <typename R = void>
		typename std::enable_if<is_trivial::value, R>::type
		initialize(A &&value)
		{
			val = std::forward<A>(value);
		}

		template <typename R = void>
		typename std::enable_if<!is_trivial::value, R>::type
		initialize(A &&value)
		{
			val = std::make_shared<A>(std::forward<A>(value));
		}

		trivial_wrapper(A &&value) { initialize(std::forward<A>(value)); }

		trivial_wrapper(const trivial_wrapper &other) = default;

		trivial_wrapper(trivial_wrapper &&other) = default;

		friend void swap(trivial_wrapper &one, trivial_wrapper &other)
		{
			using std::swap;

			swap(one.val, other.val);
		}

		trivial_wrapper &operator=(trivial_wrapper other)
		{
			swap(*this, other);

			return *this;
		}

		template <typename R = A>
		constexpr typename std::enable_if<is_trivial::value, R>::type const &
		value() const { return val; }

		template <typename R = A>
		constexpr typename std::enable_if<is_trivial::value, R>::type &
		value() { return val; }

		template <typename R = A>
		typename std::enable_if<!is_trivial::value, R>::type const &value() const { return *val; }

		template <typename R = A>
		typename std::enable_if<!is_trivial::value, R>::type &value() { return *val; }

	private:
		wrapper_type val;
	};

	template <typename... As>
	struct shared_tuple;

	template <typename A>
	struct is_shared_tuple : public std::false_type
	{
	};

	template <typename... As>
	struct is_shared_tuple<shared_tuple<As...>> : public std::true_type
	{
	};
}

namespace fcl
{
	template <typename... As>
	struct type_list_traits<detail::shared_tuple<As...>> : public type_list_traits<type_list<As...>>
	{
	};

	template <typename... As>
	struct inferred_from<detail::shared_tuple, type_list<As...>>
	{
		using type = detail::shared_tuple<As...>;
	};
}

namespace detail
{
	template <typename L>
	using inferred_t = fcl::inferred_from_t<shared_tuple, L>;

	template <size_t I, typename A>
	struct get_by_index;

	template <typename A>
	struct construct_traits;

	template <typename A>
	struct tuple_traits
	{
		using possess = std::false_type;
	};

	template <>
	struct shared_tuple<>
	{
	};

	template <typename A>
	struct shared_tuple<A>
	{

		shared_tuple() = default;
		shared_tuple(A &&head) : _head(std::forward<A>(head)), _tail() {}

		shared_tuple(shared_tuple<> &&init, trivial_wrapper<A> &&last) : _head(std::forward<trivial_wrapper<A>>(last)), _tail() {}

		shared_tuple(const shared_tuple &other) = default;
		shared_tuple(shared_tuple &&other) = default;

		friend void swap(shared_tuple &one, shared_tuple &other) noexcept
		{
			using std::swap;

			swap(one._head, other._head);
		}
		shared_tuple &operator=(shared_tuple other)
		{
			swap(*this, other);
			return *this;
		}

		template <size_t, typename>
		friend struct get_by_index;

		friend struct construct_traits<shared_tuple>;

	private:
		trivial_wrapper<A> _head;
		shared_tuple<> _tail;
	};

	template <typename A1, typename A2, typename... As>
	struct shared_tuple<A1, A2, As...>
	{

		shared_tuple() = default;
		shared_tuple(A1 &&a1, A2 &&a2, As &&...as) : _head(std::forward<A1>(a1)), _tail(std::forward<A2>(a2), std::forward<As>(as)...) {}

		shared_tuple(inferred_t<init_t<shared_tuple>> &&init, trivial_wrapper<last_t<shared_tuple>> &&last)
			: _head(init._head),
			  _tail(shared_tuple<A2, As...>(init._tail), std::forward<trivial_wrapper<last_t<shared_tuple>>>(last))) {}

		shared_tuple(const shared_tuple &other) = default;
		shared_tuple(shared_tuple &&other) = default;

		friend void swap(shared_tuple &one, shared_tuple &other) noexcept
		{
			using std::swap;

			swap(one._head, other._head);
			swap(one._tail, other._tail);
		}
		shared_tuple &operator=(shared_tuple other)
		{
			swap(*this, other);
			return *this;
		}

		template <typename... Bs>
		friend struct shared_tuple;

		template <size_t, typename>
		friend struct get_by_index;

		friend struct construct_traits<shared_tuple>;

	private:
		trivial_wrapper<A1> _head;
		shared_tuple<A2, As...> _tail;
	};

	template <typename A, typename... As>
	struct get_by_index<0, shared_tuple<A, As...>>
	{
		using tuple = shared_tuple<A, As...>;
		static constexpr A &invoke(tuple &t)
		{
			return t._head.value();
		}
		static constexpr const A &invoke(const tuple &t)
		{
			return t._head.value();
		}
		static constexpr A &&invoke(tuple &&t)
		{
			return std::forward<R &&>(t._head.value());
		}
		static constexpr const A &&invoke(const tuple &&t)
		{
			return std::forward<const R &&>(t._head.value());
		}
	};

	template <size_t I, typename A, typename... As>
	struct get_by_index<I, shared_tuple<A, As...>>
	{
		using tuple = shared_tuple<A, As...>;
		using R = index_t<I, tuple>;

		static constexpr R &invoke(tuple &t)
		{
			return get_by_index<I - 1, shared_tuple<As...>>::invoke(t._tail);
		}
		static constexpr const R &invoke(const tuple &t)
		{
			return get_by_index<I - 1, shared_tuple<As...>>::invoke(t._tail);
		}
		static constexpr R &&invoke(tuple &&t)
		{
			return std::forward<R &&>(get_by_index<I - 1, shared_tuple<As...>>::invoke(t._tail));
		}
		static constexpr const R &&invoke(const tuple &&t)
		{
			return std::forward<const R &&>(get_by_index<I - 1, shared_tuple<As...>>::invoke(t._tail));
		}
	};

	template <typename A, typename... As>
	struct construct_traits<shared_tuple<A, As...>>
	{
		using head = trivial_wrapper<A>;
		using tail = shared_tuple<As...>;

		static constexpr std::pair<head, tail> match(shared_tuple<A, As...> &&tuple)
		{
			return std::pair<head, tail>(std::move(tuple._head), std::move(tuple._tail));
		}
	};

	template <typename A, typename... As>
	constexpr shared_tuple<A, As...> make_tuple(A &&a, As &&...as) noexcept
	{
		return shared_tuple<A, As...>(std::forward<A>(a), std::forward<As>(as)...);
	}

	constexpr shared_tuple<> make_tuple() noexcept
	{
		return shared_tuple<>();
	}

	template <typename... As>
	constexpr shared_tuple<As...> tuple_cat(shared_tuple<As...> &&tuple) noexcept { return tuple; }

	template <typename... As>
	constexpr shared_tuple<As...> tuple_cat(shared_tuple<As...> &&tuple, shared_tuple<> &&) noexcept { return tuple; }

	template <typename A, typename... As>
	constexpr shared_tuple<A, As...> tuple_cat(shared_tuple<> &&, shared_tuple<A, As...> &&tuple) noexcept { return tuple; }

	template <typename A, typename... As, typename B, typename... Bs>
	constexpr shared_tuple<A, As..., B, Bs...> tuple_cat(shared_tuple<A, As...> &&t1, shared_tuple<B, Bs...> &&t2) noexcept
	{
		auto pair = construct_traits<shared_tuple<B, Bs...>>::match(std::forward<shared_tuple<B, Bs...>>(t2));
		return tuple_cat(shared_tuple<A, As..., B>(std::forward<shared_tuple<A, As...>>(t1), std::move(pair.first)), shared_tuple<Bs...>(std::move(pair.second)));
	}

	template <typename T1, typename T2, typename T3, typename... Ts>
	constexpr std::enable_if_t<all<is_shared_tuple, type_list<T1, T2, T3, Ts...>>::value, inferred_t<concat_t<T1, T2, T3, Ts...>>> tuple_cat(T1 &&t1, T2 &&t2, T3 &&t3, Ts &&...ts) noexcept
	{
		return tuple_cat(tuple_cat(std::forward<T1>(t1), std::forward<T2>(t2)), tuple_cat(std::forward<T3>(t3), std::forward<Ts>(ts)...));
	}
}

namespace fcl
{

	template <typename... As>
	constexpr detail::shared_tuple<As...> make_tuple(As &&...as) noexcept
	{
		return detail::make_tuple(std::forward<As>(as)...);
	}
}

namespace std
{

	template <typename... Ts>
	struct tuple_size<detail::shared_tuple<Ts...>>
		: std::integral_constant<std::size_t, sizeof...(Ts)>
	{
	};

	template <std::size_t I, typename... As>
	struct tuple_element<I, detail::shared_tuple<As...>>
	{
		using type = fcl::index_t<I, detail::shared_tuple<As...>>;
	};

	template <std::size_t I, typename... Ts>
	constexpr typename std::tuple_element<I, detail::shared_tuple<Ts...>>::type &get(detail::shared_tuple<Ts...> &t) noexcept
	{
		return detail::get_by_index<I, detail::shared_tuple<Ts...>>::invoke(t);
	}

	template <std::size_t I, typename... Ts>
	constexpr typename std::tuple_element<I, detail::shared_tuple<Ts...>>::type const &get(const detail::shared_tuple<Ts...> &t) noexcept
	{
		return detail::get_by_index<I, detail::shared_tuple<Ts...>>::invoke(t);
	}

	template <std::size_t I, typename... Ts>
	constexpr typename std::tuple_element<I, detail::shared_tuple<Ts...>>::type &&get(detail::shared_tuple<Ts...> &&t) noexcept
	{
		using R = typename std::tuple_element<I, detail::shared_tuple<Ts...>>::type;
		return std::forward<R &&>(detail::get_by_index<I, detail::shared_tuple<Ts...>>::invoke(t));
	}

	template <std::size_t I, typename... Ts>
	constexpr typename std::tuple_element<I, detail::shared_tuple<Ts...>>::type const &&get(const detail::shared_tuple<Ts...> &&t) noexcept
	{
		using R = typename std::tuple_element<I, detail::shared_tuple<Ts...>>::type;
		return std::forward<const R &&>(detail::get_by_index<I, detail::shared_tuple<Ts...>>::invoke(t));
	}

	template <typename... Ts>
	constexpr std::enable_if_t<all<detail::is_shared_tuple, type_list<Ts...>>::value, detail::inferred_t<concat_t<Ts...>>> tuple_cat(Ts &&...ts) noexcept
	{
		return detail::tuple_cat(std::forward<Ts>(ts)...);
	}

}

#endif