#include "test.hpp"

#include "writer_string.hpp"

#include <math.h>
#include <float.h>

bool test_string_equal(const pugi::char_t* lhs, const pugi::char_t* rhs)
{
	return (!lhs || !rhs) ? lhs == rhs : pugi::impl::strequal(lhs, rhs);
}

bool test_node(const pugi::xml_node& node, const pugi::char_t* contents, const pugi::char_t* indent, unsigned int flags)
{
	xml_writer_string writer;
	node.print(writer, indent, flags);

	return writer.as_string() == contents;
}

#ifndef PUGIXML_NO_XPATH
bool test_xpath_string(const pugi::xml_node& node, const pugi::char_t* query, const pugi::char_t* expected)
{
	pugi::xpath_query q(query);

	return q.evaluate_string(node) == expected;
}

bool test_xpath_boolean(const pugi::xml_node& node, const pugi::char_t* query, bool expected)
{
	pugi::xpath_query q(query);

	return q.evaluate_boolean(node) == expected;
}

bool test_xpath_number(const pugi::xml_node& node, const pugi::char_t* query, double expected)
{
	pugi::xpath_query q(query);

	return fabs(q.evaluate_number(node) - expected) < 1e-16f;
}

bool test_xpath_number_nan(const pugi::xml_node& node, const pugi::char_t* query)
{
	pugi::xpath_query q(query);

	double r = q.evaluate_number(node);

#if defined(_MSC_VER) || defined(__BORLANDC__)
	return _isnan(r) != 0;
#else
	return r != r;
#endif
}

bool test_xpath_fail_compile(const pugi::char_t* query)
{
	try
	{
		pugi::xpath_query q(query);
		return false;
	}
	catch (const pugi::xpath_exception&)
	{
		return true;
	}
}

void xpath_node_set_tester::check(bool condition)
{
	if (!condition)
	{
		test_runner::_failure_message = message;
		longjmp(test_runner::_failure_buffer, 1);
	}
}

xpath_node_set_tester::xpath_node_set_tester(const pugi::xml_node& node, const pugi::char_t* query, const char* message): last(0), message(message)
{
	pugi::xpath_query q(query);
	result = q.evaluate_node_set(node);
}

xpath_node_set_tester::xpath_node_set_tester(const pugi::xpath_node_set& set, const char* message): last(0), message(message)
{
	result = set;
}

xpath_node_set_tester::~xpath_node_set_tester()
{
	// check that we processed everything
	check(last == result.size());
}

xpath_node_set_tester& xpath_node_set_tester::operator%(unsigned int expected)
{
	// check element count
	check(last < result.size());

	// check document order
	pugi::xpath_node node = result.begin()[last];
	unsigned int order = node.attribute() ? node.attribute().document_order() : node.node().document_order();

	check(order == expected);

	// continue to the next element
	last++;

	return *this;
}

bool is_little_endian()
{
	unsigned int ui = 1;
	return *reinterpret_cast<char*>(&ui) == 1;
}

#endif
