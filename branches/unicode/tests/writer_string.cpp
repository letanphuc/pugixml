#include "writer_string.hpp"

#include "test.hpp"

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
