#include "writer_string.hpp"

#include "test.hpp"

static bool test_narrow(const std::string& result, const char* expected, size_t length)
{
	// check result
	if (result != std::string(expected, expected + length)) return false;

	// check comparison operator (incorrect implementation can theoretically early-out on zero terminators...)
	if (length > 0 && result == std::string(expected, expected + length - 1) + "?") return false;

	return true;
}

void xml_writer_string::write(const void* data, size_t size)
{
	contents += std::string(static_cast<const char*>(data), size);
}

std::string xml_writer_string::as_narrow() const
{
	return contents;
}

std::wstring xml_writer_string::as_wide() const
{
	CHECK(contents.size() % sizeof(wchar_t) == 0);

	return contents.empty() ? L"" : std::wstring(reinterpret_cast<const wchar_t*>(contents.data()), contents.size() / sizeof(wchar_t));
}

std::basic_string<pugi::char_t> xml_writer_string::as_string() const
{
	CHECK(contents.size() % sizeof(pugi::char_t) == 0);

	return contents.empty() ? STR("") : std::basic_string<pugi::char_t>(reinterpret_cast<const pugi::char_t*>(contents.data()), contents.size() / sizeof(pugi::char_t));
}

std::string save_narrow(const pugi::xml_document& doc, unsigned int flags)
{
	xml_writer_string writer;

	doc.save(writer, STR(""), flags);

	return writer.as_narrow();
}

bool test_save_narrow(const pugi::xml_document& doc, unsigned int flags, const char* expected, size_t length)
{
	return test_narrow(save_narrow(doc, flags), expected, length);
}

std::string write_narrow(pugi::xml_node node, unsigned int flags)
{
	xml_writer_string writer;

	node.print(writer, STR(""), flags);

	return writer.as_narrow();
}

bool test_write_narrow(pugi::xml_node node, unsigned int flags, const char* expected, size_t length)
{
	return test_narrow(write_narrow(node, flags), expected, length);
}

std::wstring write_wide(pugi::xml_node node, unsigned int flags)
{
	xml_writer_string writer;

	node.print(writer, STR(""), flags);

	return writer.as_wide();
}
