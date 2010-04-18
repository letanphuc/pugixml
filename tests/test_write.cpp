#include "common.hpp"

#include <string>
#include <sstream>

TEST_XML(write_simple, "<node attr='1'><child>text</child></node>")
{
	CHECK_NODE_EX(doc, T("<node attr=\"1\">\n<child>text</child>\n</node>\n"), T(""), 0);
}

TEST_XML(write_raw, "<node attr='1'><child>text</child></node>")
{
	CHECK_NODE_EX(doc, T("<node attr=\"1\"><child>text</child></node>"), T(""), format_raw);
}

TEST_XML(write_indent, "<node attr='1'><child><sub>text</sub></child></node>")
{
	CHECK_NODE_EX(doc, T("<node attr=\"1\">\n\t<child>\n\t\t<sub>text</sub>\n\t</child>\n</node>\n"), T("\t"), format_indent);
}

TEST_XML(write_pcdata, "<node attr='1'><child><sub/>text</child></node>")
{
	CHECK_NODE_EX(doc, T("<node attr=\"1\">\n\t<child>\n\t\t<sub />\n\t\ttext\n\t</child>\n</node>\n"), T("\t"), format_indent);
}

TEST_XML(write_cdata, "<![CDATA[value]]>")
{
	CHECK_NODE(doc, T("<![CDATA[value]]>"));
	CHECK_NODE_EX(doc, T("<![CDATA[value]]>\n"), T(""), 0);
}

TEST_XML_FLAGS(write_comment, "<!--text-->", parse_default | parse_comments)
{
	CHECK_NODE(doc, T("<!--text-->"));
	CHECK_NODE_EX(doc, T("<!--text-->\n"), T(""), 0);
}

TEST_XML_FLAGS(write_pi, "<?name value?>", parse_default | parse_pi)
{
	CHECK_NODE(doc, T("<?name value?>"));
	CHECK_NODE_EX(doc, T("<?name value?>\n"), T(""), 0);
}

TEST_XML_FLAGS(write_declaration, "<?xml version='2.0'?>", parse_default | parse_declaration)
{
	CHECK_NODE(doc, T("<?xml version=\"2.0\"?>"));
	CHECK_NODE_EX(doc, T("<?xml version=\"2.0\"?>\n"), T(""), 0);
}

TEST_XML(write_escape, "<node attr=''>text</node>")
{
	doc.child(T("node")).attribute(T("attr")) = T("<>'\"&\x04\r\n\t");
	doc.child(T("node")).first_child().set_value(T("<>'\"&\x04\r\n\t"));

	CHECK_NODE(doc, T("<node attr=\"&lt;&gt;'&quot;&amp;&#4;&#13;&#10;\t\">&lt;&gt;'\"&amp;&#4;\r\n\t</node>"));
}

struct test_writer: xml_writer
{
	pugi::string_t contents;

	virtual void write(const void* data, size_t size)
	{
		contents += pugi::string_t(static_cast<const pugi::char_t*>(data), static_cast<const pugi::char_t*>(data) + size / sizeof(pugi::char_t));
	}
};

TEST_XML(write_print_writer, "<node/>")
{
	test_writer writer;
	doc.print(writer);

	CHECK(writer.contents == T("<node />\n"));
}

#ifndef PUGIXML_NO_STL
TEST_XML(write_print_stream, "<node/>")
{
	std::ostringstream oss;
	doc.print(oss);

	CHECK(oss.str() == T("<node />\n"));
}
#endif

TEST_XML(write_huge_chunk, "<node/>")
{
	pugi::string_t name(10000, T('n'));
	doc.child(T("node")).set_name(name.c_str());

	test_writer writer;
	doc.print(writer);

	CHECK(writer.contents == T("<") + name + T(" />\n"));
}
