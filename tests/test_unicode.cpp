#include "common.hpp"

// letters taken from http://www.utf8-chartable.de/

#ifndef PUGIXML_NO_STL
TEST(as_utf16)
{
	CHECK(as_utf16("") == L"");

	// valid 1-byte, 2-byte and 3-byte inputs
#ifdef U_LITERALS
	CHECK(as_utf16("?\xd0\x80\xe2\x80\xbd") == L"?\u0400\u203D");
#else
	CHECK(as_utf16("?\xd0\x80\xe2\x80\xbd") == L"?\x0400\x203D");
#endif

	// invalid 1-byte input
	CHECK(as_utf16("\xb0") == L"");
	
	// valid 4-byte input
	std::wstring b4 = as_utf16("\xf2\x97\x98\xa4 \xf4\x80\x8f\xbf");

	size_t wcharsize = sizeof(wchar_t);

	if (wcharsize == 4)
	{
		CHECK(b4.size() == 3 && b4[0] == wchar_cast(0x97624) && b4[1] == L' ' && b4[2] == wchar_cast(0x1003ff));
	}
	else
	{
		CHECK(b4.size() == 5 && b4[0] == 0xda1d && b4[1] == 0xde24 && b4[2] == L' ' && b4[3] == 0xdbc0 && b4[4] == 0xdfff);
	}

	// invalid 5-byte input
	std::wstring b5 = as_utf16("\xf8\nbcd");
	CHECK(b5 == L"\nbcd");
}

TEST(as_utf8)
{
	CHECK(as_utf8(L"") == "");

	// valid 1-byte, 2-byte and 3-byte outputs
#ifdef U_LITERALS
	CHECK(as_utf8(L"?\u0400\u203D") == "?\xd0\x80\xe2\x80\xbd");
#else
	CHECK(as_utf8(L"?\x0400\x203D") == "?\xd0\x80\xe2\x80\xbd");
#endif
	
	// valid 4-byte output
	size_t wcharsize = sizeof(wchar_t);

	if (wcharsize == 4)
	{
		std::wstring s;
		s.resize(3);
		s[0] = wchar_cast(0x97624);
		s[1] = ' ';
		s[2] = wchar_cast(0x1003ff);

		CHECK(as_utf8(s.c_str()) == "\xf2\x97\x98\xa4 \xf4\x80\x8f\xbf");
	}
	else
	{
	#ifdef U_LITERALS
		CHECK(as_utf8(L"\uda1d\ude24 \udbc0\udfff") == "\xf2\x97\x98\xa4 \xf4\x80\x8f\xbf");
	#else
		CHECK(as_utf8(L"\xda1d\xde24 \xdbc0\xdfff") == "\xf2\x97\x98\xa4 \xf4\x80\x8f\xbf");
	#endif
	}
}
#endif

TEST_XML(parse_bom_utf8, "\xef\xbb\xbf<node/>")
{
	CHECK_NODE(doc, STR("<node />"));
}
