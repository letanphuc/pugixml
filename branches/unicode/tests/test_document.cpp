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

struct test_narrow_writer: xml_writer
{
	std::string contents;

	virtual void write(const void* data, size_t size)
	{
		contents += std::string(static_cast<const char*>(data), static_cast<const char*>(data) + size);
	}
};

#ifdef __GNUC__
__attribute__((noinline)) // GCC 4.3 and 4.4 crash below on two calls to save_narrow in single expression, http://gcc.gnu.org/bugzilla/show_bug.cgi?id=42394
#endif
static std::string save_narrow(const xml_document& doc, unsigned int flags)
{
	test_narrow_writer writer;

	doc.save(writer, STR(""), flags);

	return writer.contents;
}

static bool test_save_narrow(const xml_document& doc, unsigned int flags, const char* expected, size_t length)
{
	std::string result = save_narrow(doc, flags);

	// check result
	if (result != std::string(expected, expected + length)) return false;

	// check comparison operator (incorrect implementation can theoretically early-out on zero terminators...)
	if (result == std::string(expected, expected + length - 1) + "?") return false;

	return true;
}

TEST_XML(document_save_bom, "<n/>")
{
	unsigned int ui = 1;
	bool little_endian = *reinterpret_cast<char*>(&ui) == 1;

	xml_writer_string writer;

	unsigned int flags = format_no_declaration | format_raw | format_write_bom;

	// specific encodings
	CHECK(test_save_narrow(doc, flags | encoding_utf8, "\xef\xbb\xbf<n />", 8));
	CHECK(test_save_narrow(doc, flags | encoding_utf16_be, "\xfe\xff\x00<\x00n\x00 \x00/\x00>", 12));
	CHECK(test_save_narrow(doc, flags | encoding_utf16_le, "\xff\xfe<\x00n\x00 \x00/\x00>\x00", 12));
	CHECK(test_save_narrow(doc, flags | encoding_utf32_be, "\x00\x00\xfe\xff\x00\x00\x00<\x00\x00\x00n\x00\x00\x00 \x00\x00\x00/\x00\x00\x00>", 24));
	CHECK(test_save_narrow(doc, flags | encoding_utf32_le, "\xff\xfe\x00\x00<\x00\x00\x00n\x00\x00\x00 \x00\x00\x00/\x00\x00\x00>\x00\x00\x00", 24));

	// encodings synonyms
	CHECK(save_narrow(doc, flags | encoding_utf16) == save_narrow(doc, flags | (little_endian ? encoding_utf16_le : encoding_utf16_be)));
	CHECK(save_narrow(doc, flags | encoding_utf32) == save_narrow(doc, flags | (little_endian ? encoding_utf32_le : encoding_utf32_be)));

	size_t wcharsize = sizeof(wchar_t);
	CHECK(save_narrow(doc, flags | encoding_wchar) == save_narrow(doc, flags | (wcharsize == 2 ? encoding_utf16 : encoding_utf32)));
}

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
		"tests/data/utftest_utf16_be_nodecl.xml",
		"tests/data/utftest_utf16_le.xml",
		"tests/data/utftest_utf16_le_bom.xml",
		"tests/data/utftest_utf16_le_nodecl.xml",
		"tests/data/utftest_utf32_be.xml",
		"tests/data/utftest_utf32_be_bom.xml",
		"tests/data/utftest_utf32_be_nodecl.xml",
		"tests/data/utftest_utf32_le.xml",
		"tests/data/utftest_utf32_le_bom.xml",
		"tests/data/utftest_utf32_le_nodecl.xml",
		"tests/data/utftest_utf8.xml",
		"tests/data/utftest_utf8_bom.xml",
		"tests/data/utftest_utf8_nodecl.xml"
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
		"tests/data/utftest_utf16_be_nodecl.xml",
		"tests/data/utftest_utf16_le.xml",
		"tests/data/utftest_utf16_le_bom.xml",
		"tests/data/utftest_utf16_le_nodecl.xml",
		"tests/data/utftest_utf32_be.xml",
		"tests/data/utftest_utf32_be_bom.xml",
		"tests/data/utftest_utf32_be_nodecl.xml",
		"tests/data/utftest_utf32_le.xml",
		"tests/data/utftest_utf32_le_bom.xml",
		"tests/data/utftest_utf32_le_nodecl.xml",
		"tests/data/utftest_utf8.xml",
		"tests/data/utftest_utf8_bom.xml",
		"tests/data/utftest_utf8_nodecl.xml"
	};

	unsigned int encodings[] =
	{
		encoding_utf16_be, encoding_utf16_be, encoding_utf16_be,
		encoding_utf16_le, encoding_utf16_le, encoding_utf16_le,
		encoding_utf32_be, encoding_utf32_be, encoding_utf32_be,
		encoding_utf32_le, encoding_utf32_le, encoding_utf32_le,
		encoding_utf8, encoding_utf8, encoding_utf8
	};

	for (unsigned int i = 0; i < sizeof(files) / sizeof(files[0]); ++i)
	{
		for (unsigned int j = 0; j < sizeof(files) / sizeof(files[0]); ++j)
		{
			unsigned int encoding = encodings[j];

			xml_document doc;
			xml_parse_result res = doc.load_file(files[i], encoding);

			if (encoding == encodings[i])
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

TEST(document_load_file_convert_native_endianness)
{
	unsigned int ui = 1;
	bool little_endian = *reinterpret_cast<char*>(&ui) == 1;

	const char* files[2][6] =
	{
		{
			"tests/data/utftest_utf16_be.xml",
			"tests/data/utftest_utf16_be_bom.xml",
			"tests/data/utftest_utf16_be_nodecl.xml",
			"tests/data/utftest_utf32_be.xml",
			"tests/data/utftest_utf32_be_bom.xml",
			"tests/data/utftest_utf32_be_nodecl.xml",
		},
		{
			"tests/data/utftest_utf16_le.xml",
			"tests/data/utftest_utf16_le_bom.xml",
			"tests/data/utftest_utf16_le_nodecl.xml",
			"tests/data/utftest_utf32_le.xml",
			"tests/data/utftest_utf32_le_bom.xml",
			"tests/data/utftest_utf32_le_nodecl.xml",
		}
	};

	unsigned int encodings[] =
	{
		encoding_utf16, encoding_utf16, encoding_utf16,
		encoding_utf32, encoding_utf32, encoding_utf32
	};

	for (unsigned int i = 0; i < sizeof(files[0]) / sizeof(files[0][0]); ++i)
	{
		const char* right_file = files[little_endian][i];
		const char* wrong_file = files[!little_endian][i];

		for (unsigned int j = 0; j < sizeof(encodings) / sizeof(encodings[0]); ++j)
		{
			unsigned int encoding = encodings[j];

			// check file with right endianness
			{
				xml_document doc;
				xml_parse_result res = doc.load_file(right_file, encoding);

				if (encoding == encodings[i])
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

			// check file with wrong endianness
			{
				xml_document doc;
				doc.load_file(wrong_file, encoding);
				CHECK(!doc.first_child());
			}
		}
	}
}
