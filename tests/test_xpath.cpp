#ifndef PUGIXML_NO_XPATH

#include "common.hpp"

TEST_XML(xpath_document_order, "<node><child1 attr1='value1' attr2='value2'/><child2 attr1='value1'>test</child2></node>")
{
	CHECK(xml_node().document_order() == 0);
	CHECK(doc.child(T("node")).document_order() == 0);
	CHECK(doc.document_order() == 0);

	doc.precompute_document_order();

	CHECK(doc.document_order() == 1);
	CHECK(doc.child(T("node")).document_order() == 2);
	CHECK(doc.child(T("node")).child(T("child1")).document_order() == 3);
	CHECK(doc.child(T("node")).child(T("child1")).attribute(T("attr1")).document_order() == 4);
	CHECK(doc.child(T("node")).child(T("child1")).attribute(T("attr2")).document_order() == 5);
	CHECK(doc.child(T("node")).child(T("child2")).document_order() == 6);
	CHECK(doc.child(T("node")).child(T("child2")).attribute(T("attr1")).document_order() == 7);
	CHECK(doc.child(T("node")).child(T("child2")).first_child().document_order() == 8);
}

TEST(xpath_allocator_many_pages)
{
	std::basic_string<pugi::char_t> query = T("0");

	for (int i = 0; i < 128; ++i) query += T("+string-length('abcdefgh')");

	CHECK_XPATH_NUMBER(xml_node(), query.c_str(), 1024);
}

TEST(xpath_allocator_large_page)
{
	std::basic_string<pugi::char_t> query;

	for (int i = 0; i < 1024; ++i) query += T("abcdefgh");

	CHECK_XPATH_NUMBER(xml_node(), (T("string-length('") + query + T("')")).c_str(), 8192);
}

TEST_XML(xpath_sort_complex, "<node><child1 attr1='value1' attr2='value2'/><child2 attr1='value1'>test</child2></node>")
{
	// just some random union order, it should not matter probably?
	xpath_node_set ns = doc.child(T("node")).select_nodes(T("child1 | child2 | child1/@* | . | child2/@* | child2/text()"));

	ns.sort(false);
	xpath_node_set sorted = ns;

	ns.sort(true);
	xpath_node_set reverse_sorted = ns;

	doc.precompute_document_order();

	xpath_node_set_tester(sorted, "sorted order failed") % 2 % 3 % 4 % 5 % 6 % 7 % 8;
	xpath_node_set_tester(reverse_sorted, "reverse sorted order failed") % 8 % 7 % 6 % 5 % 4 % 3 % 2;
}

TEST_XML(xpath_sort_children, "<node><child><subchild id='1'/></child><child><subchild id='2'/></child></node>")
{
	xpath_node_set ns = doc.child(T("node")).select_nodes(T("child/subchild[@id=1] | child/subchild[@id=2]"));

	ns.sort(false);
	xpath_node_set sorted = ns;

	ns.sort(true);
	xpath_node_set reverse_sorted = ns;

	doc.precompute_document_order();

	xpath_node_set_tester(sorted, "sorted order failed") % 4 % 7;
	xpath_node_set_tester(reverse_sorted, "reverse sorted order failed") % 7 % 4;
}

TEST_XML(xpath_sort_attributes, "<node/>")
{
	xml_node n = doc.child(T("node"));

	// we need to insert attributes manually since unsorted node sets are (always?) sorted via pointers because of remove_duplicates,
	// so we need to have different document and pointer order to cover all comparator cases
	n.append_attribute(T("attr2"));
	n.append_attribute(T("attr3"));
	n.insert_attribute_before(T("attr1"), n.attribute(T("attr2")));

	xpath_node_set ns = n.select_nodes(T("@*"));

	ns.sort(true);
	xpath_node_set reverse_sorted = ns;

	ns.sort(false);
	xpath_node_set sorted = ns;

	doc.precompute_document_order();

	xpath_node_set_tester(sorted, "sorted order failed") % 3 % 4 % 5;
	xpath_node_set_tester(reverse_sorted, "reverse sorted order failed") % 5 % 4 % 3;
}

#endif
