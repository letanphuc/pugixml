#ifndef PUGIXML_NO_XPATH

#include "common.hpp"

TEST(xpath_literal_parse)
{
	xml_node c;
	CHECK_XPATH_STRING(c, T("'a\"b'"), T("a\"b"));
	CHECK_XPATH_STRING(c, T("\"a'b\""), T("a'b"));
	CHECK_XPATH_STRING(c, T("\"\""), T(""));
	CHECK_XPATH_STRING(c, T("\'\'"), T(""));
}

TEST(xpath_literal_error)
{
	CHECK_XPATH_FAIL(T("\""));
	CHECK_XPATH_FAIL(T("\'"));
}

TEST(xpath_number_parse)
{
	xml_node c;
	CHECK_XPATH_NUMBER(c, T("0"), 0);
	CHECK_XPATH_NUMBER(c, T("123"), 123);
	CHECK_XPATH_NUMBER(c, T("123.456"), 123.456);
	CHECK_XPATH_NUMBER(c, T(".123"), 0.123);
	CHECK_XPATH_NUMBER(c, T("123.4567890123456789012345"), 123.4567890123456789012345);
}

TEST(xpath_number_error)
{
	CHECK_XPATH_FAIL(T("123a"));
	CHECK_XPATH_FAIL(T("123.a"));
	CHECK_XPATH_FAIL(T(".123a"));
}

TEST(xpath_variables)
{
	CHECK_XPATH_FAIL(T("$var")); // not implemented
	CHECK_XPATH_FAIL(T("$1"));
}

TEST(xpath_empty_expression)
{
	CHECK_XPATH_FAIL(T(""));
}

TEST(xpath_lexer_error)
{
	CHECK_XPATH_FAIL(T("!"));
	CHECK_XPATH_FAIL(T("&"));
}

TEST(xpath_unmatched_braces)
{
	CHECK_XPATH_FAIL(T("node["));
	CHECK_XPATH_FAIL(T("node[1"));
	CHECK_XPATH_FAIL(T("node[]]"));
	CHECK_XPATH_FAIL(T("node("));
	CHECK_XPATH_FAIL(T("node(()"));
	CHECK_XPATH_FAIL(T("(node)[1"));
	CHECK_XPATH_FAIL(T("(1"));
}

TEST(xpath_incorrect_step)
{
	CHECK_XPATH_FAIL(T("child::1"));
	CHECK_XPATH_FAIL(T("something::*"));
	CHECK_XPATH_FAIL(T("a::*"));
	CHECK_XPATH_FAIL(T("c::*"));
	CHECK_XPATH_FAIL(T("d::*"));
	CHECK_XPATH_FAIL(T("f::*"));
	CHECK_XPATH_FAIL(T("n::*"));
	CHECK_XPATH_FAIL(T("p::*"));
}

TEST(xpath_semantics_error)
{
	CHECK_XPATH_FAIL(T("1[1]"));
	CHECK_XPATH_FAIL(T("1 | 1"));
}

TEST(xpath_semantics_posinv) // coverage for contains()
{
	xpath_query(T("(node)[substring(1, 2, 3)]"));
	xpath_query(T("(node)[concat(1, 2, 3, 4)]"));
	xpath_query(T("(node)[count(foo)]"));
	xpath_query(T("(node)[local-name()]"));
	xpath_query(T("(node)[(node)[1]]"));
}

#endif
