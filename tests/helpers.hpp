#pragma once

#include "common.hpp"

#include <utility>

template <typename U> static void generic_bool_ops_test(const U& obj)
{
	U null;

	CHECK(!null);
	CHECK(obj);
	CHECK(!!obj);

	bool b1 = null, b2 = obj;

	CHECK(!b1);
	CHECK(b2);

	CHECK(obj && b2);
	CHECK(obj || b2);
	CHECK(obj && obj);
	CHECK(obj || obj);
}

template <typename U> static void generic_eq_ops_test(const U& obj1, const U& obj2)
{
	U null = U();

	// operator==
	CHECK(null == null);
	CHECK(obj1 == obj1);
	CHECK(!(null == obj1));
	CHECK(!(null == obj2));
	CHECK(U(null) == null);
	CHECK(U(obj1) == obj1);

	// operator!=
	CHECK(!(null != null));
	CHECK(!(obj1 != obj1));
	CHECK(null != obj1);
	CHECK(null != obj2);
	CHECK(!(U(null) != null));
	CHECK(!(U(obj1) != obj1));
}

template <typename U> static void generic_rel_ops_test(U obj1, U obj2)
{
	U null = U();

	// obj1 < obj2 (we use operator<, but there is no other choice
	if (obj1 > obj2) std::swap(obj1, obj2);

	// operator<
	CHECK(null < obj1);
	CHECK(null < obj2);
	CHECK(obj1 < obj2);
	CHECK(!(null < null));
	CHECK(!(obj1 < obj1));
	CHECK(!(obj1 < null));
	CHECK(!(obj2 < obj1));

	// operator<=
	CHECK(null <= obj1);
	CHECK(null <= obj2);
	CHECK(obj1 <= obj2);
	CHECK(null <= null);
	CHECK(obj1 <= obj1);
	CHECK(!(obj1 <= null));
	CHECK(!(obj2 <= obj1));

	// operator>
	CHECK(obj1 > null);
	CHECK(obj2 > null);
	CHECK(obj2 > obj1);
	CHECK(!(null > null));
	CHECK(!(obj1 > obj1));
	CHECK(!(null > obj1));
	CHECK(!(obj1 > obj2));

	// operator>=
	CHECK(obj1 >= null);
	CHECK(obj2 >= null);
	CHECK(obj2 >= obj1);
	CHECK(null >= null);
	CHECK(obj1 >= obj1);
	CHECK(!(null >= obj1));
	CHECK(!(obj1 >= obj2));
}

