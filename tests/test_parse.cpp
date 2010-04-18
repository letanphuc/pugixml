#include "common.hpp"

TEST(parse_pi_skip)
{
	xml_document doc;
	CHECK(doc.load(T("<?pi?><?pi value?>"), parse_minimal));
	CHECK(!doc.first_child());
}

TEST(parse_pi_parse)
{
	xml_document doc;
	CHECK(doc.load(T("<?pi1?><?pi2 value?>"), parse_minimal | parse_pi));

	xml_node pi1 = doc.first_child();
	xml_node pi2 = doc.last_child();

	CHECK(pi1 != pi2);
	CHECK(pi1.type() == node_pi);
	CHECK_STRING(pi1.name(), T("pi1"));
	CHECK_STRING(pi1.value(), T(""));
	CHECK(pi2.type() == node_pi);
	CHECK_STRING(pi2.name(), T("pi2"));
	CHECK_STRING(pi2.value(), T("value"));
}

TEST(parse_pi_error)
{
	xml_document doc;

	unsigned int flag_sets[] = {parse_minimal, parse_minimal | parse_pi};

	for (unsigned int i = 0; i < sizeof(flag_sets) / sizeof(flag_sets[0]); ++i)
	{
		unsigned int flags = flag_sets[i];

		CHECK(doc.load(T("<?"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<??"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?>"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?#?>"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name>"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name ?"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name?"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name? "), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name?  "), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name "), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name  "), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name   "), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name value"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name value "), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?name value  "), flags).status == status_bad_pi);
	}
	
	CHECK(doc.load(T("<?xx#?>"), parse_minimal | parse_pi).status == status_bad_pi);
}

TEST(parse_comments_skip)
{
	xml_document doc;
	CHECK(doc.load(T("<!----><!--value-->"), parse_minimal));
	CHECK(!doc.first_child());
}

TEST(parse_comments_parse)
{
	xml_document doc;
	CHECK(doc.load(T("<!----><!--value-->"), parse_minimal | parse_comments));

	xml_node c1 = doc.first_child();
	xml_node c2 = doc.last_child();

	CHECK(c1 != c2);
	CHECK(c1.type() == node_comment);
	CHECK_STRING(c1.name(), T(""));
	CHECK_STRING(c1.value(), T(""));
	CHECK(c2.type() == node_comment);
	CHECK_STRING(c2.name(), T(""));
	CHECK_STRING(c2.value(), T("value"));
}

TEST(parse_comments_parse_no_eol)
{
	xml_document doc;
	CHECK(doc.load(T("<!--\r\rval1\rval2\r\nval3\nval4\r\r-->"), parse_minimal | parse_comments));

	xml_node c = doc.first_child();
	CHECK(c.type() == node_comment);
	CHECK_STRING(c.value(), T("\r\rval1\rval2\r\nval3\nval4\r\r"));
}

TEST(parse_comments_parse_eol)
{
	xml_document doc;
	CHECK(doc.load(T("<!--\r\rval1\rval2\r\nval3\nval4\r\r-->"), parse_minimal | parse_comments | parse_eol));

	xml_node c = doc.first_child();
	CHECK(c.type() == node_comment);
	CHECK_STRING(c.value(), T("\n\nval1\nval2\nval3\nval4\n\n"));
}

TEST(parse_comments_error)
{
	xml_document doc;

	unsigned int flag_sets[] = {parse_minimal, parse_minimal | parse_comments, parse_minimal | parse_comments | parse_eol};

	for (unsigned int i = 0; i < sizeof(flag_sets) / sizeof(flag_sets[0]); ++i)
	{
		unsigned int flags = flag_sets[i];

		CHECK(doc.load(T("<!-"), flags).status == status_bad_comment);
		CHECK(doc.load(T("<!--"), flags).status == status_bad_comment);
		CHECK(doc.load(T("<!--v"), flags).status == status_bad_comment);
		CHECK(doc.load(T("<!-->"), flags).status == status_bad_comment);
		CHECK(doc.load(T("<!--->"), flags).status == status_bad_comment);
		CHECK(doc.load(T("<!-- <!-- --><!- -->"), flags).status == status_bad_comment);
	}
}

TEST(parse_cdata_skip)
{
	xml_document doc;
	CHECK(doc.load(T("<![CDATA[]]><![CDATA[value]]>"), parse_minimal));
	CHECK(!doc.first_child());
}

TEST(parse_cdata_parse)
{
	xml_document doc;
	CHECK(doc.load(T("<![CDATA[]]><![CDATA[value]]>"), parse_minimal | parse_cdata));

	xml_node c1 = doc.first_child();
	xml_node c2 = doc.last_child();

	CHECK(c1 != c2);
	CHECK(c1.type() == node_cdata);
	CHECK_STRING(c1.name(), T(""));
	CHECK_STRING(c1.value(), T(""));
	CHECK(c2.type() == node_cdata);
	CHECK_STRING(c2.name(), T(""));
	CHECK_STRING(c2.value(), T("value"));
}

TEST(parse_cdata_parse_no_eol)
{
	xml_document doc;
	CHECK(doc.load(T("<![CDATA[\r\rval1\rval2\r\nval3\nval4\r\r]]>"), parse_minimal | parse_cdata));

	xml_node c = doc.first_child();
	CHECK(c.type() == node_cdata);
	CHECK_STRING(c.value(), T("\r\rval1\rval2\r\nval3\nval4\r\r"));
}

TEST(parse_cdata_parse_eol)
{
	xml_document doc;
	CHECK(doc.load(T("<![CDATA[\r\rval1\rval2\r\nval3\nval4\r\r]]>"), parse_minimal | parse_cdata | parse_eol));

	xml_node c = doc.first_child();
	CHECK(c.type() == node_cdata);
	CHECK_STRING(c.value(), T("\n\nval1\nval2\nval3\nval4\n\n"));
}

TEST(parse_cdata_error)
{
	xml_document doc;

	unsigned int flag_sets[] = {parse_minimal, parse_minimal | parse_cdata, parse_minimal | parse_cdata | parse_eol};

	for (unsigned int i = 0; i < sizeof(flag_sets) / sizeof(flag_sets[0]); ++i)
	{
		unsigned int flags = flag_sets[i];

		CHECK(doc.load(T("<!["), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![C"), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![CD"), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![CDA"), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![CDAT"), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![CDATA"), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![CDATA["), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![CDATA[]"), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![CDATA[data"), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![CDATA[data]"), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![CDATA[data]]"), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![CDATA[>"), flags).status == status_bad_cdata);
		CHECK(doc.load(T("<![CDATA[ <![CDATA[]]><![CDATA ]]>"), flags).status == status_bad_cdata);
	}
}

TEST(parse_ws_pcdata_skip)
{
	xml_document doc;
	CHECK(doc.load(T("  "), parse_minimal));
	CHECK(!doc.first_child());

	CHECK(doc.load(T("<root>  <node>  </node>  </root>"), parse_minimal));
	
	xml_node root = doc.child(T("root"));
	
	CHECK(root.first_child() == root.last_child());
	CHECK(!root.first_child().first_child());
}

TEST(parse_ws_pcdata_parse)
{
	xml_document doc;
	CHECK(doc.load(T("<root>  <node>  </node>  </root>"), parse_minimal | parse_ws_pcdata));

	xml_node root = doc.child(T("root"));

	xml_node c1 = root.first_child();
	xml_node c2 = c1.next_sibling();
	xml_node c3 = c2.next_sibling();

	CHECK(c3 == root.last_child());

	CHECK(c1.type() == node_pcdata);
	CHECK_STRING(c1.value(), T("  "));
	CHECK(c3.type() == node_pcdata);
	CHECK_STRING(c3.value(), T("  "));

	CHECK(c2.first_child() == c2.last_child());
	CHECK(c2.first_child().type() == node_pcdata);
	CHECK_STRING(c2.first_child().value(), T("  "));
}

TEST(parse_pcdata_no_eol)
{
	xml_document doc;
	CHECK(doc.load(T("<root>\r\rval1\rval2\r\nval3\nval4\r\r</root>"), parse_minimal));

	CHECK_STRING(doc.child_value(T("root")), T("\r\rval1\rval2\r\nval3\nval4\r\r"));
}

TEST(parse_pcdata_eol)
{
	xml_document doc;
	CHECK(doc.load(T("<root>\r\rval1\rval2\r\nval3\nval4\r\r</root>"), parse_minimal | parse_eol));

	CHECK_STRING(doc.child_value(T("root")), T("\n\nval1\nval2\nval3\nval4\n\n"));
}

TEST(parse_pcdata_skip_ext)
{
	xml_document doc;
	CHECK(doc.load(T("pre<root/>post"), parse_minimal));
	CHECK(doc.first_child() == doc.last_child());
	CHECK(doc.first_child().type() == node_element);
}

TEST(parse_pcdata_error)
{
	xml_document doc;
	CHECK(doc.load(T("<root>pcdata"), parse_minimal).status == status_end_element_mismatch);
}

TEST(parse_escapes_skip)
{
	xml_document doc;
	CHECK(doc.load(T("<node id='&lt;&gt;&amp;&apos;&quot;'>&lt;&gt;&amp;&apos;&quot;</node>"), parse_minimal));
	CHECK_STRING(doc.child(T("node")).attribute(T("id")).value(), T("&lt;&gt;&amp;&apos;&quot;"));
}

TEST(parse_escapes_parse)
{
	xml_document doc;
	CHECK(doc.load(T("<node id='&lt;&gt;&amp;&apos;&quot;'>&lt;&gt;&amp;&apos;&quot;</node>"), parse_minimal | parse_escapes));
	CHECK_STRING(doc.child_value(T("node")), T("<>&'\""));
	CHECK_STRING(doc.child(T("node")).attribute(T("id")).value(), T("<>&'\""));
}

TEST(parse_escapes_code)
{
	xml_document doc;
	CHECK(doc.load(T("<node>&#1;&#32;&#x20;</node>"), parse_minimal | parse_escapes));
	CHECK_STRING(doc.child_value(T("node")), T("\01  "));
}

TEST(parse_escapes_unicode)
{
	xml_document doc;
	CHECK(doc.load(T("<node>&#x03B3;&#x03b3;</node>"), parse_minimal | parse_escapes));

#ifdef PUGIXML_WCHAR_MODE
	CHECK_STRING(doc.child_value(T("node")), L"\x3b3\x3b3");
#else
	CHECK_STRING(doc.child_value(T("node")), "\xce\xb3\xce\xb3");
#endif
}

TEST(parse_escapes_error)
{
	xml_document doc;
	CHECK(doc.load(T("<node>&#x03g;&#ab;&quot</node>"), parse_minimal | parse_escapes));
	CHECK_STRING(doc.child_value(T("node")), T("&#x03g;&#ab;&quot"));

	CHECK(!doc.load(T("<node id='&#x12")));
	CHECK(!doc.load(T("<node id='&g")));
	CHECK(!doc.load(T("<node id='&gt")));
	CHECK(!doc.load(T("<node id='&l")));
	CHECK(!doc.load(T("<node id='&lt")));
	CHECK(!doc.load(T("<node id='&a")));
	CHECK(!doc.load(T("<node id='&amp")));
	CHECK(!doc.load(T("<node id='&apos")));
}

TEST(parse_attribute_spaces)
{
	xml_document doc;
	CHECK(doc.load(T("<node id1='v1' id2 ='v2' id3= 'v3' id4 = 'v4' id5 \n\r\t = \r\t\n 'v5' />"), parse_minimal));
	CHECK_STRING(doc.child(T("node")).attribute(T("id1")).value(), T("v1"));
	CHECK_STRING(doc.child(T("node")).attribute(T("id2")).value(), T("v2"));
	CHECK_STRING(doc.child(T("node")).attribute(T("id3")).value(), T("v3"));
	CHECK_STRING(doc.child(T("node")).attribute(T("id4")).value(), T("v4"));
	CHECK_STRING(doc.child(T("node")).attribute(T("id5")).value(), T("v5"));
}

TEST(parse_attribute_quot)
{
	xml_document doc;
	CHECK(doc.load(T("<node id1='v1' id2=\"v2\"/>"), parse_minimal));
	CHECK_STRING(doc.child(T("node")).attribute(T("id1")).value(), T("v1"));
	CHECK_STRING(doc.child(T("node")).attribute(T("id2")).value(), T("v2"));
}

TEST(parse_attribute_no_eol_no_wconv)
{
	xml_document doc;
	CHECK(doc.load(T("<node id=' \t\r\rval1  \rval2\r\nval3\nval4\r\r'/>"), parse_minimal));
	CHECK_STRING(doc.child(T("node")).attribute(T("id")).value(), T(" \t\r\rval1  \rval2\r\nval3\nval4\r\r"));
}

TEST(parse_attribute_eol_no_wconv)
{
	xml_document doc;
	CHECK(doc.load(T("<node id=' \t\r\rval1  \rval2\r\nval3\nval4\r\r'/>"), parse_minimal | parse_eol));
	CHECK_STRING(doc.child(T("node")).attribute(T("id")).value(), T(" \t\n\nval1  \nval2\nval3\nval4\n\n"));
}

TEST(parse_attribute_no_eol_wconv)
{
	xml_document doc;
	CHECK(doc.load(T("<node id=' \t\r\rval1  \rval2\r\nval3\nval4\r\r'/>"), parse_minimal | parse_wconv_attribute));
	CHECK_STRING(doc.child(T("node")).attribute(T("id")).value(), T("    val1   val2  val3 val4  "));
}

TEST(parse_attribute_eol_wconv)
{
	xml_document doc;
	CHECK(doc.load(T("<node id=' \t\r\rval1  \rval2\r\nval3\nval4\r\r'/>"), parse_minimal | parse_eol | parse_wconv_attribute));
	CHECK_STRING(doc.child(T("node")).attribute(T("id")).value(), T("    val1   val2 val3 val4  "));
}

TEST(parse_attribute_wnorm)
{
	xml_document doc;

	for (int eol = 0; eol < 2; ++eol)
		for (int wconv = 0; wconv < 2; ++wconv)
		{
			unsigned int flags = parse_minimal | parse_wnorm_attribute | (eol ? parse_eol : 0) | (wconv ? parse_wconv_attribute : 0);
			CHECK(doc.load(T("<node id=' \t\r\rval1  \rval2\r\nval3\nval4\r\r'/>"), flags));
			CHECK_STRING(doc.child(T("node")).attribute(T("id")).value(), T("val1 val2 val3 val4"));
		}
}

TEST(parse_attribute_variations)
{
	xml_document doc;

	for (int wnorm = 0; wnorm < 2; ++wnorm)
		for (int eol = 0; eol < 2; ++eol)
			for (int wconv = 0; wconv < 2; ++wconv)
				for (int escapes = 0; escapes < 2; ++escapes)
				{
					unsigned int flags = parse_minimal;
					
					 flags |= (wnorm ? parse_wnorm_attribute : 0);
					 flags |= (eol ? parse_eol : 0);
					 flags |= (wconv ? parse_wconv_attribute : 0);
					 flags |= (escapes ? parse_escapes : 0);

					CHECK(doc.load(T("<node id='1'/>"), flags));
					CHECK_STRING(doc.child(T("node")).attribute(T("id")).value(), T("1"));
				}
}

TEST(parse_attribute_error)
{
	xml_document doc;
	CHECK(doc.load(T("<node id"), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id "), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id  "), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id   "), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id/"), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id/>"), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id?/>"), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id=/>"), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id='/>"), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id=\"/>"), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id=\"'/>"), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id='\"/>"), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node id='\"/>"), parse_minimal).status == status_bad_attribute);
	CHECK(doc.load(T("<node #/>"), parse_minimal).status == status_bad_start_element);
	CHECK(doc.load(T("<node#/>"), parse_minimal).status == status_bad_start_element);
	CHECK(doc.load(T("<node id1='1'id2='2'/>"), parse_minimal).status == status_bad_attribute);
}

TEST(parse_tag_single)
{
	xml_document doc;
	CHECK(doc.load(T("<node/><node /><node\n/>"), parse_minimal));
	CHECK_NODE(doc, T("<node /><node /><node />"));
}

TEST(parse_tag_hierarchy)
{
	xml_document doc;
	CHECK(doc.load(T("<node><n1><n2/></n1><n3><n4><n5></n5></n4></n3 \r\n></node>"), parse_minimal));
	CHECK_NODE(doc, T("<node><n1><n2 /></n1><n3><n4><n5 /></n4></n3></node>"));
}

TEST(parse_tag_error)
{
	xml_document doc;
	CHECK(doc.load(T("<"), parse_minimal).status == status_unrecognized_tag);
	CHECK(doc.load(T("<!"), parse_minimal).status == status_unrecognized_tag);
	CHECK(doc.load(T("<!D"), parse_minimal).status == status_unrecognized_tag);
	CHECK(doc.load(T("<#"), parse_minimal).status == status_unrecognized_tag);
	CHECK(doc.load(T("<node#"), parse_minimal).status == status_bad_start_element);
	CHECK(doc.load(T("<node"), parse_minimal).status == status_bad_start_element);
	CHECK(doc.load(T("<node/"), parse_minimal).status == status_bad_start_element);
	CHECK(doc.load(T("<node /"), parse_minimal).status == status_bad_start_element);
	CHECK(doc.load(T("<node / "), parse_minimal).status == status_bad_start_element);
	CHECK(doc.load(T("<node / >"), parse_minimal).status == status_bad_start_element);
	CHECK(doc.load(T("<node/ >"), parse_minimal).status == status_bad_start_element);
	CHECK(doc.load(T("</ node>"), parse_minimal).status == status_end_element_mismatch);
	CHECK(doc.load(T("</node"), parse_minimal).status == status_end_element_mismatch);
	CHECK(doc.load(T("</node "), parse_minimal).status == status_end_element_mismatch);
	CHECK(doc.load(T("<node></ node>"), parse_minimal).status == status_end_element_mismatch);
	CHECK(doc.load(T("<node></node"), parse_minimal).status == status_bad_end_element);
	CHECK(doc.load(T("<node></node "), parse_minimal).status == status_bad_end_element);
	CHECK(doc.load(T("<node></nodes>"), parse_minimal).status == status_end_element_mismatch);
	CHECK(doc.load(T("<node>"), parse_minimal).status == status_end_element_mismatch);
	CHECK(doc.load(T("<node/><"), parse_minimal).status == status_unrecognized_tag);
	CHECK(doc.load(T("<node attr='value'>"), parse_minimal).status == status_end_element_mismatch);
	CHECK(doc.load(T("</></node>"), parse_minimal).status == status_end_element_mismatch);
	CHECK(doc.load(T("</node>"), parse_minimal).status == status_end_element_mismatch);
	CHECK(doc.load(T("</>"), parse_minimal).status == status_end_element_mismatch);
	CHECK(doc.load(T("<node></node v>"), parse_minimal).status == status_bad_end_element);
}

TEST(parse_declaration_cases)
{
	xml_document doc;
	CHECK(doc.load(T("<?xml?><?xmL?><?xMl?><?xML?><?Xml?><?XmL?><?XMl?><?XML?>"), parse_minimal | parse_pi));
	CHECK(!doc.first_child());
}

TEST(parse_declaration_attr_cases)
{
	xml_document doc;
	CHECK(doc.load(T("<?xml ?><?xmL ?><?xMl ?><?xML ?><?Xml ?><?XmL ?><?XMl ?><?XML ?>"), parse_minimal | parse_pi));
	CHECK(!doc.first_child());
}

TEST(parse_declaration_skip)
{
	xml_document doc;
	CHECK(doc.load(T("<?xml?><?xml version='1.0'?>"), parse_minimal));
	CHECK(!doc.first_child());
}

TEST(parse_declaration_parse)
{
	xml_document doc;
	CHECK(doc.load(T("<?xml?><?xml version='1.0'?>"), parse_minimal | parse_declaration));

	xml_node d1 = doc.first_child();
	xml_node d2 = doc.last_child();

	CHECK(d1 != d2);
	CHECK(d1.type() == node_declaration);
	CHECK_STRING(d1.name(), T("xml"));
	CHECK(d2.type() == node_declaration);
	CHECK_STRING(d2.name(), T("xml"));
	CHECK_STRING(d2.attribute(T("version")).value(), T("1.0"));
}

TEST(parse_declaration_error)
{
	xml_document doc;

	unsigned int flag_sets[] = {parse_minimal, parse_minimal | parse_declaration};

	for (unsigned int i = 0; i < sizeof(flag_sets) / sizeof(flag_sets[0]); ++i)
	{
		unsigned int flags = flag_sets[i];

		CHECK(doc.load(T("<?xml"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?xml?"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?xml>"), flags).status == status_bad_pi);
		CHECK(doc.load(T("<?xml version='1>"), flags).status == status_bad_pi);
	}
	
	CHECK(doc.load(T("<?xml version='1?>"), parse_minimal | parse_declaration).status == status_bad_attribute);
}

TEST(parse_doctype_skip)
{
	xml_document doc;
	CHECK(doc.load(T("<!DOCTYPE doc>")) && !doc.first_child());
	CHECK(doc.load(T("<!DOCTYPE doc SYSTEM 'foo'>")) && !doc.first_child());
	CHECK(doc.load(T("<!DOCTYPE doc SYSTEM \"foo\">")) && !doc.first_child());
	CHECK(doc.load(T("<!DOCTYPE doc PUBLIC \"foo\" 'bar'>")) && !doc.first_child());
	CHECK(doc.load(T("<!DOCTYPE doc PUBLIC \"foo'\">")) && !doc.first_child());
	CHECK(doc.load(T("<!DOCTYPE doc SYSTEM 'foo' [<!ELEMENT foo 'ANY'>]>")) && !doc.first_child());

	CHECK(doc.load(T("<!DOCTYPE doc SYSTEM 'foo' [<!ELEMENT foo 'ANY'>]><node/>")));
	CHECK_NODE(doc, T("<node />"));
}

TEST(parse_doctype_error)
{
	xml_document doc;
	CHECK(doc.load(T("<!DOCTYPE")).status == status_bad_doctype);
	CHECK(doc.load(T("<!DOCTYPE doc")).status == status_bad_doctype);
	CHECK(doc.load(T("<!DOCTYPE doc SYSTEM 'foo")).status == status_bad_doctype);
	CHECK(doc.load(T("<!DOCTYPE doc SYSTEM \"foo")).status == status_bad_doctype);
	CHECK(doc.load(T("<!DOCTYPE doc PUBLIC \"foo\" 'bar")).status == status_bad_doctype);
	CHECK(doc.load(T("<!DOCTYPE doc PUBLIC \"foo'\"")).status == status_bad_doctype);
	CHECK(doc.load(T("<!DOCTYPE doc SYSTEM 'foo' [<!ELEMENT foo 'ANY")).status == status_bad_doctype);
	CHECK(doc.load(T("<!DOCTYPE doc SYSTEM 'foo' [<!ELEMENT foo 'ANY'>")).status == status_bad_doctype);
	CHECK(doc.load(T("<!DOCTYPE doc SYSTEM 'foo' [<!ELEMENT foo 'ANY'>]")).status == status_bad_doctype);
	CHECK(doc.load(T("<!DOCTYPE doc SYSTEM 'foo' [<!ELEMENT foo 'ANY'>] ")).status == status_bad_doctype);
}

TEST(parse_empty)
{
	xml_document doc;
	CHECK(doc.load(T("")) && !doc.first_child());
}
