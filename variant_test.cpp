#include <iostream>
#include <utility>
#include <string>

#include "variant.hpp"

struct visitor {
	std::ostream & out;
	void operator () (int val) const {
		out << "int: " << val << '\n';
	}
	void operator () (double val) const {
		out << "double: " << val << '\n';
	}
	template <typename Other>
	void operator () (Other) const {
		out << "other\n";
	}
};
	

int main () {
	auto v = ns::variant<int, double, std::string, void *> (3.5);
	const auto v2 = std::move (v);
	auto v3 = v2;
	v = v3;

	//outputs: "double: 3.5"
	ns::visit (visitor { std::cout }, v);

	v = std::string { "text" };
	//outputs: "text"
	ns::visit ([] (auto && arg) { std::cout << arg << '\n';}, v);
	//outputs: "other"
	ns::visit (visitor { std::cout }, v);
	
}
