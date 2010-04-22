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

TEST(document_load_stream_empty)
{
	pugi::xml_document doc;

	std::istringstream iss;
	CHECK(doc.load(iss));
	CHECK(!doc.first_child());
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

#ifndef PUGIXML_WCHAR_MODE
// $$$ tests are not active right now (no utf8->wchar_t conversion implemented)
// also we'll require more load_file tests (all utf variants)
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
#endif

TEST(document_load_file_error)
{
	pugi::xml_document doc;

	CHECK(doc.load_file("filedoesnotexist").status == status_file_not_found);

#ifdef __unix
	CHECK(doc.load_file("/dev/null").status == status_io_error);
#endif

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

TEST(document_parse)
{
	pugi::char_t text[] = STR("<node/>");

	pugi::xml_document doc;

	CHECK(doc.parse(text));
	CHECK_NODE(doc, STR("<node />"));
}

TEST(document_parse_transfer_ownership)
{
	allocation_function alloc = get_memory_allocation_function();

	size_t size = (strlen("<node/>") + 1) * sizeof(pugi::char_t);

	pugi::char_t* text = static_cast<pugi::char_t*>(alloc(size));
	CHECK(text);

	memcpy(text, STR("<node/>"), size);

	pugi::xml_document doc;

	CHECK(doc.parse(transfer_ownership_tag(), text));
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
