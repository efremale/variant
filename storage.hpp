#pragma once

#include <type_traits>
#include <memory>

#include "utility.hpp"

namespace ns::impl {

template <typename ... Ts>
struct generate_destroy_table {

	template <typename T>
	constexpr static void destroy (std::aligned_union_t <0, Ts...> & data) {
		reinterpret_cast<T *> (std::addressof (data))->~T ();
	}

	using f_ptr = decltype (std::addressof (destroy<impl::head_t<Ts...>>));

	constexpr static f_ptr table[] = {
		std::addressof (destroy<Ts>)...
	};
};


template <bool is_trivially_destructible, typename ... Ts>
struct storage {
	std::aligned_union_t <0, Ts...> data;
	std::size_t index;
};

template <typename ... Ts>
struct storage<false, Ts...> {
	std::aligned_union_t <0, Ts...> data;
	std::size_t index;
	~storage () {
		(generate_destroy_table<Ts...>::table [index] (data));
	}
};

}
