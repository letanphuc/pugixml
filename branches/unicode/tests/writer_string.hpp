#ifndef HEADER_WRITER_STRING_HPP
#define HEADER_WRITER_STRING_HPP

#include "../src/pugixml.hpp"

#include <string>

struct xml_writer_string: public pugi::xml_writer
{
	std::string contents;
	
	virtual void write(const void* data, size_t size);

	std::string as_narrow() const;
	std::wstring as_wide() const;
	std::basic_string<pugi::char_t> as_string() const;
};

#endif
