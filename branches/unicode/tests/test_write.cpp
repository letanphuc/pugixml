#include "common.hpp"

#include <string>
#include <sstream>

TEST_XML(write_simple, "<node attr='1'><child>text</child></node>")
{
	CHECK_NODE_EX(doc, STR("<node attr=\"1\">\n<child>text</child>\n</node>\n"), STR(""), 0);
}

TEST_XML(write_raw, "<node attr='1'><child>text</child></node>")
{
	CHECK_NODE_EX(doc, STR("<node attr=\"1\"><child>text</child></node>"), STR(""), format_raw);
}

TEST_XML(write_indent, "<node attr='1'><child><sub>text</sub></child></node>")
{
	CHECK_NODE_EX(doc, STR("<node attr=\"1\">\n\t<child>\n\t\t<sub>text</sub>\n\t</child>\n</node>\n"), STR("\t"), format_indent);
}

TEST_XML(write_pcdata, "<node attr='1'><child><sub/>text</child></node>")
{
	CHECK_NODE_EX(doc, STR("<node attr=\"1\">\n\t<child>\n\t\t<sub />\n\t\ttext\n\t</child>\n</node>\n"), STR("\t"), format_indent);
}

TEST_XML(write_cdata, "<![CDATA[value]]>")
{
	CHECK_NODE(doc, STR("<![CDATA[value]]>"));
	CHECK_NODE_EX(doc, STR("<![CDATA[value]]>\n"), STR(""), 0);
}

TEST_XML_FLAGS(write_comment, "<!--text-->", parse_default | parse_comments)
{
	CHECK_NODE(doc, STR("<!--text-->"));
	CHECK_NODE_EX(doc, STR("<!--text-->\n"), STR(""), 0);
}

TEST_XML_FLAGS(write_pi, "<?name value?>", parse_default | parse_pi)
{
	CHECK_NODE(doc, STR("<?name value?>"));
	CHECK_NODE_EX(doc, STR("<?name value?>\n"), STR(""), 0);
}

TEST_XML_FLAGS(write_declaration, "<?xml version='2.0'?>", parse_default | parse_declaration)
{
	CHECK_NODE(doc, STR("<?xml version=\"2.0\"?>"));
	CHECK_NODE_EX(doc, STR("<?xml version=\"2.0\"?>\n"), STR(""), 0);
}

TEST_XML(write_escape, "<node attr=''>text</node>")
{
	doc.child(STR("node")).attribute(STR("attr")) = STR("<>'\"&\x04\r\n\t");
	doc.child(STR("node")).first_child().set_value(STR("<>'\"&\x04\r\n\t"));

	CHECK_NODE(doc, STR("<node attr=\"&lt;&gt;'&quot;&amp;&#4;&#13;&#10;\t\">&lt;&gt;'\"&amp;&#4;\r\n\t</node>"));
}

struct test_writer: xml_writer
{
	std::basic_string<pugi::char_t> contents;

	virtual void write(const void* data, size_t size)
	{
		contents += std::basic_string<pugi::char_t>(static_cast<const pugi::char_t*>(data), static_cast<const pugi::char_t*>(data) + size / sizeof(pugi::char_t));
	}
};

TEST_XML(write_print_writer, "<node/>")
{
	test_writer writer;
	doc.print(writer);

	CHECK(writer.contents == STR("<node />\n"));
}

#ifndef PUGIXML_NO_STL
TEST_XML(write_print_stream, "<node/>")
{
	std::ostringstream oss;
	doc.print(oss);

#ifndef PUGIXML_WCHAR_MODE // $$$ fix this (should we provide a writer for wide stream if we're wide? probably)
	CHECK(oss.str() == STR("<node />\n"));
#endif
}
#endif

TEST_XML(write_huge_chunk, "<node/>")
{
	std::basic_string<pugi::char_t> name(10000, STR('n'));
	doc.child(STR("node")).set_name(name.c_str());

	test_writer writer;
	doc.print(writer);

	CHECK(writer.contents == STR("<") + name + STR(" />\n"));
}
