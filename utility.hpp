#pragma once

#include <type_traits>

namespace ns::impl {

template <typename Head, typename ... Tail>
struct head_type {
	using type = Head;
};

template <typename ... Ts>
using head_t = typename head_type<Ts...>::type;

template <std::size_t I, typename T, typename Head, typename ... Tail>
constexpr std::size_t get_idx_impl = get_idx_impl<I + 1, T, Tail...>;

template <std::size_t I, typename T, typename ... Tail>
constexpr std::size_t get_idx_impl<I, T, T, Tail...> = I;

template <typename T, typename ... Ts>
constexpr std::size_t get_idx = get_idx_impl<0, T, Ts...>;

enum class ref_type { lvalue, const_lvalue, rvalue, not_ref };

template <typename T>
constexpr ref_type get_ref_type =
	(std::is_lvalue_reference_v<T> ?
		(std::is_const_v<T> ? ref_type::const_lvalue : ref_type::lvalue) :
		((std::is_rvalue_reference_v<T> && !std::is_const_v<T>) ? ref_type::rvalue : ref_type::not_ref));

}
