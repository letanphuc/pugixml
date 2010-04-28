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

	CHECK_NODE(doc, STR("<node attr=\"&lt;&gt;'&quot;&amp;&#04;&#13;&#10;\t\">&lt;&gt;'\"&amp;&#04;\r\n\t</node>"));
}

struct test_writer: xml_writer
{
	std::basic_string<pugi::char_t> contents;

	virtual void write(const void* data, size_t size)
	{
		CHECK(size % sizeof(pugi::char_t) == 0);
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

struct test_narrow_writer: xml_writer
{
	std::string contents;

	virtual void write(const void* data, size_t size)
	{
		contents += std::string(static_cast<const char*>(data), static_cast<const char*>(data) + size);
	}
};

struct test_wide_writer: xml_writer
{
	std::wstring contents;

	virtual void write(const void* data, size_t size)
	{
		CHECK(size % sizeof(wchar_t) == 0);
		contents += std::wstring(static_cast<const wchar_t*>(data), static_cast<const wchar_t*>(data) + size / sizeof(wchar_t));
	}
};

std::string write_narrow(xml_node node, unsigned int flags)
{
	test_narrow_writer writer;

	node.print(writer, STR(""), flags);

	return writer.contents;
}

bool test_write_narrow(xml_node node, unsigned int flags, const char* expected, size_t length)
{
	std::string result = write_narrow(node, flags);

	// check result
	if (result != std::string(expected, expected + length)) return false;

	// check comparison operator (incorrect implementation can theoretically early-out on zero terminators...)
	if (result == std::string(expected, expected + length - 1) + "?") return false;

	return true;
}

std::wstring write_wide(xml_node node, unsigned int flags)
{
	test_wide_writer writer;

	node.print(writer, STR(""), flags);

	return writer.contents;
}

TEST(write_encodings)
{
	unsigned int ui = 1;
	bool little_endian = *reinterpret_cast<char*>(&ui) == 1;

	static char s_utf8[] = "<\x54\xC2\xA2\xE2\x82\xAC\xF0\xA4\xAD\xA2/>";

	xml_document doc;
	CHECK(doc.load_buffer(s_utf8, sizeof(s_utf8), encoding_utf8));

	CHECK(write_narrow(doc, encoding_utf8) == "<\x54\xC2\xA2\xE2\x82\xAC\xF0\xA4\xAD\xA2 />\n");

	CHECK(test_write_narrow(doc, encoding_utf32_le, "<\x00\x00\x00\x54\x00\x00\x00\xA2\x00\x00\x00\xAC\x20\x00\x00\x62\x4B\x02\x00 \x00\x00\x00/\x00\x00\x00>\x00\x00\x00\n\x00\x00\x00", 36));
	CHECK(test_write_narrow(doc, encoding_utf32_be, "\x00\x00\x00<\x00\x00\x00\x54\x00\x00\x00\xA2\x00\x00\x20\xAC\x00\x02\x4B\x62\x00\x00\x00 \x00\x00\x00/\x00\x00\x00>\x00\x00\x00\n", 36));
	CHECK(write_narrow(doc, encoding_utf32) == write_narrow(doc, little_endian ? encoding_utf32_le : encoding_utf32_be));

	CHECK(test_write_narrow(doc, encoding_utf16_le, "<\x00\x54\x00\xA2\x00\xAC\x20\x52\xd8\x62\xdf \x00/\x00>\x00\n\x00", 20));
	CHECK(test_write_narrow(doc, encoding_utf16_be, "\x00<\x00\x54\x00\xA2\x20\xAC\xd8\x52\xdf\x62\x00 \x00/\x00>\x00\n", 20));
	CHECK(write_narrow(doc, encoding_utf16) == write_narrow(doc, little_endian ? encoding_utf16_le : encoding_utf16_be));

	size_t wcharsize = sizeof(wchar_t);
	std::wstring v = write_wide(doc, encoding_wchar);

	if (wcharsize == 4)
	{
		CHECK(v.size() == 9 && v[0] == '<' && v[1] == 0x54 && v[2] == 0xA2 && v[3] == 0x20AC && v[4] == wchar_cast(0x24B62) && v[5] == ' ' && v[6] == '/' && v[7] == '>' && v[8] == '\n');
	}
	else
	{
		CHECK(v.size() == 10 && v[0] == '<' && v[1] == 0x54 && v[2] == 0xA2 && v[3] == 0x20AC && v[4] == 0xd852 && v[5] == 0xdf62 && v[6] == ' ' && v[7] == '/' && v[8] == '>' && v[9] == '\n');
	}
}

#ifdef PUGIXML_WCHAR_MODE
TEST(write_encoding_huge)
{
	const unsigned int N = 16000;

	// make a large utf16 name consisting of 6-byte char pairs (6 does not divide internal buffer size, so will need split correction)
	std::string s_utf16 = std::string("\x00<", 2);

	for (unsigned int i = 0; i < N; ++i) s_utf16 += "\x20\xAC\xd8\x52\xdf\x62";

	s_utf16 += std::string("\x00/\x00>", 4);

	xml_document doc;
	CHECK(doc.load_buffer(&s_utf16[0], s_utf16.length(), encoding_utf16_be));

	std::string s_utf8 = "<";

	for (unsigned int j = 0; j < N; ++j) s_utf8 += "\xE2\x82\xAC\xF0\xA4\xAD\xA2";

	s_utf8 += " />\n";

	CHECK(test_write_narrow(doc, encoding_utf8, s_utf8.c_str(), s_utf8.length()));
}
#else
TEST(write_encoding_huge)
{
	const unsigned int N = 16000;

	// make a large utf8 name consisting of 3-byte chars (3 does not divide internal buffer size, so will need split correction)
	std::string s_utf8 = "<";

	for (unsigned int i = 0; i < N; ++i) s_utf8 += "\xE2\x82\xAC";

	s_utf8 += "/>";

	xml_document doc;
	CHECK(doc.load_buffer(&s_utf8[0], s_utf8.length(), encoding_utf8));

	std::string s_utf16 = std::string("\x00<", 2);

	for (unsigned int j = 0; j < N; ++j) s_utf16 += "\x20\xAC";

	s_utf16 += std::string("\x00 \x00/\x00>\x00\n", 8);

	CHECK(test_write_narrow(doc, encoding_utf16_be, s_utf16.c_str(), s_utf16.length()));
}
#endif

TEST(write_unicode_escape)
{
	char s_utf8[] = "<\xE2\x82\xAC \xC2\xA2='\"\xF0\xA4\xAD\xA2&#x0a;\"'>&amp;\x14\xF0\xA4\xAD\xA2&lt;</\xE2\x82\xAC>";
	
	xml_document doc;
	CHECK(doc.load_buffer(s_utf8, sizeof(s_utf8), parse_default | encoding_utf8));

	CHECK(write_narrow(doc, encoding_utf8) == "<\xE2\x82\xAC \xC2\xA2=\"&quot;\xF0\xA4\xAD\xA2&#10;&quot;\">&amp;&#20;\xF0\xA4\xAD\xA2&lt;</\xE2\x82\xAC>\n");
}
