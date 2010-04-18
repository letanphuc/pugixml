#ifndef PUGIXML_NO_XPATH

#include "common.hpp"

TEST_XML(xpath_paths_axes_child, "<node attr='value'><child attr='value'><subchild/></child><another/><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("child:: node()"));

	CHECK_XPATH_NODESET(n, T("child:: node()")) % 4 % 7 % 8; // child, another, last
	CHECK_XPATH_NODESET(n, T("another/child:: node()"));
}

TEST_XML(xpath_paths_axes_descendant, "<node attr='value'><child attr='value'><subchild/></child><another><subchild/></another><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("descendant:: node()"));

	CHECK_XPATH_NODESET(n, T("descendant:: node()")) % 4 % 6 % 7 % 8 % 9; // child, subchild, another, subchild, last
	CHECK_XPATH_NODESET(doc, T("descendant:: node()")) % 2 % 4 % 6 % 7 % 8 % 9; // node, child, subchild, another, subchild, last
	CHECK_XPATH_NODESET(n, T("another/descendant:: node()")) % 8; // subchild
	CHECK_XPATH_NODESET(n, T("last/descendant:: node()"));
}

TEST_XML(xpath_paths_axes_parent, "<node attr='value'><child attr='value'><subchild/></child><another><subchild/></another><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("parent:: node()"));

	CHECK_XPATH_NODESET(n.child(T("child")), T("parent:: node()")) % 2; // node
	CHECK_XPATH_NODESET(n, T("child/subchild/parent:: node()")) % 4; // child
	CHECK_XPATH_NODESET(n, T("@attr/parent:: node()")) % 2; // node
	CHECK_XPATH_NODESET(n, T("parent:: node()")) % 1; // root
	CHECK_XPATH_NODESET(doc, T("parent:: node()"));
}

TEST_XML(xpath_paths_axes_ancestor, "<node attr='value'><child attr='value'><subchild/></child><another><subchild/></another><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("ancestor:: node()"));

	CHECK_XPATH_NODESET(n.child(T("child")), T("ancestor:: node()")) % 2 % 1; // node, root
	CHECK_XPATH_NODESET(n, T("child/subchild/ancestor:: node()")) % 4 % 2 % 1; // child, node, root
	CHECK_XPATH_NODESET(n, T("child/@attr/ancestor:: node()")) % 4 % 2 % 1; // child, node, root
	CHECK_XPATH_NODESET(n, T("ancestor:: node()")) % 1; // root
	CHECK_XPATH_NODESET(doc, T("ancestor:: node()"));
}

TEST_XML(xpath_paths_axes_following_sibling, "<node attr1='value' attr2='value'><child attr='value'><subchild/></child><another><subchild/></another><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("following-sibling:: node()"));

	CHECK_XPATH_NODESET(n.child(T("child")), T("following-sibling:: node()")) % 8 % 10; // another, last
	CHECK_XPATH_NODESET(n.child(T("last")), T("following-sibling:: node()"));
	CHECK_XPATH_NODESET(n, T("@attr1/following-sibling:: node()")); // attributes are not siblings
}

TEST_XML(xpath_paths_axes_preceding_sibling, "<node attr1='value' attr2='value'><child attr='value'><subchild/></child><another><subchild/></another><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("preceding-sibling:: node()"));

	CHECK_XPATH_NODESET(n.child(T("child")), T("preceding-sibling:: node()"));
	CHECK_XPATH_NODESET(n.child(T("last")), T("preceding-sibling:: node()")) % 8 % 5; // another, child
	CHECK_XPATH_NODESET(n, T("@attr2/following-sibling:: node()")); // attributes are not siblings
}

TEST_XML(xpath_paths_axes_following, "<node attr1='value' attr2='value'><child attr='value'><subchild/></child><another><subchild/></another><almost/><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("following:: node()"));

	CHECK_XPATH_NODESET(n, T("following:: node()")); // no descendants
	CHECK_XPATH_NODESET(n.child(T("child")), T("following:: node()")) % 8 % 9 % 10 % 11; // another, subchild, almost, last
	CHECK_XPATH_NODESET(n.child(T("child")).child(T("subchild")), T("following:: node()")) % 8 % 9 % 10 % 11; // another, subchild, almost, last
	CHECK_XPATH_NODESET(n.child(T("last")), T("following:: node()"));
}

TEST_XML(xpath_paths_axes_preceding, "<node attr1='value' attr2='value'><child attr='value'><subchild/></child><another><subchild/></another><almost/><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("preceding:: node()"));

	CHECK_XPATH_NODESET(n.child(T("child")), T("preceding:: node()")); // no ancestors
	CHECK_XPATH_NODESET(n.child(T("last")), T("preceding:: node()")) % 10 % 9 % 8 % 7 % 5; // almost, subchild, another, subchild, child
	CHECK_XPATH_NODESET(n.child(T("another")).child(T("subchild")), T("preceding:: node()")) % 7 % 5; // subchild, child
	CHECK_XPATH_NODESET(n, T("preceding:: node()"));
}

TEST_XML(xpath_paths_axes_attribute, "<node attr1='value' attr2='value'><child attr='value'><subchild/></child><another xmlns:foo='bar'><subchild/></another><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("attribute:: node()"));

	CHECK_XPATH_NODESET(n.child(T("child")), T("attribute:: node()")) % 6; // child/@attr
	CHECK_XPATH_NODESET(n.child(T("last")), T("attribute:: node()"));
	CHECK_XPATH_NODESET(n, T("attribute:: node()")) % 3 % 4; // node/@attr1 node/@attr2
	CHECK_XPATH_NODESET(doc, T("descendant-or-self:: node()/attribute:: node()")) % 3 % 4 % 6; // all attributes
	CHECK_XPATH_NODESET(n.child(T("another")), T("attribute:: node()")); // namespace nodes are not attributes
}

TEST_XML(xpath_paths_axes_namespace, "<node xmlns:foo='bar'/>")
{
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(n, T("namespace:: node()")); // namespace nodes are not supported
}

TEST_XML(xpath_paths_axes_self, "<node attr='value'><child attr='value'><subchild/></child><another><subchild/></another><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("self:: node()"));

	CHECK_XPATH_NODESET(n.child(T("child")), T("self:: node()")) % 4; // child
	CHECK_XPATH_NODESET(n, T("self:: node()")) % 2; // node
	CHECK_XPATH_NODESET(n, T("child/self:: node()")) % 4; // child
	CHECK_XPATH_NODESET(n, T("child/@attr/self:: node()")) % 5; // @attr
	CHECK_XPATH_NODESET(doc, T("self:: node()")) % 1; // root
}

TEST_XML(xpath_paths_axes_descendant_or_self, "<node attr='value'><child attr='value'><subchild/></child><another><subchild/></another><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("descendant-or-self:: node()"));

	CHECK_XPATH_NODESET(n, T("descendant-or-self:: node()")) % 2 % 4 % 6 % 7 % 8 % 9; // node, child, subchild, another, subchild, last
	CHECK_XPATH_NODESET(doc, T("descendant-or-self:: node()")) % 1 % 2 % 4 % 6 % 7 % 8 % 9; // root, node, child, subchild, another, subchild, last
	CHECK_XPATH_NODESET(n, T("another/descendant-or-self:: node()")) % 7 % 8; // another, subchild
	CHECK_XPATH_NODESET(n, T("last/descendant-or-self:: node()")) % 9; // last
}

TEST_XML(xpath_paths_axes_ancestor_or_self, "<node attr='value'><child attr='value'><subchild/></child><another><subchild/></another><last/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("ancestor-or-self:: node()"));

	CHECK_XPATH_NODESET(n.child(T("child")), T("ancestor-or-self:: node()")) % 4 % 2 % 1; // child, node, root
	CHECK_XPATH_NODESET(n, T("child/subchild/ancestor-or-self:: node()")) % 6 % 4 % 2 % 1; // subchild, child, node, root
	CHECK_XPATH_NODESET(n, T("child/@attr/ancestor-or-self:: node()")) % 5 % 4 % 2 % 1; // @attr, child, node, root
	CHECK_XPATH_NODESET(n, T("ancestor-or-self:: node()")) % 2 % 1; // root, node
	CHECK_XPATH_NODESET(doc, T("ancestor-or-self:: node()")) % 1; // root
}

TEST_XML(xpath_paths_axes_abbrev, "<node attr='value'><foo/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	// @ axis
	CHECK_XPATH_NODESET(c, T("@attr"));
	CHECK_XPATH_NODESET(n, T("@attr")) % 3;

	// no axis - child implied
	CHECK_XPATH_NODESET(c, T("foo"));
	CHECK_XPATH_NODESET(n, T("foo")) % 4;
	CHECK_XPATH_NODESET(doc, T("node()")) % 2;

	// @ axis should disable all other axis specifiers
	CHECK_XPATH_FAIL(T("@child::foo"));
	CHECK_XPATH_FAIL(T("@attribute::foo"));
}

TEST_XML(xpath_paths_nodetest_all, "<node a1='v1' x:a2='v2'><c1/><x:c2/><c3/><x:c4/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("*"));
	CHECK_XPATH_NODESET(c, T("child::*"));

	CHECK_XPATH_NODESET(n, T("*")) % 5 % 6 % 7 % 8;
	CHECK_XPATH_NODESET(n, T("child::*")) % 5 % 6 % 7 % 8;
	CHECK_XPATH_NODESET(n, T("attribute::*")) % 3 % 4;
}

TEST_XML_FLAGS(xpath_paths_nodetest_name, "<node a1='v1' x:a2='v2'><c1/><x:c2/><c3/><x:c4/><?c1?></node>", parse_default | parse_pi)
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("c1"));
	CHECK_XPATH_NODESET(c, T("child::c1"));

	CHECK_XPATH_NODESET(n, T("c1")) % 5;
	CHECK_XPATH_NODESET(n, T("x:c2")) % 6;

	CHECK_XPATH_NODESET(n, T("child::c1")) % 5;
	CHECK_XPATH_NODESET(n, T("child::x:c2")) % 6;

	CHECK_XPATH_NODESET(n, T("attribute::a1")) % 3;
	CHECK_XPATH_NODESET(n, T("attribute::x:a2")) % 4;
	CHECK_XPATH_NODESET(n, T("@x:a2")) % 4;
}

TEST_XML(xpath_paths_nodetest_all_in_namespace, "<node a1='v1' x:a2='v2'><c1/><x:c2/><c3/><x:c4/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("x:*"));
	CHECK_XPATH_NODESET(c, T("child::x:*"));

	CHECK_XPATH_NODESET(n, T("x:*")) % 6 % 8;
	CHECK_XPATH_NODESET(n, T("child::x:*")) % 6 % 8;

	CHECK_XPATH_NODESET(n, T("attribute::x:*")) % 4;
	CHECK_XPATH_NODESET(n, T("@x:*")) % 4;

	CHECK_XPATH_FAIL(T(":*"));
	CHECK_XPATH_FAIL(T("@:*"));
}

TEST_XML_FLAGS(xpath_paths_nodetest_type, "<node attr='value'>pcdata<child/><?pi1 value?><?pi2 value?><!--comment--><![CDATA[cdata]]></node>", parse_default | parse_pi | parse_comments)
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	// check on empty nodes
	CHECK_XPATH_NODESET(c, T("node()"));
	CHECK_XPATH_NODESET(c, T("text()"));
	CHECK_XPATH_NODESET(c, T("comment()"));
	CHECK_XPATH_NODESET(c, T("processing-instruction()"));
	CHECK_XPATH_NODESET(c, T("processing-instruction('foobar')"));

	// child axis
	CHECK_XPATH_NODESET(n, T("node()")) % 4 % 5 % 6 % 7 % 8 % 9;
	CHECK_XPATH_NODESET(n, T("text()")) % 4 % 9;
	CHECK_XPATH_NODESET(n, T("comment()")) % 8;
	CHECK_XPATH_NODESET(n, T("processing-instruction()")) % 6 % 7;
	CHECK_XPATH_NODESET(n, T("processing-instruction('pi2')")) % 7;

	// attribute axis
	CHECK_XPATH_NODESET(n, T("@node()")) % 3;
	CHECK_XPATH_NODESET(n, T("@text()"));
	CHECK_XPATH_NODESET(n, T("@comment()"));
	CHECK_XPATH_NODESET(n, T("@processing-instruction()"));
	CHECK_XPATH_NODESET(n, T("@processing-instruction('pi2')"));

	// incorrect 'argument' number
	CHECK_XPATH_FAIL(T("node('')"));
	CHECK_XPATH_FAIL(T("text('')"));
	CHECK_XPATH_FAIL(T("comment('')"));
	CHECK_XPATH_FAIL(T("processing-instruction(1)"));
	CHECK_XPATH_FAIL(T("processing-instruction('', '')"));
}

TEST_XML(xpath_paths_absolute, "<node><foo><foo/><foo/></foo></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("/foo"));
	CHECK_XPATH_NODESET(n, T("/foo"));
	CHECK_XPATH_NODESET(n, T("/node/foo")) % 3;
	CHECK_XPATH_NODESET(n.child(T("foo")), T("/node/foo")) % 3;

	CHECK_XPATH_NODESET(c, T("/"));
	CHECK_XPATH_NODESET(n, T("/")) % 1;
	CHECK_XPATH_NODESET(n.child(T("foo")), T("/")) % 1;
}

TEST_XML(xpath_paths_step_abbrev, "<node><foo/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("."));
	CHECK_XPATH_NODESET(c, T(".."));

	CHECK_XPATH_NODESET(n, T(".")) % 2;
	CHECK_XPATH_NODESET(n, T("..")) % 1;
	CHECK_XPATH_NODESET(n, T("../node")) % 2;
	CHECK_XPATH_NODESET(n.child(T("foo")), T("..")) % 2;

	CHECK_XPATH_FAIL(T(".node"));
	CHECK_XPATH_FAIL(T("..node"));
}

TEST_XML(xpath_paths_relative_abbrev, "<node><foo><foo/><foo/></foo></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("foo//bar"));

	CHECK_XPATH_NODESET(n, T("foo/foo")) % 4 % 5;
	CHECK_XPATH_NODESET(n, T("foo//foo")) % 4 % 5;
	CHECK_XPATH_NODESET(n, T(".//foo")) % 3 % 4 % 5;
}

TEST_XML(xpath_paths_absolute_abbrev, "<node><foo><foo/><foo/></foo></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(c, T("//bar"));

	CHECK_XPATH_NODESET(n, T("//foo")) % 3 % 4 % 5;
	CHECK_XPATH_NODESET(n.child(T("foo")), T("//foo")) % 3 % 4 % 5;
	CHECK_XPATH_NODESET(doc, T("//foo")) % 3 % 4 % 5;
}

TEST_XML(xpath_paths_predicate_boolean, "<node><chapter/><chapter/><chapter/><chapter/><chapter/></node>")
{
	doc.precompute_document_order();

	xml_node n = doc.child(T("node")).child(T("chapter")).next_sibling().next_sibling();

	CHECK_XPATH_NODESET(n, T("following-sibling::chapter[position()=1]")) % 6;
	CHECK_XPATH_NODESET(n, T("following-sibling::chapter[position()=2]")) % 7;
	CHECK_XPATH_NODESET(n, T("preceding-sibling::chapter[position()=1]")) % 4;
	CHECK_XPATH_NODESET(n, T("preceding-sibling::chapter[position()=2]")) % 3;
}

TEST_XML(xpath_paths_predicate_number, "<node><chapter/><chapter/><chapter/><chapter/><chapter/></node>")
{
	doc.precompute_document_order();

	xml_node n = doc.child(T("node")).child(T("chapter")).next_sibling().next_sibling();

	CHECK_XPATH_NODESET(n, T("following-sibling::chapter[1]")) % 6;
	CHECK_XPATH_NODESET(n, T("following-sibling::chapter[2]")) % 7;
	CHECK_XPATH_NODESET(n, T("preceding-sibling::chapter[1]")) % 4;
	CHECK_XPATH_NODESET(n, T("preceding-sibling::chapter[2]")) % 3;
}

TEST_XML(xpath_paths_predicate_several, "<node><employee/><employee secretary=''/><employee assistant=''/><employee secretary='' assistant=''/><employee assistant='' secretary=''/></node>")
{
	doc.precompute_document_order();

	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(n, T("employee")) % 3 % 4 % 6 % 8 % 11;
	CHECK_XPATH_NODESET(n, T("employee[@secretary]")) % 4 % 8 % 11;
	CHECK_XPATH_NODESET(n, T("employee[@assistant]")) % 6 % 8 % 11;
	CHECK_XPATH_NODESET(n, T("employee[@secretary][@assistant]")) % 8 % 11;
	CHECK_XPATH_NODESET(n, T("employee[@assistant][@secretary]")) % 8 % 11;
	CHECK_XPATH_NODESET(n, T("employee[@secretary and @assistant]")) % 8 % 11;
}

TEST_XML(xpath_paths_predicate_filter_boolean, "<node><chapter/><chapter/><chapter/><chapter/><chapter/></node>")
{
	doc.precompute_document_order();

	xml_node n = doc.child(T("node")).child(T("chapter")).next_sibling().next_sibling();

	CHECK_XPATH_NODESET(n, T("(following-sibling::chapter)[position()=1]")) % 6;
	CHECK_XPATH_NODESET(n, T("(following-sibling::chapter)[position()=2]")) % 7;
	CHECK_XPATH_NODESET(n, T("(preceding-sibling::chapter)[position()=1]")) % 3;
	CHECK_XPATH_NODESET(n, T("(preceding-sibling::chapter)[position()=2]")) % 4;
}

TEST_XML(xpath_paths_predicate_filter_number, "<node><chapter/><chapter/><chapter/><chapter/><chapter/></node>")
{
	doc.precompute_document_order();

	xml_node n = doc.child(T("node")).child(T("chapter")).next_sibling().next_sibling();

	CHECK_XPATH_NODESET(n, T("(following-sibling::chapter)[1]")) % 6;
	CHECK_XPATH_NODESET(n, T("(following-sibling::chapter)[2]")) % 7;
	CHECK_XPATH_NODESET(n, T("(preceding-sibling::chapter)[1]")) % 3;
	CHECK_XPATH_NODESET(n, T("(preceding-sibling::chapter)[2]")) % 4;
}

TEST_XML(xpath_paths_predicate_filter_posinv, "<node><employee/><employee secretary=''/><employee assistant=''/><employee secretary='' assistant=''/><employee assistant='' secretary=''/></node>")
{
	doc.precompute_document_order();

	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(n, T("employee")) % 3 % 4 % 6 % 8 % 11;
	CHECK_XPATH_NODESET(n, T("(employee[@secretary])[@assistant]")) % 8 % 11;
	CHECK_XPATH_NODESET(n, T("((employee)[@assistant])[@secretary]")) % 8 % 11;
}

TEST_XML(xpath_paths_step_compose, "<node><foo><foo/><foo/></foo><foo/></node>")
{
	doc.precompute_document_order();

	xml_node n = doc.child(T("node"));

	CHECK_XPATH_NODESET(n, T("(.)/foo")) % 3 % 6;
	CHECK_XPATH_NODESET(n, T("(.)//foo")) % 3 % 4 % 5 % 6;
	CHECK_XPATH_NODESET(n, T("(./..)//*")) % 2 % 3 % 4 % 5 % 6;

	CHECK_XPATH_FAIL(T("(1)/foo"));
	CHECK_XPATH_FAIL(T("(1)//foo"));
}

TEST_XML(xpath_paths_descendant_double_slash_w3c, "<node><para><para/><para/><para><para/></para></para><para/></node>")
{
	doc.precompute_document_order();

	CHECK_XPATH_NODESET(doc, T("//para")) % 3 % 4 % 5 % 6 % 7 % 8;
	CHECK_XPATH_NODESET(doc, T("/descendant::para")) % 3 % 4 % 5 % 6 % 7 % 8;
	CHECK_XPATH_NODESET(doc, T("//para[1]")) % 3 % 4 % 7;
	CHECK_XPATH_NODESET(doc, T("/descendant::para[1]")) % 3;
}

TEST_XML(xpath_paths_needs_sorting, "<node><child/><child/><child><subchild/><subchild/></child></node>")
{
    doc.precompute_document_order();

    CHECK_XPATH_NODESET(doc, T("(node/child/subchild)[2]")) % 7;
}

#endif
