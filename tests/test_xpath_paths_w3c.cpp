#ifndef PUGIXML_NO_XPATH

#include "common.hpp"

TEST_XML(xpath_paths_w3c_1, "<node><para/><foo/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::para"));
	CHECK_XPATH_NODESET(n, T("child::para")) % 3 % 5;
}

TEST_XML(xpath_paths_w3c_2, "<node><para/><foo/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::*"));
	CHECK_XPATH_NODESET(n, T("child::*")) % 3 % 4 % 5;
}

TEST_XML(xpath_paths_w3c_3, "<node>pcdata<child/><![CDATA[cdata]]></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::text()"));
	CHECK_XPATH_NODESET(n, T("child::text()")) % 3 % 5;
}

TEST_XML(xpath_paths_w3c_4, "<node>pcdata<child/><![CDATA[cdata]]></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::node()"));
	CHECK_XPATH_NODESET(n, T("child::node()")) % 3 % 4 % 5;
}

TEST_XML(xpath_paths_w3c_5, "<node name='value' foo='bar' />")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("attribute::name"));
	CHECK_XPATH_NODESET(n, T("attribute::name")) % 3;
}

TEST_XML(xpath_paths_w3c_6, "<node name='value' foo='bar' />")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("attribute::*"));
	CHECK_XPATH_NODESET(n, T("attribute::*")) % 3 % 4;
}

TEST_XML(xpath_paths_w3c_7, "<node><para><para/><para/><foo><para/></foo></para><foo/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("descendant::para"));
	CHECK_XPATH_NODESET(n, T("descendant::para")) % 3 % 4 % 5 % 7 % 9;
	CHECK_XPATH_NODESET(n.child(T("para")), T("descendant::para")) % 4 % 5 % 7;
}

TEST_XML(xpath_paths_w3c_8, "<node><div><font><div><div/></div></font></div></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("ancestor::div"));
	CHECK_XPATH_NODESET(n.child(T("div")).child(T("font")).child(T("div")).child(T("div")), T("ancestor::div")) % 5 % 3;
}

TEST_XML(xpath_paths_w3c_9, "<node><div><font><div><div/></div></font></div></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("ancestor-or-self::div"));
	CHECK_XPATH_NODESET(n.child(T("div")).child(T("font")).child(T("div")).child(T("div")), T("ancestor-or-self::div")) % 6 % 5 % 3;
}

TEST_XML(xpath_paths_w3c_10, "<node><para><para/><para/><foo><para/></foo></para><foo/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("descendant-or-self::para"));
	CHECK_XPATH_NODESET(n, T("descendant-or-self::para")) % 3 % 4 % 5 % 7 % 9;
	CHECK_XPATH_NODESET(n.child(T("para")), T("descendant-or-self::para")) % 3 % 4 % 5 % 7;
}

TEST_XML(xpath_paths_w3c_11, "<node><para><para/><para/><foo><para/></foo></para><foo/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("self::para"));
	CHECK_XPATH_NODESET(n, T("self::para"));
	CHECK_XPATH_NODESET(n.child(T("para")), T("self::para")) % 3;
}

TEST_XML(xpath_paths_w3c_12, "<chapter><para><para/><para/><foo><para/></foo></para><foo/><para/></chapter>")
{
	doc.precompute_document_order();

	xml_node c;

	CHECK_XPATH_NODESET(c, T("child::chapter/descendant::para"));
	CHECK_XPATH_NODESET(doc, T("child::chapter/descendant::para")) % 3 % 4 % 5 % 7 % 9;
}

TEST_XML(xpath_paths_w3c_13, "<node><para><para/><para/><foo><para/></foo></para><foo/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;

	CHECK_XPATH_NODESET(c, T("child::*/child::para"));
	CHECK_XPATH_NODESET(doc, T("child::*/child::para")) % 3 % 9;
}

TEST_XML(xpath_paths_w3c_14, "<node><para><para/><para/><foo><para/></foo></para><foo/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("/"));

	CHECK_XPATH_NODESET(doc, T("/")) % 1;
	CHECK_XPATH_NODESET(n, T("/")) % 1;
	CHECK_XPATH_NODESET(n.child(T("para")), T("/")) % 1;
}

TEST_XML(xpath_paths_w3c_15, "<node><para><para/><para/><foo><para/></foo></para><foo/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("/descendant::para"));
	CHECK_XPATH_NODESET(n, T("/descendant::para")) % 3 % 4 % 5 % 7 % 9;
	CHECK_XPATH_NODESET(n.child(T("para")), T("/descendant::para")) % 3 % 4 % 5 % 7 % 9;
}

TEST_XML(xpath_paths_w3c_16, "<node><olist><item/></olist><item/><olist><olist><item/><item/></olist></olist></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("/descendant::olist/child::item"));
	CHECK_XPATH_NODESET(n, T("/descendant::olist/child::item")) % 4 % 8 % 9;
	CHECK_XPATH_NODESET(n.child(T("olist")), T("/descendant::olist/child::item")) % 4 % 8 % 9;
}

TEST_XML(xpath_paths_w3c_17, "<node><para/><para/><para/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::para[position()=1]"));
	CHECK_XPATH_NODESET(n, T("child::para[position()=1]")) % 3;
}

TEST_XML(xpath_paths_w3c_18, "<node><para/><para/><para/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::para[position()=last()]"));
	CHECK_XPATH_NODESET(n, T("child::para[position()=last()]")) % 6;
}

TEST_XML(xpath_paths_w3c_19, "<node><para/><para/><para/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::para[position()=last()-1]"));
	CHECK_XPATH_NODESET(n, T("child::para[position()=last()-1]")) % 5;
}

TEST_XML(xpath_paths_w3c_20, "<node><para/><para/><para/><para/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::para[position()>1]"));
	CHECK_XPATH_NODESET(n, T("child::para[position()>1]")) % 4 % 5 % 6;
}

TEST_XML(xpath_paths_w3c_21, "<node><chapter/><chapter/><chapter/><chapter/><chapter/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node")).child(T("chapter")).next_sibling().next_sibling();

	CHECK_XPATH_NODESET(c, T("following-sibling::chapter[position()=1]"));
	CHECK_XPATH_NODESET(n, T("following-sibling::chapter[position()=1]")) % 6;
}

TEST_XML(xpath_paths_w3c_22, "<node><chapter/><chapter/><chapter/><chapter/><chapter/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node")).child(T("chapter")).next_sibling().next_sibling();

	CHECK_XPATH_NODESET(c, T("preceding-sibling::chapter[position()=1]"));
	CHECK_XPATH_NODESET(n, T("preceding-sibling::chapter[position()=1]")) % 4;
}

TEST_XML(xpath_paths_w3c_23, "<node><figure><figure/><figure/><foo><figure/></foo></figure><foo/><figure/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("/descendant::figure[position()=4]"));
	CHECK_XPATH_NODESET(n, T("/descendant::figure[position()=4]")) % 7;
	CHECK_XPATH_NODESET(n.child(T("figure")), T("/descendant::figure[position()=4]")) % 7;
}

TEST_XML(xpath_paths_w3c_24, "<doc><chapter/><chapter/><chapter/><chapter/><chapter><section/><section/><section/></chapter><chapter/></doc>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("doc")).child(T("chapter"));

	CHECK_XPATH_NODESET(c, T("/child::doc/child::chapter[position()=5]/child::section[position()=2]"));
	CHECK_XPATH_NODESET(n, T("/child::doc/child::chapter[position()=5]/child::section[position()=2]")) % 9;
	CHECK_XPATH_NODESET(doc, T("/child::doc/child::chapter[position()=5]/child::section[position()=2]")) % 9;
}

TEST_XML(xpath_paths_w3c_25, "<node><para/><para type='warning'/><para type='warning'/><para/><para type='error'/><para type='warning'/><para type='warning'/><para type='warning'/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::para[attribute::type=\"warning\"]"));
	CHECK_XPATH_NODESET(n, T("child::para[attribute::type=\"warning\"]")) % 4 % 6 % 11 % 13 % 15;
}

TEST_XML(xpath_paths_w3c_26, "<node><para/><para type='warning'/><para type='warning'/><para/><para type='error'/><para type='warning'/><para type='warning'/><para type='warning'/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::para[attribute::type=\"warning\"][position()=5]"));
	CHECK_XPATH_NODESET(n, T("child::para[attribute::type=\"warning\"][position()=5]")) % 15;
}

TEST_XML(xpath_paths_w3c_27a, "<node><para/><para type='warning'/><para type='warning'/><para/><para type='error'/><para type='warning'/><para type='warning'/><para type='warning'/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::para[position()=5][attribute::type=\"warning\"]"));
	CHECK_XPATH_NODESET(n, T("child::para[position()=5][attribute::type=\"warning\"]"));
}

TEST_XML(xpath_paths_w3c_27b, "<node><para/><para type='warning'/><para type='warning'/><para/><para type='warning'/><para type='warning'/><para type='warning'/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::para[position()=5][attribute::type=\"warning\"]"));
	CHECK_XPATH_NODESET(n, T("child::para[position()=5][attribute::type=\"warning\"]")) % 9;
}

TEST_XML(xpath_paths_w3c_28, "<node><chapter><title>foo</title></chapter><chapter><title>Introduction</title></chapter><chapter><title>introduction</title></chapter><chapter/><chapter><title>Introduction</title><title>foo</title></chapter></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::chapter[child::title='Introduction']"));
	CHECK_XPATH_NODESET(n, T("child::chapter[child::title='Introduction']")) % 6 % 13;
}

TEST_XML(xpath_paths_w3c_29, "<node><chapter><title>foo</title></chapter><chapter><title>Introduction</title></chapter><chapter><title>introduction</title></chapter><chapter/><chapter><title>Introduction</title><title>foo</title></chapter></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::chapter[child::title]"));
	CHECK_XPATH_NODESET(n, T("child::chapter[child::title]")) % 3 % 6 % 9 % 13;
}

TEST_XML(xpath_paths_w3c_30, "<node><abstract/><chapter/><chapter/><references/><appendix/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::*[self::chapter or self::appendix]"));
	CHECK_XPATH_NODESET(n, T("child::*[self::chapter or self::appendix]")) % 4 % 5 % 7;
}

TEST_XML(xpath_paths_w3c_31a, "<node><abstract/><chapter/><chapter/><references/><appendix/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::*[self::chapter or self::appendix][position()=last()]"));
	CHECK_XPATH_NODESET(n, T("child::*[self::chapter or self::appendix][position()=last()]")) % 7;
}

TEST_XML(xpath_paths_w3c_31b, "<node><abstract/><chapter/><chapter/><references/><appendix/><chapter/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child::*[self::chapter or self::appendix][position()=last()]"));
	CHECK_XPATH_NODESET(n, T("child::*[self::chapter or self::appendix][position()=last()]")) % 8;
}

#endif
