#pragma once

#include <type_traits>
#include <memory>
#include <exception>

#include "storage.hpp"
#include "utility.hpp"

namespace ns {

template <typename ... Ts>
class variant;

namespace impl {

template <typename F, ref_type RT, typename ... Ts>
struct generate_visit_table {
	
	using variant_ref =
		std::conditional_t<
			RT == impl::ref_type::lvalue, variant<Ts...> &,
			std::conditional_t<
				RT == impl::ref_type::const_lvalue, const variant<Ts...> &,
				std::conditional_t<
					RT == impl::ref_type::rvalue, variant<Ts...> &&,
					void>>>;

	using return_type =
		std::common_type_t<std::invoke_result_t<F, Ts>...>;

	template <typename T> 
	constexpr static return_type apply (F && f, variant_ref variant) {
		return f (std::forward<variant_ref> (variant).template get<T> ());
	}

	using f_ptr_type = 
		std::common_type_t<decltype (std::addressof (apply<Ts>))...>;

	constexpr static f_ptr_type table [] = {
		std::addressof (apply<Ts>)...
	};
};

}



template <typename ... Ts>
class variant {
	static_assert (sizeof... (Ts) > 0, "Union must hold at least one type");

private:
	constexpr static bool is_trivially_destructible =
		std::conjunction_v<std::is_trivially_destructible<Ts>...>;

	impl::storage<is_trivially_destructible, Ts...> data;
public:
	~variant () = default;
	variant () = delete;

	template <
		typename Arg,
		typename = std::enable_if_t<
			std::disjunction_v<
				std::is_same<
					Ts,
				 	std::decay_t<Arg>>...>>>

	constexpr variant (Arg && arg) {
		using T = std::decay_t<Arg>;
		::new (&data.data) T (std::forward<Arg> (arg));
		data.index = impl::get_idx<T, Ts...>;
	}

	constexpr variant (variant<Ts...> && arg) noexcept {
		visit (
			[ptr = &this->data.data] (auto && val) {
				::new (ptr) std::decay_t<decltype (val)> (std::move (val));
			}, std::move (arg));	
		data.index = arg.index ();
	}

	constexpr variant (const variant<Ts...> & arg) {
		visit (
			[ptr = &this->data.data] (auto && val) {
				::new (ptr) std::decay_t<decltype (val)> (val);
			}, arg);	
		data.index = arg.index ();
	}

	template <
		typename Arg,
		typename = std::enable_if_t<
			std::disjunction_v<
				std::is_same<
					Ts,
				 	std::decay_t<Arg>>...>>>	
	constexpr variant<Ts...> & operator = (Arg && arg) {
		using T = std::decay_t<Arg>;
		::new (&data.data) T (std::forward<Arg> (arg));
		data.index = impl::get_idx<T, Ts...>;
		return (*this);
	}

	constexpr variant<Ts...> & operator = (variant<Ts...> && arg) noexcept {
		visit (
			[ptr = &this->data.data] (auto && val) {
				::new (ptr) std::decay_t<decltype (val)> (std::move (val));
			}, std::move (arg));
		data.index = arg.index ();
		return (*this);
	}

	constexpr variant<Ts...> & operator = (const variant<Ts...> & arg) {
		data.~storage ();
		visit (
			[ptr = &this->data.data] (auto && val) {
				::new (ptr) std::decay_t<decltype (val)> (val);
			}, arg);
		data.index = arg.index ();
		return (*this);
	}

	constexpr std::size_t index () const noexcept {
		return data.index;
	}

	template <typename T>
	constexpr T & get () & {
		static_assert (std::disjunction_v<std::is_same<T, Ts>...>, "Can't get that type");
		if (this->index () != impl::get_idx<T, Ts...>)
			throw std::logic_error ("Bad variant access");
		return (*reinterpret_cast<T*> (&data));
	}

	template <typename T>
	constexpr T && get () && {
		static_assert (std::disjunction_v<std::is_same<T, Ts>...>, "Can't get that type");
		if (this->index () != impl::get_idx<T, Ts...>)
			throw std::logic_error ("Bad variant access");
		return std::move ((*reinterpret_cast<T*> (&data)));
	}

	template <typename T>
	constexpr const T & get () const {
		static_assert (std::disjunction_v<std::is_same<T, Ts>...>, "Can't get that type");
		if (this->index () != impl::get_idx<T, Ts...>)
			throw std::logic_error ("Bad variant access");
		return (*reinterpret_cast<const T*> (&data));
	}
};

template <typename T, typename ... Ts>
constexpr bool holds_alternative (const variant<Ts...> & var) {
	return (var.index () == impl::get_idx<T, Ts...>);
}

template <typename T, typename ... Ts>
constexpr T & get (variant<Ts...> & var) {
	return var.template get<T> ();
}

template <typename T, typename ... Ts>
constexpr T && get (variant<Ts...> && var) {
	return std::move (var).template get<T> ();
}

template <typename T, typename ... Ts>
constexpr T & get (const variant<Ts...> & var) {
	return var.template get<T> ();
}

template <typename F, typename ... Ts>
constexpr decltype (auto) visit (F && f, variant<Ts...> & var) {
	return impl::generate_visit_table<F, impl::ref_type::lvalue, Ts...>::table[var.index ()] (
		std::forward<F> (f), var);
}

template <typename F, typename ... Ts>
constexpr decltype (auto) visit (F && f, const variant<Ts...> & var) {
	return impl::generate_visit_table<F, impl::ref_type::const_lvalue, Ts...>::table[var.index ()] (
		std::forward<F> (f), var);
}

template <typename F, typename ... Ts>
constexpr decltype (auto) visit (F && f, variant<Ts...> && var) {
	return impl::generate_visit_table<F, impl::ref_type::rvalue, Ts...>::table[var.index ()] (
		std::forward<F> (f), std::move (var));
}

template <typename F>
constexpr auto visit (F && f) {
	return
		[&f] (auto && var) {
			return visit (std::forward<F> (f), std::forward<decltype(var)> (var));
		};
}

}
