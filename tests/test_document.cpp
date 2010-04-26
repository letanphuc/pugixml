#include <string.h>

#include "common.hpp"

#include <fstream>
#include <sstream>
#include <string>

#include <stdio.h>

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

TEST(document_create)
{
	pugi::xml_document doc;
	doc.append_child().set_name(STR("node"));
	CHECK_NODE(doc, STR("<node />"));
}

#ifndef PUGIXML_NO_STL
#ifndef PUGIXML_WCHAR_MODE
// $$$ recover (wide streams?)
TEST(document_load_stream)
{
	pugi::xml_document doc;

	std::istringstream iss("<node/>");
	CHECK(doc.load(iss));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_stream_offset)
{
	pugi::xml_document doc;

	std::istringstream iss("<foobar> <node/>");

	std::string s;
	iss >> s;

	CHECK(doc.load(iss));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_stream_text)
{
	pugi::xml_document doc;

	std::ifstream iss("tests/data/multiline.xml");
	CHECK(doc.load(iss));
	CHECK_NODE(doc, STR("<node1 /><node2 /><node3 />"));
}
#endif

TEST(document_load_stream_error)
{
	pugi::xml_document doc;

	std::ifstream fs1("filedoesnotexist");
	CHECK(doc.load(fs1).status == status_io_error);
	
#ifndef __DMC__ // Digital Mars CRT does not like 'con' pseudo-file
	std::ifstream fs2("con");
	CHECK(doc.load(fs2).status == status_io_error);
#endif

	test_runner::_memory_fail_threshold = 1;
	std::istringstream iss("<node/>");
	CHECK(doc.load(iss).status == status_out_of_memory);
}
#endif

TEST(document_load_string)
{
	pugi::xml_document doc;

	CHECK(doc.load(STR("<node/>")));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_file)
{
	pugi::xml_document doc;

	CHECK(doc.load_file("tests/data/small.xml"));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_file_empty)
{
	pugi::xml_document doc;

	CHECK(doc.load_file("tests/data/empty.xml"));
	CHECK(!doc.first_child());
}

TEST(document_load_file_large)
{
	pugi::xml_document doc;

	CHECK(doc.load_file("tests/data/large.xml"));

	std::basic_string<pugi::char_t> str;
	str += STR("<node>");
	for (int i = 0; i < 10000; ++i) str += STR("<node />");
	str += STR("</node>");

	CHECK_NODE(doc, str.c_str());
}

TEST(document_load_file_error)
{
	pugi::xml_document doc;

	CHECK(doc.load_file("filedoesnotexist").status == status_file_not_found);

#ifdef _WIN32
#ifndef __DMC__ // Digital Mars CRT does not like 'con' pseudo-file
	CHECK(doc.load_file("con").status == status_io_error);
#endif
#endif

	test_runner::_memory_fail_threshold = 1;
	CHECK(doc.load_file("tests/data/small.xml").status == status_out_of_memory);
}

TEST_XML(document_save, "<node/>")
{
	xml_writer_string writer;

	doc.save(writer, STR(""), pugi::format_no_declaration | pugi::format_raw);

	CHECK(writer.result == STR("<node />"));
}

#ifndef PUGIXML_WCHAR_MODE
// $$$ fix this (custom writer?)
TEST_XML(document_save_bom_utf8, "<node/>")
{
	xml_writer_string writer;

	doc.save(writer, STR(""), pugi::format_no_declaration | pugi::format_raw | pugi::format_write_bom_utf8);

	CHECK(writer.result == STR("\xef\xbb\xbf<node />"));
}
#endif

TEST_XML(document_save_declaration, "<node/>")
{
	xml_writer_string writer;

	doc.save(writer);

	CHECK(writer.result == STR("<?xml version=\"1.0\"?>\n<node />\n"));
}

TEST_XML(document_save_file, "<node/>")
{
	CHECK(doc.save_file("tests/data/output.xml"));

	CHECK(doc.load_file("tests/data/output.xml", pugi::parse_default | pugi::parse_declaration));
	CHECK_NODE(doc, STR("<?xml version=\"1.0\"?><node />"));

	unlink("tests/data/output.xml");
}

TEST(document_load_buffer)
{
	const pugi::char_t text[] = STR("<?xml?><node/>");

	pugi::xml_document doc;

	CHECK(doc.load_buffer(text, sizeof(text)));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_buffer_inplace)
{
	pugi::char_t text[] = STR("<?xml?><node/>");

	pugi::xml_document doc;

	CHECK(doc.load_buffer_inplace(text, sizeof(text)));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_load_buffer_inplace_own)
{
	allocation_function alloc = get_memory_allocation_function();

	size_t size = strlen("<?xml?><node/>") * sizeof(pugi::char_t);

	pugi::char_t* text = static_cast<pugi::char_t*>(alloc(size));
	CHECK(text);

	memcpy(text, STR("<?xml?><node/>"), size);

	pugi::xml_document doc;

	CHECK(doc.load_buffer_inplace_own(text, size));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_parse_result_bool)
{
	xml_parse_result result;

	result.status = status_ok;
	CHECK(result);
	CHECK(!!result);
	CHECK(result == true);

	for (int i = 1; i < 20; ++i)
	{
		result.status = (xml_parse_status)i;
		CHECK(!result);
		CHECK(result == false);
	}
}

TEST(document_parse_result_description)
{
	xml_parse_result result;

	for (int i = 0; i < 20; ++i)
	{
		result.status = (xml_parse_status)i;

		CHECK(result.description() != 0);
		CHECK(result.description()[0] != 0);
	}
}

TEST(document_load_fail)
{
	xml_document doc;
	CHECK(!doc.load(STR("<foo><bar/>")));
	CHECK(doc.child(STR("foo")).child(STR("bar")));
}

inline void check_utftest_document(const xml_document& doc)
{
	// ascii text
	CHECK_STRING(doc.last_child().first_child().name(), STR("English"));

	// check that we have parsed some non-ascii text
	CHECK((unsigned)doc.last_child().last_child().name()[0] >= 0x80);

	// check magic string
	const pugi::char_t* v = doc.last_child().child(STR("Heavy")).previous_sibling().child_value();

#ifdef PUGIXML_WCHAR_MODE
	CHECK(v[0] == 0x4e16 && v[1] == 0x754c && v[2] == 0x6709 && v[3] == 0x5f88 && v[4] == 0x591a && v[5] == 0x8bed && v[6] == 0x8a00);

	// last character is a surrogate pair
	unsigned int v7 = v[7];
	size_t wcharsize = sizeof(wchar_t);

	CHECK(wcharsize == 2 ? (v[7] == 0xd852 && v[8] == 0xdf62) : (v7 == 0x24b62));
#else
	// unicode string
	CHECK_STRING(v, "\xe4\xb8\x96\xe7\x95\x8c\xe6\x9c\x89\xe5\xbe\x88\xe5\xa4\x9a\xe8\xaf\xad\xe8\xa8\x80\xf0\xa4\xad\xa2");
#endif
}

TEST(document_load_file_convert_auto)
{
	const char* files[] =
	{
		"tests/data/utftest_utf16_be.xml",
		"tests/data/utftest_utf16_be_bom.xml",
		"tests/data/utftest_utf16_le.xml",
		"tests/data/utftest_utf16_le_bom.xml",
		"tests/data/utftest_utf32_be.xml",
		"tests/data/utftest_utf32_be_bom.xml",
		"tests/data/utftest_utf32_le.xml",
		"tests/data/utftest_utf32_le_bom.xml",
		"tests/data/utftest_utf8.xml",
		"tests/data/utftest_utf8_bom.xml"
	};

	for (unsigned int i = 0; i < sizeof(files) / sizeof(files[0]); ++i)
	{
		xml_document doc;
		CHECK(doc.load_file(files[i]));
		check_utftest_document(doc);
	}
}

TEST(document_load_file_convert_specific)
{
	const char* files[] =
	{
		"tests/data/utftest_utf16_be.xml",
		"tests/data/utftest_utf16_be_bom.xml",
		"tests/data/utftest_utf16_le.xml",
		"tests/data/utftest_utf16_le_bom.xml",
		"tests/data/utftest_utf32_be.xml",
		"tests/data/utftest_utf32_be_bom.xml",
		"tests/data/utftest_utf32_le.xml",
		"tests/data/utftest_utf32_le_bom.xml",
		"tests/data/utftest_utf8.xml",
		"tests/data/utftest_utf8_bom.xml"
	};

	unsigned int formats[] =
	{
		parse_format_utf16_be, parse_format_utf16_be,
		parse_format_utf16_le, parse_format_utf16_le,
		parse_format_utf32_be, parse_format_utf32_be,
		parse_format_utf32_le, parse_format_utf32_le,
		parse_format_utf8, parse_format_utf8
	};

	for (unsigned int i = 0; i < sizeof(files) / sizeof(files[0]); ++i)
	{
		for (unsigned int j = 0; j < sizeof(files) / sizeof(files[0]); ++j)
		{
			unsigned int format = formats[j];

			xml_document doc;
			xml_parse_result res = doc.load_file(files[i], format);

			if (format == formats[i])
			{
				CHECK(res);
				check_utftest_document(doc);
			}
			else
			{
				// should not get past first tag
				CHECK(!doc.first_child());
			}
		}
	}
}
