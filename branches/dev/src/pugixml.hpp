///////////////////////////////////////////////////////////////////////////////
//
// Pug Improved XML Parser - Version 0.3
// --------------------------------------------------------
// Copyright (C) 2006-2007, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
// This work is based on the pugxml parser, which is:
// Copyright (C) 2003, by Kristen Wegner (kristen@tima.net)
// Released into the Public Domain. Use at your own risk.
// See pugxml.xml for further information, history, etc.
// Contributions by Neville Franks (readonly@getsoft.com).
//
///////////////////////////////////////////////////////////////////////////////

#ifndef HEADER_PUGIXML_HPP
#define HEADER_PUGIXML_HPP

#include "pugiconfig.hpp"

#ifndef PUGIXML_NO_STL
#	include <string>
#	include <istream>
#endif

// No XPath without STL
#ifdef PUGIXML_NO_STL
#	ifndef PUGIXML_NO_XPATH
#		define PUGIXML_NO_XPATH
#	endif
#endif

#include <cstddef>
#include <cstring>

/// The PugiXML Parser namespace.
namespace pugi
{
	/// Tree node classification.
	/// See 'xml_node_struct::type'
	enum xml_node_type
	{
		node_null,			///< Undifferentiated entity
		node_document,		///< A document tree's absolute root.
		node_element,		///< E.g. '<...>'
		node_pcdata,		///< E.g. '>...<'
		node_cdata,			///< E.g. '<![CDATA[...]]>'
		node_comment,		///< E.g. '<!--...-->'
		node_pi				///< E.g. '<?...?>'
	};

	/// Parser Options
	const size_t memory_block_size = 32768;		///< Memory block size, 32 kb

	const unsigned int parse_minimal			= 0x00000000; ///< Unset the following flags.
	const unsigned int parse_pi					= 0x00000001; ///< Parse '<?...?>'
	const unsigned int parse_comments			= 0x00000002; ///< Parse '<!--...-->'
	const unsigned int parse_cdata				= 0x00000004; ///< Parse '<![CDATA[...]]>'
	const unsigned int parse_ws_pcdata			= 0x00000008; ///< Do not skip PCDATA that consists only of whitespaces
	const unsigned int parse_ext_pcdata			= 0x00000010; ///< Do not skip PCDATA that is outside all tags (i.e. root)
	const unsigned int parse_escapes			= 0x00000020; ///< Parse &lt;, &gt;, &amp;, &quot;, &apos;, &#.. sequences
	const unsigned int parse_wnorm_attribute	= 0x00000080; ///< Normalize spaces in attributes (convert space-like characters to spaces + merge adjacent spaces + trim leading/trailing spaces)
	const unsigned int parse_wconv_attribute	= 0x00000100; ///< Convert space-like characters to spaces in attributes (only if wnorm is not set)
	const unsigned int parse_eol				= 0x00000200; ///< Perform EOL handling
	///< Set all flags, except parse_ws_pcdata, parse_trim_attribute, parse_pi and parse_comments
	const unsigned int parse_default			= parse_cdata | parse_ext_pcdata | parse_escapes | parse_wconv_attribute | parse_eol;
	const unsigned int parse_noset				= 0x80000000; ///< Parse with flags in xml_parser

	const unsigned int parse_w3c				= parse_pi | parse_comments | parse_cdata |
												parse_escapes | parse_wconv_attribute |
												parse_ws_pcdata | parse_eol;

	/// Forward declarations
	struct xml_attribute_struct;
	struct xml_node_struct;

	class xml_allocator;

	class xml_node_iterator;
	class xml_attribute_iterator;

	class xml_tree_walker;
	
	class xml_node;

	#ifndef PUGIXML_NO_XPATH
	class xpath_node;
	class xpath_node_set;
	class xpath_ast_node;
	class xpath_allocator;
	
	class xpath_query
	{
	private:
		// Noncopyable semantics
		xpath_query(const xpath_query&);
		xpath_query& operator=(const xpath_query&);

		xpath_allocator* m_alloc;
		xpath_ast_node* m_root;

		bool compile(const char* query);

	public:
		explicit xpath_query(const char* query);
		~xpath_query();
		
		bool evaluate_boolean(const xml_node& n);
		double evaluate_number(const xml_node& n);
		std::string evaluate_string(const xml_node& n);
		xpath_node_set evaluate_node_set(const xml_node& n);
	};
	#endif
	
	/// Provides a light-weight wrapper for manipulating xml_attribute_struct structures.
	///	Note: xml_attribute does not create any memory for the attribute it wraps; 
	///	it only wraps a pointer to an existing xml_attribute_struct.
	class xml_attribute
	{
		friend class xml_attribute_iterator;
		friend class xml_node;

	private:
		xml_attribute_struct* _attr; ///< The internal attribute pointer.
	
    	/// Safe bool type
    	typedef xml_attribute_struct* xml_attribute::*unspecified_bool_type;

		/// Initializing ctor
		explicit xml_attribute(xml_attribute_struct* attr);

	public:
		/// Default ctor
		xml_attribute();
		
	public:
		/// Comparison operators
		bool operator==(const xml_attribute& r) const;
		bool operator!=(const xml_attribute& r) const;
		bool operator<(const xml_attribute& r) const;
		bool operator>(const xml_attribute& r) const;
		bool operator<=(const xml_attribute& r) const;
		bool operator>=(const xml_attribute& r) const;
	
    	/// Safe bool conversion
    	operator unspecified_bool_type() const;

    	/// Get next attribute if any, else xml_attribute()
    	xml_attribute next_attribute() const;

    	/// Get previous attribute if any, else xml_attribute()
    	xml_attribute previous_attribute() const;

		/// Cast attribute value as int. If not found, return 0.
		/// \return Attribute value as int, or 0.
		int as_int() const;

		/// Cast attribute value as double. If not found, return 0.0.
		/// \return Attribute value as double, or 0.0.
		double as_double() const;
	
		/// Cast attribute value as float. If not found, return 0.0.
		/// \return Attribute value as float, or 0.0.
		float as_float() const;

		/// Cast attribute value as bool. If not found, return false.
		/// \return Attribute value as bool, or false.
		bool as_bool() const;

		/// Document order or 0 if not set
		unsigned int document_order() const;

	public:
		/// Set string value
		xml_attribute& operator=(const char* rhs);
	
		/// Set int value
		xml_attribute& operator=(int rhs);
	
		/// Set double value
		xml_attribute& operator=(double rhs);
		
		/// Set bool value
		xml_attribute& operator=(bool rhs);

		/// Set attribute name
		bool set_name(const char* rhs);
		
		/// Set attribute value
		bool set_value(const char* rhs);

	public:
		/// True if internal pointer is valid
		bool empty() const;

	public:
		/// Access the attribute name.
		const char* name() const;

		/// Access the attribute value.
		const char* value() const;
	};

	/// Provides a light-weight wrapper for manipulating xml_node_struct structures.
	class xml_node
	{
		friend class xml_node_iterator;
		friend class xml_parser;

	private:
		xml_node_struct* _root; ///< Pointer to node root.

    	/// Safe bool type
    	typedef xml_node_struct* xml_node::*unspecified_bool_type;

	private:
		/// Node is tree root.
		bool type_document() const;

		/// Get allocator
		xml_allocator& get_allocator() const;

	public:
		/// Default constructor.
		///	Node root points to a dummy 'xml_node_struct' structure. Test for this 
		///	with 'empty'.
		xml_node();

		/// Construct, wrapping the given 'xml_node_struct' pointer.
		explicit xml_node(xml_node_struct* p);

	public:
		/// Base iterator type (for child nodes). Same as 'child_iterator'.
		typedef xml_node_iterator iterator;

		/// Attribute iterator type.
		typedef xml_attribute_iterator attribute_iterator;

		/// Access the begin iterator for this node's collection of child nodes.
		/// Same as 'children_begin'.
		iterator begin() const;
	
		/// Access the end iterator for this node's collection of child nodes.
		/// Same as 'children_end'.
		iterator end() const;
		
		/// Access the begin iterator for this node's collection of child nodes.
		/// Same as 'begin'.
		iterator children_begin() const;
	
		/// Access the end iterator for this node's collection of child nodes.
		/// Same as 'end'.
		iterator children_end() const;
	
		/// Access the begin iterator for this node's collection of attributes.
		attribute_iterator attributes_begin() const;
	
		/// Access the end iterator for this node's collection of attributes.
		attribute_iterator attributes_end() const;

		/// Access the begin iterator for this node's collection of siblings.
		iterator siblings_begin() const;
	
		/// Access the end iterator for this node's collection of siblings.
		iterator siblings_end() const;
	
	public:
    	/// Safe bool conversion
		operator unspecified_bool_type() const;
	
		/// Comparison operators
		bool operator==(const xml_node& r) const;
		bool operator!=(const xml_node& r) const;
		bool operator<(const xml_node& r) const;
		bool operator>(const xml_node& r) const;
		bool operator<=(const xml_node& r) const;
		bool operator>=(const xml_node& r) const;

	public:
		/// Node pointer is null, or type is node_null. Same as type_null.
		bool empty() const;

	public:
		/// Access node entity type.
		xml_node_type type() const;

		/// Access pointer to node name if any, else empty string.
		const char* name() const;

		/// Access pointer to data if any, else empty string.
		const char* value() const;
	
		/// Access child node at name as xml_node or xml_node(NULL) if bad name.
		xml_node child(const char* name) const;

		/// Access child node at name as xml_node or xml_node(NULL) if bad name.
		/// Enable wildcard matching.
		xml_node child_w(const char* name) const;

		/// Access the attribute having 'name'.
		xml_attribute attribute(const char* name) const;

		/// Access the attribute having 'name'.
		/// Enable wildcard matching.
		xml_attribute attribute_w(const char* name) const;

		/// Access sibling node at name as xml_node or xml_node(NULL) if bad name.
		xml_node sibling(const char* name) const;

		/// Access sibling node at name as xml_node or xml_node(NULL) if bad name.
		/// Enable wildcard matching.
		xml_node sibling_w(const char* name) const;

		/// Access current node's next sibling by position and name.
		xml_node next_sibling(const char* name) const;

		/// Access current node's next sibling by position and name.
		/// Enable wildcard matching.
		xml_node next_sibling_w(const char* name) const;

		/// Access current node's next sibling by position.
		xml_node next_sibling() const;

		/// Access current node's previous sibling by position and name.
		xml_node previous_sibling(const char* name) const;

		/// Access current node's previous sibling by position and name.
		/// Enable wildcard matching.
		xml_node previous_sibling_w(const char* name) const;

		/// Access current node's previous sibling by position.
		xml_node previous_sibling() const;

		/// Access node's parent if any, else xml_node(NULL)
		xml_node parent() const;

		/// Access root of the tree this node belongs to.
		xml_node root() const;

		/// Return PCDATA/CDATA that is child of current node. If none, return empty string.
		const char* child_value() const;

		/// Return PCDATA/CDATA that is child of specified child node. If none, return empty string.
		const char* child_value(const char* name) const;

		/// Return PCDATA/CDATA that is child of specified child node. If none, return empty string.
		/// Enable wildcard matching.
		const char* child_value_w(const char* name) const;

	public:	
		/// Set node name (for PI/element nodes)
		bool set_name(const char* rhs);
		
		/// Set node value (for PI/PCDATA/CDATA/comment nodes)
		bool set_value(const char* rhs);

		/// Add attribute with specified name (for element nodes)
		xml_attribute append_attribute(const char* name);

		/// Add node with specified type (for element nodes)
		xml_node append_child(xml_node_type type = node_element);

	public:
		/// Access node's first attribute if any, else xml_attribute()
		xml_attribute first_attribute() const;

		/// Access node's last attribute if any, else xml_attribute()
        xml_attribute last_attribute() const;

		/// Find all elements having the given name.
		template <typename OutputIterator> void all_elements_by_name(const char* name, OutputIterator it) const;

		/// Find all elements having the given name.
		/// Enable wildcard matching.
		template <typename OutputIterator> void all_elements_by_name_w(const char* name, OutputIterator it) const;

		/// Access node's first child if any, else xml_node()
		xml_node first_child() const;

		/// Access node's last child if any, else xml_node()
        xml_node last_child() const;
		
		/// Find attribute using the predicate
		/// Predicate should take xml_attribute and return bool.
		template <typename Predicate> xml_attribute find_attribute(Predicate pred) const;

		/// Find child using the predicate
		/// Predicate should take xml_node and return bool.
		template <typename Predicate> xml_node find_child(Predicate pred) const;

		/// Recursively-implemented depth-first find element using the predicate
		/// Predicate should take xml_node and return bool.
		template <typename Predicate> xml_node find_element(Predicate pred) const;

		/// Recursively-implemented depth-first find the first matching element. 
		/// Use for shallow drill-downs.
		xml_node first_element(const char* name) const;

		/// Recursively-implemented depth-first find the first matching element. 
		/// Use for shallow drill-downs.
		/// Enable wildcard matching.
		xml_node first_element_w(const char* name) const;

		/// Recursively-implemented depth-first find the first matching element 
		/// also having matching PCDATA.
		xml_node first_element_by_value(const char* name, const char* value) const;

		/// Recursively-implemented depth-first find the first matching element 
		/// also having matching PCDATA.
		/// Enable wildcard matching.
		xml_node first_element_by_value_w(const char* name, const char* value) const;

		/// Recursively-implemented depth-first find the first matching element 
		/// also having matching attribute.
		xml_node first_element_by_attribute(const char* name, const char* attr_name, const char* attr_value) const;

		/// Recursively-implemented depth-first find the first matching element 
		/// also having matching attribute.
		/// Enable wildcard matching.
		xml_node first_element_by_attribute_w(const char* name, const char* attr_name, const char* attr_value) const;

		/// Recursively-implemented depth-first find the first element 
		/// having matching attribute.
		xml_node first_element_by_attribute(const char* attr_name, const char* attr_value) const;

		/// Recursively-implemented depth-first find the first element 
		/// having matching attribute.
		/// Enable wildcard matching.
		xml_node first_element_by_attribute_w(const char* attr_name, const char* attr_value) const;

		/// Recursively-implemented depth-first find the first matching entity. 
		/// Use for shallow drill-downs.
		xml_node first_node(xml_node_type type) const;

#ifndef PUGIXML_NO_STL
		/// Compile the absolute node path from root as a text string.
		/// \param delimiter - Delimiter character to insert between element names.
		/// \return path string (e.g. with '/' as delimiter, '/document/.../this'.
		std::string path(char delimiter = '/') const;
#endif

		/// Search for a node by path.
		/// \param path - Path string; e.g. './foo/bar' (relative to node), '/foo/bar' (relative 
		/// to root), '../foo/bar' (pop relative position).
		/// \param delimiter - Delimiter character to use in tokenizing path.
		/// \return Matching node, or xml_node() if not found.
		xml_node first_element_by_path(const char* path, char delimiter = '/') const;

		/// Recursively traverse the tree.
		bool traverse(xml_tree_walker& walker) const;
	
	#ifndef PUGIXML_NO_XPATH
		/// Select single node by query
		/// \param query - query string
		/// \return Matching XPath node, or xpath_node() if not found
		xpath_node select_single_node(const char* query) const;

		/// Select single node by query
		/// \param query - compiled query
		/// \return Matching XPath node, or xpath_node() if not found
		xpath_node select_single_node(xpath_query& query) const;

		/// Select nodes by query
		/// \param query - query string
		/// \return Matching XPath nodes, or empty set if not found
		xpath_node_set select_nodes(const char* query) const;

		/// Select nodes by query
		/// \param query - compiled query
		/// \return Matching XPath nodes, or empty set if not found
		xpath_node_set select_nodes(xpath_query& query) const;
	#endif
		
		/// Document order or 0 if not set
		unsigned int document_order() const;

		/// Compute document order for the whole tree (valid only for node_document)
		void precompute_document_order();
	};

	/// Child node iterator.
	class xml_node_iterator
#ifndef PUGIXML_NO_STL
	: public std::iterator<std::bidirectional_iterator_tag, const xml_node>
#endif
	{
		friend class xml_node;

	private:
		xml_node _prev;
		xml_node _wrap;

		/// Initializing ctor
		explicit xml_node_iterator(xml_node_struct* ref);
	public:
		/// Default ctor
		xml_node_iterator();

		/// Initializing ctor
		xml_node_iterator(const xml_node& node);

		/// Initializing ctor (for past-the-end)
		xml_node_iterator(xml_node_struct* ref, xml_node_struct* prev);

		bool operator==(const xml_node_iterator& rhs) const;
		bool operator!=(const xml_node_iterator& rhs) const;

		const xml_node& operator*() const;
		const xml_node* operator->() const;

		const xml_node_iterator& operator++();
		xml_node_iterator operator++(int);
		
		const xml_node_iterator& operator--();
		xml_node_iterator operator--(int);
	};

	/// Attribute iterator.
	class xml_attribute_iterator
#ifndef PUGIXML_NO_STL
	: public std::iterator<std::bidirectional_iterator_tag, const xml_attribute>
#endif
	{
		friend class xml_node;

	private:
		xml_attribute _prev;
		xml_attribute _wrap;

		/// Initializing ctor
		explicit xml_attribute_iterator(xml_attribute_struct* ref);
	public:
		/// Default ctor
		xml_attribute_iterator();

		/// Initializing ctor
		xml_attribute_iterator(const xml_attribute& attr);

		/// Initializing ctor (for past-the-end)
		xml_attribute_iterator(xml_attribute_struct* ref, xml_attribute_struct* prev);

		bool operator==(const xml_attribute_iterator& rhs) const;
		bool operator!=(const xml_attribute_iterator& rhs) const;

		const xml_attribute& operator*() const;
		const xml_attribute* operator->() const;

		const xml_attribute_iterator& operator++();
		xml_attribute_iterator operator++(int);
		
		const xml_attribute_iterator& operator--();
		xml_attribute_iterator operator--(int);
	};

	/// Abstract tree walker class for xml_node::traverse().
	class xml_tree_walker
	{
	private:
		int _deep; ///< Current node depth.
	public:
		/// Default ctor
		xml_tree_walker();

		/// Virtual dtor
		virtual ~xml_tree_walker();

	public:
		/// Increment node depth.
		virtual void push();

		/// Decrement node depth
		virtual void pop();

		/// Access node depth
		virtual int depth() const;
	
	public:
		/// Callback when traverse on a node begins.
		/// \return returning false will abort the traversal.
		virtual bool begin(const xml_node&);

		/// Callback when traverse on a node ends.
		/// \return Returning false will abort the traversal.
		virtual bool end(const xml_node&);
	};

	/// Memory block (internal)
	struct xml_memory_block
	{
		xml_memory_block();

		xml_memory_block* next;
		size_t size;

		char data[memory_block_size];
	};

	struct transfer_ownership_tag {};

	/// Provides a high-level interface to the XML parser.
	class xml_parser
	{
	private:
		char*				_buffer; ///< character buffer

		xml_memory_block	_memory; ///< Memory block
		
		xml_node_struct*	_xmldoc; ///< Pointer to current XML document tree root.
		unsigned int		_optmsk; ///< Parser options.
	
		xml_parser(const xml_parser&);
		const xml_parser& operator=(const xml_parser&);

		void free();	///< free memory

	public:
		/// Constructor.
		/// \param optmsk - Options mask.
		xml_parser(unsigned int optmsk = parse_default);

		/// Parse constructor.
		/// \param xmlstr - readwrite string with xml data
		/// \param optmsk - Options mask.
		/// \see parse
		xml_parser(char* xmlstr, unsigned int optmsk = parse_default);

		/// Parse constructor that gains ownership.
		/// \param xmlstr - readwrite string with xml data
		/// \param optmsk - Options mask.
		/// \see parse
		xml_parser(const transfer_ownership_tag&, char* xmlstr, unsigned int optmsk = parse_default);

#ifndef PUGIXML_NO_STL
		/// Parse constructor.
		/// \param stream - stream with xml data
		/// \param optmsk - Options mask.
		/// \see parse
		xml_parser(std::istream& stream, unsigned int optmsk = parse_default);
#endif

		/// Dtor
		~xml_parser();

	public:
		/// Cast as xml_node (same as document).
		operator xml_node() const;

		/// Returns the root wrapped by an xml_node.
		xml_node document() const;

	public:
		/// Get parser options mask.
		unsigned int options() const;

		/// Set parser options mask.
		unsigned int options(unsigned int optmsk);

	public:
#ifndef PUGIXML_NO_STL
		/// Parse the given XML stream
		/// \param stream - stream with xml data
		/// \param optmsk - Options mask.
		void parse(std::istream& stream, unsigned int optmsk = parse_noset);
#endif

		/// Parse the file
		/// \param name - file name
		/// \param optmsk - Options mask.
		/// \return last position or NULL
		/// \rem input string is zero-segmented
		char* load(const char* name, unsigned int optmsk = parse_noset);

		/// Parse the given XML string in-situ.
		/// \param xmlstr - readwrite string with xml data
		/// \param optmsk - Options mask.
		/// \return last position or NULL
		/// \rem input string is zero-segmented
		char* parse(char* xmlstr, unsigned int optmsk = parse_noset);
		
		/// Parse the given XML string in-situ (gains ownership).
		/// \param xmlstr - readwrite string with xml data
		/// \param optmsk - Options mask.
		/// \return last position or NULL
		/// \rem input string is zero-segmented
		char* parse(const transfer_ownership_tag&, char* xmlstr, unsigned int optmsk = parse_noset);
	};

	/// XPath
#ifndef PUGIXML_NO_XPATH
	class xpath_exception: public std::exception
	{
	private:
		const char* m_message;

	public:
		xpath_exception(const char* message);

		virtual const char* what() const throw();
	};
	
	class xpath_node
	{
	private:
		xml_node m_node;
		xml_attribute m_attribute;
	
    	/// Safe bool type
    	typedef xml_node xpath_node::*unspecified_bool_type;

	public:
		xpath_node();
		xpath_node(const xml_node& node);
		xpath_node(const xml_attribute& attribute, const xml_node& parent);

		xml_node node() const;
		xml_attribute attribute() const;
		xml_node parent() const;

		operator unspecified_bool_type() const;
		
		bool operator==(const xpath_node& n) const;
		bool operator!=(const xpath_node& n) const;
	};

	class xpath_node_set
	{
		friend class xpath_ast_node;
		
	public:
		enum type_t
		{
			type_unsorted,
			type_sorted,
			type_sorted_reverse
		};
		
		typedef xpath_node* iterator;
		typedef const xpath_node* const_iterator;
	
	private:
		type_t m_type;
		
		xpath_node m_storage;
		
		xpath_node* m_begin;
		xpath_node* m_end;
		xpath_node* m_eos;
		
		bool m_using_storage;
		
	public:
		xpath_node_set();
		~xpath_node_set();
		
		xpath_node_set(const xpath_node_set& ns);
		xpath_node_set& operator=(const xpath_node_set& ns);
		
		type_t type() const;
		
		size_t size() const;
		
		iterator begin();
		const_iterator begin() const;
		
		iterator end();
		const_iterator end() const;
		
		void sort(bool reverse = false);
		void remove_duplicates();
		
		xpath_node first() const;
		
		bool empty() const;
		
		void push_back(const xpath_node& n);
		
		template <typename Iterator> void append(Iterator begin, Iterator end);
		
		void truncate(iterator it);
	};
#endif

	/// Utility functions for xml
	
#ifndef PUGIXML_NO_STL
	/// Convert utf16 to utf8
	std::string utf8(const wchar_t* str);
	
	/// Convert utf8 to utf16
	std::wstring utf16(const char* str);
#endif
	
	size_t utf8_length(const char* s);
}

/// Inline implementation

namespace pugi
{
	namespace impl
	{
		int strcmpwild(const char*, const char*);
	}

	template <typename OutputIterator> void xml_node::all_elements_by_name(const char* name, OutputIterator it) const
	{
		if (empty()) return;
		
		for (xml_node node = first_child(); node; node = node.next_sibling())
		{
			if (!strcmp(name, node.name()))
			{
				*it = node;
				++it;
			}
			
			if (node.first_child()) node.all_elements_by_name(name, it);
		}
	}

	template <typename OutputIterator> void xml_node::all_elements_by_name_w(const char* name, OutputIterator it) const
	{
		if (empty()) return;
		
		for (xml_node node = first_child(); node; node = node.next_sibling())
		{
			if (!impl::strcmpwild(name, node.name()))
			{
				*it = node;
				++it;
			}
			
			if (node.first_child()) node.all_elements_by_name_w(name, it);
		}
	}
	
	template <typename Predicate> inline xml_attribute xml_node::find_attribute(Predicate pred) const
	{
		if (!empty())
			for (xml_attribute attrib = first_attribute(); attrib; attrib = attrib.next_attribute())
				if (pred(attrib))
					return attrib;
		
		return xml_attribute();
	}

	template <typename Predicate> inline xml_node xml_node::find_child(Predicate pred) const
	{
		if (!empty())
			for (xml_node node = first_child(); node; node = node.next_sibling())
				if (pred(node))
					return node;

		return xml_node();
	}

	template <typename Predicate> inline xml_node xml_node::find_element(Predicate pred) const
	{
		if (!empty())
			for (xml_node node = first_child(); node; node = node.next_sibling())
			{
				if (pred(node))
					return node;
				
				if (node.first_child())
				{
					xml_node found = node.find_element(pred);
					if (found) return found;
				}
			}

		return xml_node();
	}
}

#endif
