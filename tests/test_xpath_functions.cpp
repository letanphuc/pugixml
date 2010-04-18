#ifndef PUGIXML_NO_XPATH

#include "common.hpp"

TEST_XML(xpath_number_number, "<node>123</node>")
{
	xml_node c;
	xml_node n = doc.child(T("node")).first_child();
	
	// number with 0 arguments
	CHECK_XPATH_NUMBER_NAN(c, T("number()"));
	CHECK_XPATH_NUMBER(n, T("number()"), 123);

	// number with 1 string argument
	CHECK_XPATH_NUMBER(c, T("number(' -123.456 ')"), -123.456);
	CHECK_XPATH_NUMBER(c, T("number(' -123.')"), -123);
	CHECK_XPATH_NUMBER(c, T("number('123.')"), 123);
	CHECK_XPATH_NUMBER(c, T("number('.56')"), 0.56);
	CHECK_XPATH_NUMBER(c, T("number('123 ')"), 123);
	CHECK_XPATH_NUMBER_NAN(c, T("number('foobar')"));
	CHECK_XPATH_NUMBER_NAN(c, T("number('f1')"));
	CHECK_XPATH_NUMBER_NAN(c, T("number('1f')"));
	CHECK_XPATH_NUMBER_NAN(c, T("number('1.f')"));
	CHECK_XPATH_NUMBER_NAN(c, T("number('1.0f')"));
	CHECK_XPATH_NUMBER_NAN(c, T("number('123 f')"));

	// number with 1 bool argument
	CHECK_XPATH_NUMBER(c, T("number(true())"), 1);
	CHECK_XPATH_NUMBER(c, T("number(false())"), 0);

	// number with 1 node set argument
	CHECK_XPATH_NUMBER(n, T("number(.)"), 123);

	// number with 1 number argument
	CHECK_XPATH_NUMBER(c, T("number(1)"), 1);
	
	// number with 2 arguments
	CHECK_XPATH_FAIL(T("number(1, 2)"));
}

TEST_XML(xpath_number_sum, "<node>123<child>789</child></node><node/>")
{
	xml_node c;
	xml_node n = doc.child(T("node"));
	
	// sum with 0 arguments
	CHECK_XPATH_FAIL(T("sum()"));

	// sum with 1 argument
	CHECK_XPATH_NUMBER(c, T("sum(.)"), 0);
	CHECK_XPATH_NUMBER(n, T("sum(.)"), 123789); // 123 .. 789
	
	CHECK_XPATH_NUMBER(n, T("sum(./descendant-or-self::node())"), 125490); // node + 123 + child + 789 = 123789 + 123 + 789 + 789 = 125490
	CHECK_XPATH_NUMBER(n, T("sum(.//node())"), 1701); // 123 + child + 789 = 123 + 789 + 789
	CHECK_XPATH_NUMBER_NAN(doc.last_child(), T("sum(.)"));

	// sum with 2 arguments
	CHECK_XPATH_FAIL(T("sum(1, 2)"));
	
	// sum with 1 non-node-set argument
	CHECK_XPATH_FAIL(T("sum(1)"));
}

TEST(xpath_number_floor)
{
	xml_node c;

	// floor with 0 arguments
	CHECK_XPATH_FAIL(T("floor()"));

	// floor with 1 argument
	CHECK_XPATH_NUMBER(c, T("floor(1.2)"), 1);
	CHECK_XPATH_NUMBER(c, T("floor(1)"), 1);
	CHECK_XPATH_NUMBER(c, T("floor(-1.2)"), -2);
	CHECK_XPATH_NUMBER_NAN(c, T("floor(string('nan'))"));
	CHECK_XPATH_STRING(c, T("string(floor(1 div 0))"), T("Infinity"));
	CHECK_XPATH_STRING(c, T("string(floor(-1 div 0))"), T("-Infinity"));

	// floor with 2 arguments
	CHECK_XPATH_FAIL(T("floor(1, 2)"));
}

TEST(xpath_number_ceiling)
{
	xml_node c;

	// ceiling with 0 arguments
	CHECK_XPATH_FAIL(T("ceiling()"));

	// ceiling with 1 argument
	CHECK_XPATH_NUMBER(c, T("ceiling(1.2)"), 2);
	CHECK_XPATH_NUMBER(c, T("ceiling(1)"), 1);
	CHECK_XPATH_NUMBER(c, T("ceiling(-1.2)"), -1);
	CHECK_XPATH_NUMBER_NAN(c, T("ceiling(string('nan'))"));
	CHECK_XPATH_STRING(c, T("string(ceiling(1 div 0))"), T("Infinity"));
	CHECK_XPATH_STRING(c, T("string(ceiling(-1 div 0))"), T("-Infinity"));

	// ceiling with 2 arguments
	CHECK_XPATH_FAIL(T("ceiling(1, 2)"));
}

TEST(xpath_number_round)
{
	xml_node c;

	// round with 0 arguments
	CHECK_XPATH_FAIL(T("round()"));

	// round with 1 argument
	CHECK_XPATH_NUMBER(c, T("round(1.2)"), 1);
	CHECK_XPATH_NUMBER(c, T("round(1.5)"), 2);
	CHECK_XPATH_NUMBER(c, T("round(1.8)"), 2);
	CHECK_XPATH_NUMBER(c, T("round(1)"), 1);
	CHECK_XPATH_NUMBER(c, T("round(-1.2)"), -1);
	CHECK_XPATH_NUMBER(c, T("round(-1.5)"), -1);
	CHECK_XPATH_NUMBER(c, T("round(-1.6)"), -2);
	CHECK_XPATH_NUMBER_NAN(c, T("round(string('nan'))"));
	CHECK_XPATH_STRING(c, T("string(round(1 div 0))"), T("Infinity"));
	CHECK_XPATH_STRING(c, T("string(round(-1 div 0))"), T("-Infinity"));

	// round with 2 arguments
	CHECK_XPATH_FAIL(T("round(1, 2)"));
}

TEST_XML(xpath_boolean_boolean, "<node />")
{
	xml_node c;
	
	// boolean with 0 arguments
	CHECK_XPATH_FAIL(T("boolean()"));

	// boolean with 1 number argument
	CHECK_XPATH_BOOLEAN(c, T("boolean(0)"), false);
	CHECK_XPATH_BOOLEAN(c, T("boolean(1)"), true);
	CHECK_XPATH_BOOLEAN(c, T("boolean(-1)"), true);
	CHECK_XPATH_BOOLEAN(c, T("boolean(0.1)"), true);
	CHECK_XPATH_BOOLEAN(c, T("boolean(number('nan'))"), false);

	// boolean with 1 string argument
	CHECK_XPATH_BOOLEAN(c, T("boolean('x')"), true);
	CHECK_XPATH_BOOLEAN(c, T("boolean('')"), false);

	// boolean with 1 node set argument
	CHECK_XPATH_BOOLEAN(c, T("boolean(.)"), false);
	CHECK_XPATH_BOOLEAN(doc, T("boolean(.)"), true);
	CHECK_XPATH_BOOLEAN(doc, T("boolean(foo)"), false);

	// boolean with 2 arguments
	CHECK_XPATH_FAIL(T("boolean(1, 2)"));
}

TEST(xpath_boolean_not)
{
	xml_node c;
	
	// not with 0 arguments
	CHECK_XPATH_FAIL(T("not()"));

	// not with 1 argument
	CHECK_XPATH_BOOLEAN(c, T("not(true())"), false);
	CHECK_XPATH_BOOLEAN(c, T("not(false())"), true);
	
	// boolean with 2 arguments
	CHECK_XPATH_FAIL(T("not(1, 2)"));
}

TEST(xpath_boolean_true)
{
	xml_node c;
	
	// true with 0 arguments
	CHECK_XPATH_BOOLEAN(c, T("true()"), true);

	// true with 1 argument
	CHECK_XPATH_FAIL(T("true(1)"));
}

TEST(xpath_boolean_false)
{
	xml_node c;
	
	// false with 0 arguments
	CHECK_XPATH_BOOLEAN(c, T("false()"), false);

	// false with 1 argument
	CHECK_XPATH_FAIL(T("false(1)"));
}

TEST_XML(xpath_boolean_lang, "<node xml:lang='en'><child xml:lang='ru-UK'><subchild/></child></node><foo><bar/></foo>")
{
	xml_node c;
	
	// lang with 0 arguments
	CHECK_XPATH_FAIL(T("lang()"));

	// lang with 1 argument, no language
	CHECK_XPATH_BOOLEAN(c, T("lang('en')"), false);
	CHECK_XPATH_BOOLEAN(doc.child(T("foo")), T("lang('en')"), false);
	CHECK_XPATH_BOOLEAN(doc.child(T("foo")), T("lang('')"), false);
	CHECK_XPATH_BOOLEAN(doc.child(T("foo")).child(T("bar")), T("lang('en')"), false);
	
	// lang with 1 argument, same language/prefix
	CHECK_XPATH_BOOLEAN(doc.child(T("node")), T("lang('en')"), true);
	CHECK_XPATH_BOOLEAN(doc.child(T("node")).child(T("child")), T("lang('ru-uk')"), true);
	CHECK_XPATH_BOOLEAN(doc.child(T("node")).child(T("child")), T("lang('ru')"), true);
	CHECK_XPATH_BOOLEAN(doc.child(T("node")).child(T("child")).child(T("subchild")), T("lang('ru')"), true);
	CHECK_XPATH_BOOLEAN(doc.child(T("node")).child(T("child")).child(T("subchild")), T("lang('RU')"), true);

	// lang with 1 argument, different language/prefix
	CHECK_XPATH_BOOLEAN(doc.child(T("node")), T("lang('')"), false);
	CHECK_XPATH_BOOLEAN(doc.child(T("node")), T("lang('e')"), false);
	CHECK_XPATH_BOOLEAN(doc.child(T("node")).child(T("child")), T("lang('en')"), false);
	CHECK_XPATH_BOOLEAN(doc.child(T("node")).child(T("child")), T("lang('ru-gb')"), false);
	CHECK_XPATH_BOOLEAN(doc.child(T("node")).child(T("child")), T("lang('r')"), false);
	CHECK_XPATH_BOOLEAN(doc.child(T("node")).child(T("child")).child(T("subchild")), T("lang('en')"), false);

	// lang with 2 arguments
	CHECK_XPATH_FAIL(T("lang(1, 2)"));
}

TEST_XML(xpath_string_string, "<node>123<child id='1'>789</child><child><![CDATA[200]]></child>100</node>")
{
	xml_node c;
	xml_node n = doc.child(T("node"));

	// string with 0 arguments
	CHECK_XPATH_STRING(c, T("string()"), T(""));
	CHECK_XPATH_STRING(n.child(T("child")), T("string()"), T("789"));

	// string with 1 node-set argument
	CHECK_XPATH_STRING(n, T("string(child)"), T("789"));
	CHECK_XPATH_STRING(n, T("string(child/@id)"), T("1"));
	CHECK_XPATH_STRING(n, T("string(.)"), T("123789200100"));

	// string with 1 number argument
	CHECK_XPATH_STRING(c, T("string(0 div 0)"), T("NaN"));
	CHECK_XPATH_STRING(c, T("string(0)"), T("0"));
	CHECK_XPATH_STRING(c, T("string(-0)"), T("0"));
	CHECK_XPATH_STRING(c, T("string(1 div 0)"), T("Infinity"));
	CHECK_XPATH_STRING(c, T("string(-1 div 0)"), T("-Infinity"));
	CHECK_XPATH_STRING(c, T("string(1234567)"), T("1234567"));
	CHECK_XPATH_STRING(c, T("string(-1234567)"), T("-1234567"));
	CHECK_XPATH_STRING(c, T("string(1234.5678)"), T("1234.5678"));
	CHECK_XPATH_STRING(c, T("string(-1234.5678)"), T("-1234.5678"));
	CHECK_XPATH_STRING(c, T("string(0.5678)"), T("0.5678"));
	CHECK_XPATH_STRING(c, T("string(-0.5678)"), T("-0.5678"));
	CHECK_XPATH_STRING(c, T("string(0.0)"), T("0"));
	CHECK_XPATH_STRING(c, T("string(-0.0)"), T("0"));

	// string with 1 boolean argument
	CHECK_XPATH_STRING(c, T("string(true())"), T("true"));
	CHECK_XPATH_STRING(c, T("string(false())"), T("false"));

	// string with 1 string argument
	CHECK_XPATH_STRING(c, T("string('abc')"), T("abc"));

	// string with 2 arguments
	CHECK_XPATH_FAIL(T("string(1, 2)"));
}

TEST(xpath_string_concat)
{
	xml_node c;

	// concat with 0 arguments
	CHECK_XPATH_FAIL(T("concat()"));

	// concat with 1 argument
	CHECK_XPATH_FAIL(T("concat('')"));

	// concat with exactly 2 arguments
	CHECK_XPATH_STRING(c, T("concat('prev','next')"), T("prevnext"));
	CHECK_XPATH_STRING(c, T("concat('','next')"), T("next"));
	CHECK_XPATH_STRING(c, T("concat('prev','')"), T("prev"));

	// concat with 3 or more arguments
	CHECK_XPATH_STRING(c, T("concat('a', 'b', 'c')"), T("abc"));
	CHECK_XPATH_STRING(c, T("concat('a', 'b', 'c', 'd')"), T("abcd"));
	CHECK_XPATH_STRING(c, T("concat('a', 'b', 'c', 'd', 'e')"), T("abcde"));
	CHECK_XPATH_STRING(c, T("concat('a', 'b', 'c', 'd', 'e', 'f')"), T("abcdef"));
	CHECK_XPATH_STRING(c, T("concat('a', 'b', 'c', 'd', 'e', 'f', 'g')"), T("abcdefg"));
}

TEST(xpath_string_starts_with)
{
	xml_node c;

	// starts-with with 0 arguments
	CHECK_XPATH_FAIL(T("starts-with()"));

	// starts-with with 1 argument
	CHECK_XPATH_FAIL(T("starts-with('a')"));

	// starts-with with 2 arguments
	CHECK_XPATH_BOOLEAN(c, T("starts-with('abc', '')"), true);
	CHECK_XPATH_BOOLEAN(c, T("starts-with('abc', 'a')"), true);
	CHECK_XPATH_BOOLEAN(c, T("starts-with('abc', 'abc')"), true);
	CHECK_XPATH_BOOLEAN(c, T("starts-with('abc', 'abcd')"), false);
	CHECK_XPATH_BOOLEAN(c, T("starts-with('bc', 'c')"), false);
	CHECK_XPATH_BOOLEAN(c, T("starts-with('', 'c')"), false);
	CHECK_XPATH_BOOLEAN(c, T("starts-with('', '')"), true);

	// starts-with with 3 arguments
	CHECK_XPATH_FAIL(T("starts-with('a', 'b', 'c')"));
}

TEST(xpath_string_contains)
{
	xml_node c;

	// contains with 0 arguments
	CHECK_XPATH_FAIL(T("contains()"));

	// contains with 1 argument
	CHECK_XPATH_FAIL(T("contains('a')"));

	// contains with 2 arguments
	CHECK_XPATH_BOOLEAN(c, T("contains('abc', '')"), true);
	CHECK_XPATH_BOOLEAN(c, T("contains('abc', 'a')"), true);
	CHECK_XPATH_BOOLEAN(c, T("contains('abc', 'abc')"), true);
	CHECK_XPATH_BOOLEAN(c, T("contains('abcd', 'bc')"), true);
	CHECK_XPATH_BOOLEAN(c, T("contains('abc', 'abcd')"), false);
	CHECK_XPATH_BOOLEAN(c, T("contains('b', 'bc')"), false);
	CHECK_XPATH_BOOLEAN(c, T("contains('', 'c')"), false);
	CHECK_XPATH_BOOLEAN(c, T("contains('', '')"), true);

	// contains with 3 arguments
	CHECK_XPATH_FAIL(T("contains('a', 'b', 'c')"));
}

TEST(xpath_string_substring_before)
{
	xml_node c;

	// substring-before with 0 arguments
	CHECK_XPATH_FAIL(T("substring-before()"));

	// substring-before with 1 argument
	CHECK_XPATH_FAIL(T("substring-before('a')"));
	
	// substring-before with 2 arguments
	CHECK_XPATH_STRING(c, T("substring-before('abc', 'abc')"), T(""));
	CHECK_XPATH_STRING(c, T("substring-before('abc', 'a')"), T(""));
	CHECK_XPATH_STRING(c, T("substring-before('abc', 'cd')"), T(""));
	CHECK_XPATH_STRING(c, T("substring-before('abc', 'b')"), T("a"));
	CHECK_XPATH_STRING(c, T("substring-before('abc', 'c')"), T("ab"));
	CHECK_XPATH_STRING(c, T("substring-before('', '')"), T(""));
	
	// substring-before with 2 arguments, from W3C standard
	CHECK_XPATH_STRING(c, T("substring-before(\"1999/04/01\",\"/\")"), T("1999"));

	// substring-before with 3 arguments
	CHECK_XPATH_FAIL(T("substring-before('a', 'b', 'c')"));
}

TEST(xpath_string_substring_after)
{
	xml_node c;

	// substring-after with 0 arguments
	CHECK_XPATH_FAIL(T("substring-after()"));

	// substring-after with 1 argument
	CHECK_XPATH_FAIL(T("substring-after('a')"));
	
	// substring-after with 2 arguments
	CHECK_XPATH_STRING(c, T("substring-after('abc', 'abc')"), T(""));
	CHECK_XPATH_STRING(c, T("substring-after('abc', 'a')"), T("bc"));
	CHECK_XPATH_STRING(c, T("substring-after('abc', 'cd')"), T(""));
	CHECK_XPATH_STRING(c, T("substring-after('abc', 'b')"), T("c"));
	CHECK_XPATH_STRING(c, T("substring-after('abc', 'c')"), T(""));
	CHECK_XPATH_STRING(c, T("substring-after('', '')"), T(""));

	// substring-before with 2 arguments, from W3C standard
	CHECK_XPATH_STRING(c, T("substring-after(\"1999/04/01\",\"/\")"), T("04/01"));
	CHECK_XPATH_STRING(c, T("substring-after(\"1999/04/01\",\"19\")"), T("99/04/01"));

	// substring-after with 3 arguments
	CHECK_XPATH_FAIL(T("substring-after('a', 'b', 'c')"));
}

TEST(xpath_string_substring)
{
	xml_node c;

	// substring with 0 arguments
	CHECK_XPATH_FAIL(T("substring()"));
	
	// substring with 1 argument
	CHECK_XPATH_FAIL(T("substring('')"));
	
	// substring with 2 arguments
	CHECK_XPATH_STRING(c, T("substring('abcd', 2)"), T("bcd"));
	CHECK_XPATH_STRING(c, T("substring('abcd', 1)"), T("abcd"));
	CHECK_XPATH_STRING(c, T("substring('abcd', 1.1)"), T("abcd"));
	CHECK_XPATH_STRING(c, T("substring('abcd', 1.5)"), T("bcd"));
	CHECK_XPATH_STRING(c, T("substring('abcd', 1.8)"), T("bcd"));
	CHECK_XPATH_STRING(c, T("substring('abcd', 10)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('abcd', 0)"), T("abcd"));
	CHECK_XPATH_STRING(c, T("substring('abcd', -100)"), T("abcd"));
	CHECK_XPATH_STRING(c, T("substring('abcd', -1 div 0)"), T("abcd"));
	CHECK_XPATH_STRING(c, T("substring('abcd', 1 div 0)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('abcd', 0 div 0)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('', 1)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('', 0)"), T(""));

	// substring with 3 arguments
	CHECK_XPATH_STRING(c, T("substring('abcd', 2, 1)"), T("b"));
	CHECK_XPATH_STRING(c, T("substring('abcd', 2, 2)"), T("bc"));
	CHECK_XPATH_STRING(c, T("substring('abcd', 1, 0)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('abcd', 1, 0.4)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('abcd', 1, 0.5)"), T("a"));
	CHECK_XPATH_STRING(c, T("substring('abcd', 10, -5)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('abcd', 0, -1)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('abcd', -100, 100)"), T("abcd"));
	CHECK_XPATH_STRING(c, T("substring('abcd', -1 div 0, 4)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('abcd', 1 div 0, 0 div 0)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('abcd', 0 div 0, 1)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('', 1, 2)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('', 0, 0)"), T(""));

	// substring with 3 arguments, from W3C standard
	CHECK_XPATH_STRING(c, T("substring('12345', 1.5, 2.6)"), T("234"));
	CHECK_XPATH_STRING(c, T("substring('12345', 0, 3)"), T("12"));
	CHECK_XPATH_STRING(c, T("substring('12345', 0 div 0, 3)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('12345', 1, 0 div 0)"), T(""));
	CHECK_XPATH_STRING(c, T("substring('12345', -42, 1 div 0)"), T("12345"));
	CHECK_XPATH_STRING(c, T("substring('12345', -1 div 0, 1 div 0)"), T(""));

	// substring with 4 arguments
	CHECK_XPATH_FAIL(T("substring('', 1, 2, 3)"));
}

TEST_XML(xpath_string_string_length, "<node>123</node>")
{
	xml_node c;
	xml_node n = doc.child(T("node"));

	// string-length with 0 arguments
	CHECK_XPATH_NUMBER(c, T("string-length()"), 0);
	CHECK_XPATH_NUMBER(n, T("string-length()"), 3);

	// string-length with 1 argument
	CHECK_XPATH_NUMBER(c, T("string-length('')"), 0);
	CHECK_XPATH_NUMBER(c, T("string-length('a')"), 1);
	CHECK_XPATH_NUMBER(c, T("string-length('abcdef')"), 6);

	// string-length with 2 arguments
	CHECK_XPATH_FAIL(T("string-length(1, 2)"));
}

TEST_XML_FLAGS(xpath_string_normalize_space, "<node> \t\r\rval1  \rval2\r\nval3\nval4\r\r</node>", parse_minimal)
{
	xml_node c;
	xml_node n = doc.child(T("node"));

	// normalize-space with 0 arguments
	CHECK_XPATH_STRING(c, T("normalize-space()"), T(""));
	CHECK_XPATH_STRING(n, T("normalize-space()"), T("val1 val2 val3 val4"));
	
	// normalize-space with 1 argument
	CHECK_XPATH_STRING(c, T("normalize-space('')"), T(""));
	CHECK_XPATH_STRING(c, T("normalize-space('abcd')"), T("abcd"));
	CHECK_XPATH_STRING(c, T("normalize-space(' \r\nabcd')"), T("abcd"));
	CHECK_XPATH_STRING(c, T("normalize-space('abcd \n\r')"), T("abcd"));
	CHECK_XPATH_STRING(c, T("normalize-space('ab\r\n\tcd')"), T("ab cd"));
	CHECK_XPATH_STRING(c, T("normalize-space('ab    cd')"), T("ab cd"));
	CHECK_XPATH_STRING(c, T("normalize-space('\07')"), T("\07"));
	
	// normalize-space with 2 arguments
	CHECK_XPATH_FAIL(T("normalize-space(1, 2)"));
}

TEST(xpath_string_translate)
{
	xml_node c;

	// translate with 0 arguments
	CHECK_XPATH_FAIL(T("translate()"));
	
	// translate with 1 argument
	CHECK_XPATH_FAIL(T("translate('a')"));

	// translate with 2 arguments
	CHECK_XPATH_FAIL(T("translate('a', 'b')"));
	
	// translate with 3 arguments
	CHECK_XPATH_STRING(c, T("translate('abc', '', '')"), T("abc"));
	CHECK_XPATH_STRING(c, T("translate('abc', '', 'foo')"), T("abc"));
	CHECK_XPATH_STRING(c, T("translate('abc', 'ab', 'ba')"), T("bac"));
	CHECK_XPATH_STRING(c, T("translate('abc', 'ab', 'f')"), T("fc"));
	CHECK_XPATH_STRING(c, T("translate('abc', 'aabb', '1234')"), T("13c"));
	CHECK_XPATH_STRING(c, T("translate('', 'abc', 'bac')"), T(""));

	// translate with 3 arguments, from W3C standard
	CHECK_XPATH_STRING(c, T("translate('bar','abc','ABC')"), T("BAr"));
	CHECK_XPATH_STRING(c, T("translate('--aaa--','abc-','ABC')"), T("AAA"));

	// translate with 4 arguments
	CHECK_XPATH_FAIL(T("translate('a', 'b', 'c', 'd')"));
}

TEST_XML(xpath_nodeset_last, "<node><c1/><c1/><c2/><c3/><c3/><c3/><c3/></node>")
{
	doc.precompute_document_order();

	xml_node n = doc.child(T("node"));

	// last with 0 arguments
	CHECK_XPATH_NUMBER(n, T("last()"), 1);
	CHECK_XPATH_NODESET(n, T("c1[last() = 1]"));
	CHECK_XPATH_NODESET(n, T("c1[last() = 2]")) % 3 % 4; // c1, c1
	CHECK_XPATH_NODESET(n, T("c2/preceding-sibling::node()[last() = 2]")) % 4 % 3; // c1, c1

	// last with 1 argument
	CHECK_XPATH_FAIL(T("last(c)"));
}

TEST_XML(xpath_nodeset_position, "<node><c1/><c1/><c2/><c3/><c3/><c3/><c3/></node>")
{
	doc.precompute_document_order();

	xml_node n = doc.child(T("node"));

	// position with 0 arguments
	CHECK_XPATH_NUMBER(n, T("position()"), 1);
	CHECK_XPATH_NODESET(n, T("c1[position() = 0]"));
	CHECK_XPATH_NODESET(n, T("c1[position() = 1]")) % 3;
	CHECK_XPATH_NODESET(n, T("c1[position() = 2]")) % 4;
	CHECK_XPATH_NODESET(n, T("c1[position() = 3]"));
	CHECK_XPATH_NODESET(n, T("c2/preceding-sibling::node()[position() = 1]")) % 4;
	CHECK_XPATH_NODESET(n, T("c2/preceding-sibling::node()[position() = 2]")) % 3;
	
	// position with 1 argument
	CHECK_XPATH_FAIL(T("position(c)"));
}

TEST_XML(xpath_nodeset_count, "<node><c1/><c1/><c2/><c3/><c3/><c3/><c3/></node>")
{
	xml_node c;
	xml_node n = doc.child(T("node"));

	// count with 0 arguments
	CHECK_XPATH_FAIL(T("count()"));

	// count with 1 non-node-set argument
	CHECK_XPATH_FAIL(T("count(1)"));
	CHECK_XPATH_FAIL(T("count(true())"));
	CHECK_XPATH_FAIL(T("count('')"));

	// count with 1 node-set argument
	CHECK_XPATH_NUMBER(c, T("count(.)"), 0);
	CHECK_XPATH_NUMBER(n, T("count(.)"), 1);
	CHECK_XPATH_NUMBER(n, T("count(c1)"), 2);
	CHECK_XPATH_NUMBER(n, T("count(c2)"), 1);
	CHECK_XPATH_NUMBER(n, T("count(c3)"), 4);
	CHECK_XPATH_NUMBER(n, T("count(c4)"), 0);

	// count with 2 arguments
	CHECK_XPATH_FAIL(T("count(x, y)"));
}

TEST_XML(xpath_nodeset_id, "<node id='foo'/>")
{
	xml_node n = doc.child(T("node"));

	// id with 0 arguments
	CHECK_XPATH_FAIL(T("id()"));
	
	// id with 1 argument - no DTD => no id
	CHECK_XPATH_NODESET(n, T("id('foo')"));

	// id with 2 arguments
	CHECK_XPATH_FAIL(T("id(1, 2)"));
}

TEST_XML_FLAGS(xpath_nodeset_local_name, "<node xmlns:foo='http://foo'><c1>text</c1><c2 xmlns:foo='http://foo2' foo:attr='value'><foo:child/></c2><c3 xmlns='http://def' attr='value'><child/></c3><c4><?target stuff?></c4></node>", parse_default | parse_pi)
{
	xml_node c;
	xml_node n = doc.child(T("node"));

	// local-name with 0 arguments
	CHECK_XPATH_STRING(c, T("local-name()"), T(""));
	CHECK_XPATH_STRING(n, T("local-name()"), T("node"));
	
	// local-name with 1 non-node-set argument
	CHECK_XPATH_FAIL(T("local-name(1)"));

	// local-name with 1 node-set argument
	CHECK_XPATH_STRING(n, T("local-name(c1)"), T("c1"));
	CHECK_XPATH_STRING(n, T("local-name(c2/node())"), T("child"));
	CHECK_XPATH_STRING(n, T("local-name(c2/attribute::node())"), T("attr"));
	CHECK_XPATH_STRING(n, T("local-name(c1/node())"), T(""));
	CHECK_XPATH_STRING(n, T("local-name(c4/node())"), T("target"));
	CHECK_XPATH_STRING(n, T("local-name(c1/following-sibling::node())"), T("c2"));
	CHECK_XPATH_STRING(n, T("local-name(c4/preceding-sibling::node())"), T("c1"));

	// local-name with 2 arguments
	CHECK_XPATH_FAIL(T("local-name(c1, c2)"));
}

TEST_XML_FLAGS(xpath_nodeset_namespace_uri, "<node xmlns:foo='http://foo'><c1>text</c1><c2 xmlns:foo='http://foo2' foo:attr='value'><foo:child/></c2><c3 xmlns='http://def' attr='value'><child/></c3><c4><?target stuff?></c4><c5><foo:child/></c5><c6 bar:attr=''/></node>", parse_default | parse_pi)
{
	xml_node c;
	xml_node n = doc.child(T("node"));

	// namespace-uri with 0 arguments
	CHECK_XPATH_STRING(c, T("namespace-uri()"), T(""));
	CHECK_XPATH_STRING(n.child(T("c2")).child(T("foo:child")), T("namespace-uri()"), T("http://foo2"));
	
	// namespace-uri with 1 non-node-set argument
	CHECK_XPATH_FAIL(T("namespace-uri(1)"));

	// namespace-uri with 1 node-set argument
	CHECK_XPATH_STRING(n, T("namespace-uri(c1)"), T(""));
	CHECK_XPATH_STRING(n, T("namespace-uri(c5/child::node())"), T("http://foo"));
	CHECK_XPATH_STRING(n, T("namespace-uri(c2/attribute::node())"), T("http://foo2"));
	CHECK_XPATH_STRING(n, T("namespace-uri(c2/child::node())"), T("http://foo2"));
	CHECK_XPATH_STRING(n, T("namespace-uri(c1/child::node())"), T(""));
	CHECK_XPATH_STRING(n, T("namespace-uri(c4/child::node())"), T(""));
	CHECK_XPATH_STRING(n, T("namespace-uri(c3)"), T("http://def"));
	CHECK_XPATH_STRING(n, T("namespace-uri(c3/@attr)"), T("")); // the namespace name for an unprefixed attribute name always has no value (Namespaces in XML 1.0)
	CHECK_XPATH_STRING(n, T("namespace-uri(c3/child::node())"), T("http://def"));
	CHECK_XPATH_STRING(n, T("namespace-uri(c6/@bar:attr)"), T(""));

	// namespace-uri with 2 arguments
	CHECK_XPATH_FAIL(T("namespace-uri(c1, c2)"));
}

TEST_XML_FLAGS(xpath_nodeset_name, "<node xmlns:foo='http://foo'><c1>text</c1><c2 xmlns:foo='http://foo2' foo:attr='value'><foo:child/></c2><c3 xmlns='http://def' attr='value'><child/></c3><c4><?target stuff?></c4></node>", parse_default | parse_pi)
{
	xml_node c;
	xml_node n = doc.child(T("node"));

	// name with 0 arguments
	CHECK_XPATH_STRING(c, T("name()"), T(""));
	CHECK_XPATH_STRING(n, T("name()"), T("node"));
	
	// name with 1 non-node-set argument
	CHECK_XPATH_FAIL(T("name(1)"));

	// name with 1 node-set argument
	CHECK_XPATH_STRING(n, T("name(c1)"), T("c1"));
	CHECK_XPATH_STRING(n, T("name(c2/node())"), T("foo:child"));
	CHECK_XPATH_STRING(n, T("name(c2/attribute::node())"), T("foo:attr"));
	CHECK_XPATH_STRING(n, T("name(c1/node())"), T(""));
	CHECK_XPATH_STRING(n, T("name(c4/node())"), T("target"));
	CHECK_XPATH_STRING(n, T("name(c1/following-sibling::node())"), T("c2"));
	CHECK_XPATH_STRING(n, T("name(c4/preceding-sibling::node())"), T("c1"));

	// name with 2 arguments
	CHECK_XPATH_FAIL(T("name(c1, c2)"));
}

TEST(xpath_function_arguments)
{
	xml_node c;

	// conversion to string
	CHECK_XPATH_NUMBER(c, T("string-length(12)"), 2);
	
	// conversion to number
	CHECK_XPATH_NUMBER(c, T("round('1.2')"), 1);
	CHECK_XPATH_NUMBER(c, T("round('1.7')"), 2);

	// conversion to boolean
	CHECK_XPATH_BOOLEAN(c, T("not('1')"), false);
	CHECK_XPATH_BOOLEAN(c, T("not('')"), true);
	
	// conversion to node set
	CHECK_XPATH_FAIL(T("sum(1)"));

	// expression evaluation
	CHECK_XPATH_NUMBER(c, T("round((2 + 2 * 2) div 4)"), 2);
	
	// empty expressions
	CHECK_XPATH_FAIL(T("round(,)"));
	CHECK_XPATH_FAIL(T("substring(,)"));
	CHECK_XPATH_FAIL(T("substring('a',)"));
	CHECK_XPATH_FAIL(T("substring(,'a')"));

	// extra commas
	CHECK_XPATH_FAIL(T("round(,1)"));
	CHECK_XPATH_FAIL(T("round(1,)"));

	// lack of commas
	CHECK_XPATH_FAIL(T("substring(1 2)"));

	// whitespace after function name
	CHECK_XPATH_BOOLEAN(c, T("true ()"), true);

	// too many arguments
	CHECK_XPATH_FAIL(T("round(1, 2, 3, 4, 5, 6)"));
}

TEST_XML_FLAGS(xpath_string_value, "<node><c1>pcdata</c1><c2><child/></c2><c3 attr='avalue'/><c4><?target pivalue?></c4><c5><!--comment--></c5><c6><![CDATA[cdata]]></c6></node>", parse_default | parse_pi | parse_comments)
{
	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_STRING(c, T("string()"), T(""));
	CHECK_XPATH_STRING(doc, T("string()"), T("pcdatacdata"));
	CHECK_XPATH_STRING(n, T("string()"), T("pcdatacdata"));
	CHECK_XPATH_STRING(n, T("string(c1/node())"), T("pcdata"));
	CHECK_XPATH_STRING(n, T("string(c2/node())"), T(""));
	CHECK_XPATH_STRING(n, T("string(c3/@attr)"), T("avalue"));
	CHECK_XPATH_STRING(n, T("string(c4/node())"), T("pivalue"));
	CHECK_XPATH_STRING(n, T("string(c5/node())"), T("comment"));
	CHECK_XPATH_STRING(n, T("string(c6/node())"), T("cdata"));
}

#endif
