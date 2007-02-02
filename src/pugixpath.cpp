///////////////////////////////////////////////////////////////////////////////
//
// Pug Improved XML Parser - Version 0.2
// --------------------------------------------------------
// Copyright (C) 2006-2007, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
// This work is based on the pugxml parser, which is:
// Copyright (C) 2003, by Kristen Wegner (kristen@tima.net)
// Released into the Public Domain. Use at your own risk.
// See pugxml.xml for further information, history, etc.
// Contributions by Neville Franks (readonly@getsoft.com).
//
///////////////////////////////////////////////////////////////////////////////

#include "pugixml.hpp"

#include <string>
#include <istream>

#include <iostream>
#include <fstream>
#include <vector>
#include <functional>

#include <algorithm>

#include <cfloat>
#include <cmath>

namespace
{
	using namespace pugi;
		
	enum chartype
	{
		ct_space = 1,			// \r, \n, space, tab
		ct_start_symbol = 2,	// Any symbol > 127, a-z, A-Z, _, :
		ct_digit = 4,			// 0-9
		ct_symbol = 8			// Any symbol > 127, a-z, A-Z, 0-9, _, :, -, .
	};

	static unsigned char chartype_table[256] =
	{
		0, 0, 0, 0, 0, 0, 0, 0,					0, 1, 1, 0, 0, 1, 0, 0,				// 0-15
		0, 0, 0, 0, 0, 0, 0, 0,					0, 0, 0, 0, 0, 0, 0, 0,				// 16-31
		1, 0, 0, 0, 0, 0, 0, 0,					0, 0, 0, 0, 0, 8, 8, 0,				// 32-47
		12, 12, 12, 12, 12, 12, 12, 12,			12, 12, 10, 0, 0, 0, 0, 0,			// 48-63
		0, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 10, 10, 10, 10, 10,		// 64-79
		10, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 0, 0, 0, 0, 10,			// 80-95
		0, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 10, 10, 10, 10, 10,		// 96-111
		10, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 0, 0, 0, 0, 0,			// 112-127

		10, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 10, 10, 10, 10, 10,
		10, 10, 10, 10, 10, 10, 10, 10,			10, 10, 10, 10, 10, 10, 10, 10
	};
	
	bool is_chartype(char c, chartype ct)
	{
		return !!(chartype_table[static_cast<unsigned char>(c)] & ct);
	}

	bool starts_with(const std::string& s, const char* pattern)
	{
		return s.compare(0, strlen(pattern), pattern) == 0;
	}

	std::string string_value(const xpath_node& na)
	{
		if (na.attribute())
			return na.attribute().value();
		else
		{
			const xml_node& n = na.node();

			if (n.type() == node_pcdata || n.type() == node_cdata ||
				n.type() == node_comment || n.type() == node_pi)
				return n.value();
			else if (n.type() == node_document || n.type() == node_element)
			{
				std::string result;

				xml_node c = n.first_child();
				
				while (c)
				{
					if (c.type() == node_pcdata || c.type() == node_cdata)
						result += c.value();
				
					if (c.first_child())
						c = c.first_child();
					else if (c.next_sibling())
						c = c.next_sibling();
					else
					{
						while (c && c != n) c = c.parent();
						
						if (c == n) break;
						
						c = c.next_sibling();
					}
				}
				
				return result;
			}
			else throw std::exception("Unknown node type");
		}
	}
	
	struct document_order_comparator: public std::binary_function<const xpath_node&, const xpath_node&, bool>
	{
		bool operator()(const xpath_node& lhs, const xpath_node& rhs) const
		{
			unsigned int lo = lhs.attribute() ? lhs.attribute().document_order() : lhs.node().document_order();
			unsigned int ro = rhs.attribute() ? rhs.attribute().document_order() : rhs.node().document_order();
			
			if (lo != 0 && ro != 0)
				return lo < ro;

			xml_node ln = lhs.node(), rn = rhs.node();

			if (lhs.attribute() && rhs.attribute())
			{
				if (lhs.parent() == rhs.parent()) return lhs.attribute() < rhs.attribute();
				
				ln = lhs.parent();
				rn = rhs.parent();
			}
			else if (lhs.attribute())
			{
				if (lhs.parent() == rhs.node()) return false;
				
				ln = lhs.parent();
			}
			else if (rhs.attribute())
			{
				if (rhs.parent() == lhs.node()) return true;
				
				rn = rhs.parent();
			}

			if (ln == rn) return false;
				
			xml_node lp = ln, rp = rn;
				
			while (lp != rp)
			{
				ln = lp;
				lp = lp.parent();
					
				if (lp != rp)
				{
					rn = rp;
					rp = rp.parent();
				}
			}
				
			if (!lp) // no common parent - ???
				return false;
			else // lp is parent, ln & rn are distinct siblings
			{
				for (; ln; ln = ln.next_sibling());
					if (ln == rn)
						return true;
			
				return false;
			}
		}
	};
};

namespace pugi
{
	xpath_node::xpath_node()
	{
	}
		
	xpath_node::xpath_node(const xml_node& node): m_node(node)
	{
	}
		
	xpath_node::xpath_node(const xml_attribute& attribute, const xml_node& parent): m_attribute(attribute), m_node(parent)
	{
	}

	xml_node xpath_node::node() const
	{
		return m_attribute ? xml_node() : m_node;
	}
		
	xml_attribute xpath_node::attribute() const
	{
		return m_attribute;
	}
	
	xml_node xpath_node::parent() const
	{
		return m_attribute ? m_node : m_node.parent();
	}

	xpath_node::operator xpath_node::unspecified_bool_type() const
	{
		return (m_node || m_attribute) ? &xpath_node::m_node : 0;
	}
	
	bool xpath_node::operator==(const xpath_node& n) const
	{
		return m_node == n.m_node && m_attribute == n.m_attribute;
	}
	
	bool xpath_node::operator!=(const xpath_node& n) const
	{
		return m_node != n.m_node || m_attribute != n.m_attribute;
	}

	xpath_node_set::xpath_node_set(): m_type(type_unsorted), m_begin(&m_storage), m_end(&m_storage), m_eos(&m_storage + 1), m_using_storage(true)
	{
	}

	xpath_node_set::~xpath_node_set()
	{
		if (!m_using_storage) delete[] m_begin;
	}
		
	xpath_node_set::xpath_node_set(const xpath_node_set& ns): m_type(type_unsorted), m_begin(&m_storage), m_end(&m_storage), m_eos(&m_storage + 1), m_using_storage(true)
	{
		*this = ns;
	}
	
	xpath_node_set& xpath_node_set::operator=(const xpath_node_set& ns)
	{
		if (!m_using_storage) delete[] m_begin;
		
		m_begin = m_end = m_eos = 0;
		
		if (ns.size() == 1)
		{
			m_storage = ns[0];
			m_begin = &m_storage;
			m_end = m_eos = &m_storage + 1;
			m_using_storage = true;
		}
		else
		{
			m_using_storage = false;
			append(ns.begin(), ns.end());
		}
		
		return *this;
	}	

	xpath_node_set::type_t xpath_node_set::type() const
	{
		return m_type;
	}
		
	size_t xpath_node_set::size() const
	{
		return m_end - m_begin;
	}
		
	bool xpath_node_set::empty() const
	{
		return size() == 0;
	}
		
	xpath_node_set::iterator xpath_node_set::begin()
	{
		return m_begin;
	}
	
	xpath_node_set::const_iterator xpath_node_set::begin() const
	{
		return m_begin;
	}
		
	xpath_node_set::iterator xpath_node_set::end()
	{
		return m_end;
	}
	
	xpath_node_set::const_iterator xpath_node_set::end() const
	{
		return m_end;
	}
	
	void xpath_node_set::sort(bool reverse)
	{
		std::sort(begin(), end(), document_order_comparator());
		
		if (reverse)
			std::reverse(begin(), end());
			
		m_type = reverse ? type_sorted_reverse : type_sorted;
	}

	const xpath_node& xpath_node_set::operator[](size_t i) const
	{
		return m_type == type_sorted_reverse ? *(end() - 1 - i) : *(begin() + i);
	}

	void xpath_node_set::push_back(const xpath_node& n)
	{
		if (m_end == m_eos)
			append(&n, &n + 1);
		else
		{
			*m_end = n;
			++m_end;
		}
	}

	template <typename Iterator> void xpath_node_set::append(Iterator begin, Iterator end)
	{
		size_t count = std::distance(begin, end);
		size_t size = m_end - m_begin;
		size_t capacity = m_eos - m_begin;
		
		if (capacity < size + count)
		{
			if (capacity < 2) capacity = 2;
			
			while (capacity < size + count) capacity += capacity / 2;
			
			xpath_node* storage = new xpath_node[capacity];
			std::copy(m_begin, m_end, storage);
			
			if (!m_using_storage) delete[] m_begin;
			
			m_using_storage = false;
			
			m_begin = storage;
			m_end = storage + size;
			m_eos = storage + capacity;
		}
		
		std::copy(begin, end, m_end);
		m_end += count;
	}

	void xpath_node_set::truncate(iterator it)
	{
		m_end = it;
	}

	xpath_result::xpath_result(const xpath_node_set& value): m_type(t_node_set), m_nodeset_value(value)
	{
	}

	xpath_result::xpath_result(double value): m_type(t_number), m_number_value(value)
	{
	}

	xpath_result::xpath_result(bool value): m_type(t_boolean), m_boolean_value(value)
	{
	}

	xpath_result::xpath_result(const char* value): m_type(t_string), m_string_value(value)
	{
	}

	xpath_result::xpath_result(const std::string& value): m_type(t_string), m_string_value(value)
	{
	}

	const xpath_node_set& xpath_result::as_node_set() const
	{
		if (m_type == t_node_set) return m_nodeset_value;
		else throw std::exception("Can't convert to node set from anything else");
	}

	bool xpath_result::as_boolean() const
	{
		if (m_type == t_boolean) return m_boolean_value;
		else if (m_type == t_number) return m_number_value != 0 && !_isnan(m_number_value);
		else if (m_type == t_string) return !m_string_value.empty();
		else if (m_type == t_node_set) return !m_nodeset_value.empty();
		else throw std::exception("Unknown type");
	}

	double xpath_result::as_number() const
	{
		if (m_type == t_boolean) return m_boolean_value ? 1.f : 0.f;
		else if (m_type == t_number) return m_number_value;
		else if (m_type == t_string) return atof(m_string_value.c_str());
		else if (m_type == t_node_set) return atof(as_string().c_str());
		else throw std::exception("Unknown type");
	}
		
	std::string xpath_result::as_string() const
	{
		if (m_type == t_boolean) return m_boolean_value ? "true" : "false";
		else if (m_type == t_number)
		{
			if (_isnan(m_number_value)) return "NaN";
			else if (_finite(m_number_value))
			{
				char buf[20];
				sprintf(buf, "%f", m_number_value);

				return buf;
			}
			else return m_number_value < 0 ? "-Infinity" : "Infinity";
		}
		else if (m_type == t_string) return m_string_value;
		else if (m_type == t_node_set)
		{
			if (m_nodeset_value.empty()) return "";
			else
			{
				return string_value(m_nodeset_value[0]);
			}
		}
		else throw std::exception("Unknown type");
	}

	xpath_result::type_t xpath_result::type() const
	{
		return m_type;
	}

	struct xpath_context
	{
		xml_node root;
		xpath_node n;
		size_t position, size;
	};

	enum lexeme_t
	{
		lex_none = 0,
		lex_equal,
		lex_not_equal,
		lex_less,
		lex_greater,
		lex_less_or_equal,
		lex_greater_or_equal,
		lex_plus,
		lex_minus,
		lex_multiply,
		lex_union,
		lex_var_ref,
		lex_open_brace,
		lex_close_brace,
		lex_quoted_string,
		lex_number,
		lex_slash,
		lex_double_slash,
		lex_open_square_brace,
		lex_close_square_brace,
		lex_string,
		lex_comma,
		lex_axis_attribute,
		lex_dot,
		lex_double_dot
	};

	class xpath_lexer
	{
	private:
		const char* m_cur;

		char* m_cur_lexeme_contents;
		size_t m_clc_size;
		size_t m_clc_capacity;

		lexeme_t m_cur_lexeme;

		void contents_clear()
		{
			m_clc_size = 0;
		}

		void contents_push(char c)
		{
			if (m_clc_size == m_clc_capacity)
			{
				if (!m_clc_capacity) m_clc_capacity = 16;
				else m_clc_capacity *= 2;

				char* s = new char[m_clc_capacity + 1];
				if (m_cur_lexeme_contents) strcpy(s, m_cur_lexeme_contents);
				
				delete[] m_cur_lexeme_contents;
				m_cur_lexeme_contents = s;
			}

			m_cur_lexeme_contents[m_clc_size++] = c;
			m_cur_lexeme_contents[m_clc_size] = 0;
		}

	public:
		explicit xpath_lexer(const char* query): m_cur(query)
		{
			m_clc_capacity = m_clc_size = 0;
			m_cur_lexeme_contents = 0;

			next();
		}
		
		~xpath_lexer()
		{
			delete[] m_cur_lexeme_contents;
		}
		
		const char* state() const
		{
			return m_cur;
		}
		
		void reset(const char* state)
		{
			m_cur = state;
			next();
		}

		void next()
		{
			contents_clear();

			while (is_chartype(*m_cur, ct_space)) ++m_cur;

			switch (*m_cur)
			{
			case 0:
				m_cur_lexeme = lex_none;
				break;
			
			case '>':
				if (*(m_cur+1) == '=')
				{
					m_cur += 2;
					m_cur_lexeme = lex_greater_or_equal;
				}
				else
				{
					m_cur += 1;
					m_cur_lexeme = lex_greater;
				}
				break;

			case '<':
				if (*(m_cur+1) == '=')
				{
					m_cur += 2;
					m_cur_lexeme = lex_less_or_equal;
				}
				else
				{
					m_cur += 1;
					m_cur_lexeme = lex_less;
				}
				break;

			case '!':
				if (*(m_cur+1) == '=')
				{
					m_cur += 2;
					m_cur_lexeme = lex_not_equal;
				}
				else
				{
					m_cur_lexeme = lex_none;
				}
				break;

			case '=':
				m_cur += 1;
				m_cur_lexeme = lex_equal;

				break;
			
			case '+':
				m_cur += 1;
				m_cur_lexeme = lex_plus;

				break;

			case '-':
				m_cur += 1;
				m_cur_lexeme = lex_minus;

				break;

			case '*':
				m_cur += 1;
				m_cur_lexeme = lex_multiply;

				break;

			case '|':
				m_cur += 1;
				m_cur_lexeme = lex_union;

				break;

			case '$':
				m_cur += 1;
				m_cur_lexeme = lex_var_ref;

				break;
			
			case '(':
				m_cur += 1;
				m_cur_lexeme = lex_open_brace;

				break;

			case ')':
				m_cur += 1;
				m_cur_lexeme = lex_close_brace;

				break;
			
			case '[':
				m_cur += 1;
				m_cur_lexeme = lex_open_square_brace;

				break;

			case ']':
				m_cur += 1;
				m_cur_lexeme = lex_close_square_brace;

				break;

			case ',':
				m_cur += 1;
				m_cur_lexeme = lex_comma;

				break;

			case '/':
				if (*(m_cur+1) == '/')
				{
					m_cur += 2;
					m_cur_lexeme = lex_double_slash;
				}
				else
				{
					m_cur += 1;
					m_cur_lexeme = lex_slash;
				}
				break;
		
			case '.':
				if (*(m_cur+1) == '.')
				{
					m_cur += 2;
					m_cur_lexeme = lex_double_dot;
				}
				else if (is_chartype(*(m_cur+1), ct_digit))
				{
					contents_push('0');
					contents_push('.');

					++m_cur;

					while (is_chartype(*m_cur, ct_digit))
						contents_push(*m_cur++);
					
					m_cur_lexeme = lex_number;
				}
				else
				{
					m_cur += 1;
					m_cur_lexeme = lex_dot;
				}
				break;

			case '@':
				m_cur += 1;
				m_cur_lexeme = lex_axis_attribute;

				break;

			case '"':
			case '\'':
			{
				char terminator = *m_cur;

				++m_cur;

				while (*m_cur && *m_cur != terminator)
					contents_push(*m_cur++);
				
				if (!*m_cur)
					m_cur_lexeme = lex_none;
				else
				{
					m_cur += 1;
					m_cur_lexeme = lex_quoted_string;
				}

				break;
			}

			default:
				if (is_chartype(*m_cur, ct_digit))
				{
					while (is_chartype(*m_cur, ct_digit))
						contents_push(*m_cur++);
				
					if (*m_cur == '.' && is_chartype(*(m_cur+1), ct_digit))
					{
						contents_push(*m_cur++);

						while (is_chartype(*m_cur, ct_digit))
							contents_push(*m_cur++);
					}

					m_cur_lexeme = lex_number;
				}
				else if (is_chartype(*m_cur, ct_start_symbol))
				{
					while (is_chartype(*m_cur, ct_symbol))
						contents_push(*m_cur++);
				
					while (is_chartype(*m_cur, ct_space)) ++m_cur;

					m_cur_lexeme = lex_string;
				}
			}
		}

		lexeme_t current() const
		{
			return m_cur_lexeme;
		}

		const char* contents() const
		{
			return m_cur_lexeme_contents;
		}
	};

	enum ast_type_t
	{
		ast_none,
		ast_op_or,						// left or right
		ast_op_and,						// left and right
		ast_op_equal,					// left = right
		ast_op_not_equal, 				// left != right
		ast_op_less,					// left < right
		ast_op_greater,					// left > right
		ast_op_less_or_equal,			// left <= right
		ast_op_greater_or_equal,		// left >= right
		ast_op_add,						// left + right
		ast_op_subtract,				// left - right
		ast_op_multiply,				// left * right
		ast_op_divide,					// left / right
		ast_op_mod,						// left % right
		ast_op_negate,					// left - right
		ast_op_union,					// left | right
		ast_predicate,					// select * from left where right
		ast_variable,					// variable value
		ast_string_constant,			// string constant
		ast_number_constant,			// number constant
		ast_func_last,					// last()
		ast_func_position,				// position()
		ast_func_count,					// count(left)
		ast_func_id,					// id(left)
		ast_func_local_name_0,			// local-name()
		ast_func_local_name_1,			// local-name(left)
		ast_func_namespace_uri_0,		// namespace-uri()
		ast_func_namespace_uri_1,		// namespace-uri(left)
		ast_func_name_0,				// name()
		ast_func_name_1,				// name(left)
		ast_func_string_0,				// string()
		ast_func_string_1,				// string(left)
		ast_func_concat,				// concat(left, right, siblings)
		ast_func_starts_with,			// starts_with(left, right)
		ast_func_contains,				// contains(left, right)
		ast_func_substring_before,		// substring-before(left, right)
		ast_func_substring_after,		// substring-after(left, right)
		ast_func_substring_2,			// substring(left, right)
		ast_func_substring_3,			// substring(left, right, third)
		ast_func_string_length_0,		// string-length()
		ast_func_string_length_1,		// string-length(left)
		ast_func_normalize_space_0,		// normalize-space()
		ast_func_normalize_space_1,		// normalize-space(left)
		ast_func_translate,				// translate(left, right, third)
		ast_func_boolean,				// boolean(left)
		ast_func_not,					// not(left)
		ast_func_true,					// true()
		ast_func_false,					// false()
		ast_func_lang,					// lang(left)
		ast_func_number_0,				// number()
		ast_func_number_1,				// number(left)
		ast_func_sum,					// sum(left)
		ast_func_floor,					// floor(left)
		ast_func_ceiling,				// ceiling(left)
		ast_func_round,					// round(left)
		ast_step,						// process set left with step
		ast_step_root,					// select root node
		ast_step_root_or_descendant,	// select the whole tree
	};

	enum ast_rettype_t
	{
		ast_type_none,
		ast_type_node_set,
		ast_type_number,
		ast_type_string,
		ast_type_boolean
	};

	enum axis_t
	{
		axis_ancestor,
		axis_ancestor_or_self,
		axis_attribute,
		axis_child,
		axis_descendant,
		axis_descendant_or_self,
		axis_following,
		axis_following_sibling,
		axis_namespace,
		axis_parent,
		axis_preceding,
		axis_preceding_sibling,
		axis_self
	};
	
	enum nodetest_t
	{
		nodetest_name,
		nodetest_type,
		nodetest_pi,
		nodetest_all,
		nodetest_all_in_namespace
	};
		
	class xpath_ast_node
	{
	private:
		ast_type_t m_type;
		
		ast_rettype_t m_rettype;

		// tree node structure
		xpath_ast_node* m_left;
		xpath_ast_node* m_right;
		xpath_ast_node* m_third;
		xpath_ast_node* m_concat_next;

		// variable name for ast_variable
		// string value for ast_constant
		// node test for ast_step (node name/namespace/node type/pi target)
		const char* m_contents;

		// for t_step
		axis_t m_axis;
		nodetest_t m_test;
		
		xpath_ast_node(const xpath_ast_node&);
		xpath_ast_node& operator=(const xpath_ast_node&);

		template <template <class> class C> bool compare(xpath_ast_node* lhs, xpath_ast_node* rhs, xpath_context& c)
		{
			if (lhs->rettype() == ast_type_node_set && rhs->rettype() == ast_type_node_set)
			{
				xpath_node_set ls = lhs->eval_node_set(c);
				xpath_node_set rs = lhs->eval_node_set(c);
				
				for (xpath_node_set::const_iterator li = ls.begin(); li != ls.end(); ++li)
				for (xpath_node_set::const_iterator ri = rs.begin(); ri != rs.end(); ++ri)
				{
					if (C<std::string>()(string_value(*li), string_value(*ri)) == true)
						return true;
				}
				
				return false;
			}
			else if (lhs->rettype() != ast_type_node_set && rhs->rettype() == ast_type_node_set)
			{
				if (lhs->rettype() == ast_type_boolean)
					return C<bool>()(lhs->eval_boolean(c), rhs->eval_boolean(c));
				else if (lhs->rettype() == ast_type_number)
				{
					double l = lhs->eval_number(c);
					xpath_node_set rs = rhs->eval_node_set(c);
					
					for (xpath_node_set::const_iterator ri = rs.begin(); ri != rs.end(); ++ri)
					{
						if (C<double>()(l, atof(string_value(*ri).c_str())) == true)
							return true;
					}
					
					return false;
				}
				else if (lhs->rettype() == ast_type_string)
				{
					std::string l = lhs->eval_string(c);
					xpath_node_set rs = rhs->eval_node_set(c);

					for (xpath_node_set::const_iterator ri = rs.begin(); ri != rs.end(); ++ri)
					{
						if (C<std::string>()(l, string_value(*ri)) == true)
							return true;
					}
					
					return false;
				}
				else throw std::exception("Wrong types");
			}
			else if (lhs->rettype() == ast_type_node_set && rhs->rettype() != ast_type_node_set)
			{
				if (rhs->rettype() == ast_type_boolean)
					return C<bool>()(lhs->eval_boolean(c), rhs->eval_boolean(c));
				else if (rhs->rettype() == ast_type_number)
				{
					xpath_node_set ls = lhs->eval_node_set(c);
					double r = rhs->eval_number(c);

					for (xpath_node_set::const_iterator li = ls.begin(); li != ls.end(); ++li)
					{
						if (C<double>()(atof(string_value(*li).c_str()), r) == true)
							return true;
					}
					
					return false;
				}
				else if (rhs->rettype() == ast_type_string)
				{
					xpath_node_set ls = lhs->eval_node_set(c);
					std::string r = rhs->eval_string(c);

					for (xpath_node_set::const_iterator li = ls.begin(); li != ls.end(); ++li)
					{
						if (C<std::string>()(string_value(*li), r) == true)
							return true;
					}
					
					return false;
				}
				else throw std::exception("Wrong types");
			}
			else throw std::exception("Wrong types");
		}

		void step_push(xpath_node_set& ns, const xml_attribute& a, const xml_node& parent)
		{
			switch (m_test)
			{
			case nodetest_name:
				if (!strcmp(a.name(), m_contents)) ns.push_back(xpath_node(a, parent));
				break;
				
			case nodetest_all:
				ns.push_back(xpath_node(a, parent));
				break;
				
			case nodetest_all_in_namespace:
				if (!strncmp(a.name(), m_contents, strlen(m_contents)) && a.name()[strlen(m_contents)] == ':')
					ns.push_back(xpath_node(a, parent));
				break;
			}
		}
		
		void step_push(xpath_node_set& ns, const xml_node& n)
		{
			switch (m_test)
			{
			case nodetest_name:
				if (!strcmp(n.name(), m_contents)) ns.push_back(n);
				break;
				
			case nodetest_type:
				if ((n.type() == node_comment && !strcmp(m_contents, "comment")) ||
					(n.type() == node_pcdata && !strcmp(m_contents, "text")) ||
					(n.type() == node_cdata && !strcmp(m_contents, "text")) ||
					(n.type() == node_pi && !strcmp(m_contents, "processing-instruction")) ||
					!strcmp(m_contents, "node"))
					ns.push_back(n);
					break;
					
			case nodetest_pi:
				if (n.type() == node_pi && !strcmp(n.name(), m_contents))
					ns.push_back(n);
				break;
				
			case nodetest_all:
				ns.push_back(n);
				break;
				
			case nodetest_all_in_namespace:
				if (!strncmp(n.name(), m_contents, strlen(m_contents)) && n.name()[strlen(m_contents)] == ':')
					ns.push_back(n);
				break;
			} 
		}

		template <axis_t axis> void step_fill(xpath_node_set& ns, const xml_node& n)
		{
			switch (axis)
			{
			case axis_attribute:
				for (xml_attribute a = n.first_attribute(); a; a = a.next_attribute())
					step_push(ns, a, n);
				
				break;
			
			case axis_child:
				for (xml_node c = n.first_child(); c; c = c.next_sibling())
					step_push(ns, c);
					
				break;
			
			case axis_descendant_or_self:
				step_push(ns, n);
				// fall through
				
			case axis_descendant:
			{
				xml_node cur = n.first_child();
				
				if (cur)
				{
					do 
					{
						step_push(ns, cur);
						
						if (cur.first_child())
							cur = cur.first_child();
						else if (cur.next_sibling())
							cur = cur.next_sibling();
						else
						{
							while (!cur.next_sibling() && cur != n && cur.parent())
								cur = cur.parent();
						
							if (cur != n)
								cur = cur.next_sibling();
						}
					}
					while (cur && cur != n);
				}
				
				break;
			}
			
			case axis_following_sibling:
				for (xml_node c = n.next_sibling(); c; c = c.next_sibling())
					step_push(ns, c);
				
				break;
			
			case axis_preceding_sibling:
				for (xml_node c = n.previous_sibling(); c; c = c.previous_sibling())
					step_push(ns, c);
				
				break;
			
			case axis_following:
			{
				struct tree_traverser: public xml_tree_walker
				{
					xpath_ast_node& m_node;
					xpath_node_set& m_ns;
					xml_node m_start_node;
					bool m_fill;
				
					tree_traverser(xpath_ast_node& node, xpath_node_set& ns, const xml_node& start): m_node(node), m_ns(ns), m_start_node(start), m_fill(false)
					{
					}
					
					virtual bool begin(const xml_node& c)
					{
						if (m_fill) m_node.step_push(m_ns, c);
						if (c == m_start_node) m_fill = true;
						
						return true;
					}
				};
			
				xml_node root = n;
				while (root.parent()) root = root.parent();

				root.traverse(tree_traverser(*this, ns, n));
				
				break;
			}

			case axis_preceding:
			{
				struct tree_traverser: public xml_tree_walker
				{
					xpath_ast_node& m_node;
					xpath_node_set& m_ns;
					xml_node m_stop_node;
				
					tree_traverser(xpath_ast_node& node, xpath_node_set& ns, const xml_node& stop): m_node(node), m_ns(ns), m_stop_node(stop)
					{
					}
					
					virtual bool begin(const xml_node& c)
					{
						if (c == m_stop_node) return false;
						
						m_node.step_push(m_ns, c);
						return true;
					}
				};
				
				xml_node root = n;
				while (root.parent()) root = root.parent();
			
				root.traverse(tree_traverser(*this, ns, n));
				
				break;
			}
			
			case axis_ancestor_or_self:
				step_push(ns, n);
				// fall through
				
			case axis_ancestor:
			{
				xml_node cur = n.parent();
				
				while (cur)
				{
					step_push(ns, cur);
					
					cur = cur.parent();
				}
				
				break;
			}
				
			default:
				throw std::exception("Unimplemented axis");
			}
		}
		
		template <axis_t axis> void step_fill(xpath_node_set& ns, const xml_attribute& a, const xml_node& p)
		{
			switch (axis)
			{
			case axis_ancestor_or_self:
				step_push(ns, a, p);
				// fall through
				
			case axis_ancestor:
				step_fill<axis_ancestor_or_self>(ns, p);
				break;
			
			default:
				throw std::exception("Unimplemented axis");
			}
		}
		
		template <axis_t axis> void step_do(xpath_node_set& ns, xpath_context& c)
		{
			switch (axis)
			{
			case axis_parent:
				if (m_left)
				{
					xpath_node_set s = m_left->eval_node_set(c);
					
					for (xpath_node_set::const_iterator it = s.begin(); it != s.end(); ++it)
					{
						xml_node p = it->parent();
						if (p) step_push(ns, p);
					}
				}
				else
				{
					xml_node p = c.n.parent();
					if (p) step_push(ns, p);
				}

				break;
				
			case axis_self:
				if (m_left)
				{
					xpath_node_set s = m_left->eval_node_set(c);
					
					for (xpath_node_set::const_iterator it = s.begin(); it != s.end(); ++it)
						if (it->attribute()) step_push(ns, it->attribute(), it->parent());
						else step_push(ns, it->node());
				}
				else
				{
					if (c.n.node()) step_push(ns, c.n.node());
					else step_push(ns, c.n.attribute(), c.n.parent());
				}

				break;
				
			case axis_namespace:
				break;
				
			case axis_ancestor:
			case axis_ancestor_or_self:
				if (m_left)
				{
					xpath_node_set s = m_left->eval_node_set(c);
							
					for (xpath_node_set::const_iterator it = s.begin(); it != s.end(); ++it)
						if (it->node())
							step_fill<axis>(ns, it->node());
						else
							step_fill<axis>(ns, it->attribute(), it->parent());
				}
				else
				{
					if (c.n.node()) step_fill<axis>(ns, c.n.node());
					else step_fill<axis>(ns, c.n.attribute(), c.n.parent());
				}
				
				break;
		
			case axis_following:
			case axis_following_sibling:
			case axis_preceding:
			case axis_preceding_sibling:
			case axis_attribute:
			case axis_child:
			case axis_descendant:
			case axis_descendant_or_self:
				if (m_left)
				{
					xpath_node_set s = m_left->eval_node_set(c);
					
					for (xpath_node_set::const_iterator it = s.begin(); it != s.end(); ++it)
						if (it->node())
							step_fill<axis>(ns, it->node());
				}
				else if (c.n.node())
				{
					step_fill<axis>(ns, c.n.node());
				}
				
				break;
			
			default:
				throw std::exception("Unimplemented axis");
			}
		}
		
		void set_contents(const char* value)
		{
			if (value)
			{
				char* c = new char[strlen(value) + 1];
				strcpy(c, value);
				m_contents = c;
			}
			else m_contents = 0;
		}
	public:
		xpath_ast_node(ast_type_t type, const char* contents): m_type(type), m_left(0), m_right(0),
			m_third(0), m_concat_next(0), m_contents(0), m_rettype(ast_type_none)
		{
			set_contents(contents);
		}
		
		xpath_ast_node(ast_type_t type, xpath_ast_node* left = 0, xpath_ast_node* right = 0, xpath_ast_node* third = 0): m_type(type),
			m_left(left), m_right(right), m_third(third), m_concat_next(0), m_contents(0), m_rettype(ast_type_none)
		{
		}

		xpath_ast_node(ast_type_t type, xpath_ast_node* left, axis_t axis, nodetest_t test, const char* contents):
			m_type(type), m_left(left), m_right(0), m_third(0), m_concat_next(0), m_contents(0),
			m_axis(axis), m_test(test), m_rettype(ast_type_none)
		{
			set_contents(contents);
		}

		~xpath_ast_node()
		{
			delete[] m_contents;
			delete m_left;
			delete m_right;
			delete m_third;
			delete m_concat_next;
		}
		
		void set_concat_next(xpath_ast_node* value)
		{
			delete m_concat_next;
			m_concat_next = value;
		}

		bool eval_boolean(xpath_context& c)
		{
			switch (m_type)
			{
			case ast_op_or:
				if (m_left->eval_boolean(c)) return true;
				else return m_right->eval_boolean(c);
				
			case ast_op_and:
				if (!m_left->eval_boolean(c)) return false;
				else return m_right->eval_boolean(c);
				
			case ast_op_equal:
				if (m_left->rettype() != ast_type_node_set && m_right->rettype() != ast_type_node_set)
				{
					if (m_left->rettype() == ast_type_boolean || m_right->rettype() == ast_type_boolean)
						return m_left->eval_boolean(c) == m_right->eval_boolean(c);
					else if (m_left->rettype() == ast_type_number || m_right->rettype() == ast_type_number)
						return m_left->eval_number(c) == m_right->eval_number(c);
					else if (m_left->rettype() == ast_type_string || m_right->rettype() == ast_type_string)
						return m_left->eval_string(c) == m_right->eval_string(c);
					else
						throw std::exception("Wrong types");
				}
				else return compare<std::equal_to>(m_left, m_right, c);

			case ast_op_not_equal:
				if (m_left->rettype() != ast_type_node_set && m_right->rettype() != ast_type_node_set)
				{
					if (m_left->rettype() == ast_type_boolean || m_right->rettype() == ast_type_boolean)
						return m_left->eval_boolean(c) == m_right->eval_boolean(c);
					else if (m_left->rettype() == ast_type_number || m_right->rettype() == ast_type_number)
						return m_left->eval_number(c) == m_right->eval_number(c);
					else if (m_left->rettype() == ast_type_string || m_right->rettype() == ast_type_string)
						return m_left->eval_string(c) == m_right->eval_string(c);
					else
						throw std::exception("Wrong types");
				}
				else return compare<std::not_equal_to>(m_left, m_right, c);
	
			case ast_op_less:
				if (m_left->rettype() != ast_type_node_set && m_right->rettype() != ast_type_node_set)
					return m_left->eval_number(c) < m_right->eval_number(c);
				else
					return compare<std::less>(m_left, m_right, c);
			
			case ast_op_greater:
				if (m_left->rettype() != ast_type_node_set && m_right->rettype() != ast_type_node_set)
					return m_left->eval_number(c) > m_right->eval_number(c);
				else
					return compare<std::greater>(m_left, m_right, c);

			case ast_op_less_or_equal:
				if (m_left->rettype() != ast_type_node_set && m_right->rettype() != ast_type_node_set)
					return m_left->eval_number(c) <= m_right->eval_number(c);
				else
					return compare<std::less_equal>(m_left, m_right, c);
			
			case ast_op_greater_or_equal:
				if (m_left->rettype() != ast_type_node_set && m_right->rettype() != ast_type_node_set)
					return m_left->eval_number(c) >= m_right->eval_number(c);
				else
					return compare<std::greater_equal>(m_left, m_right, c);

			case ast_func_starts_with:
				return starts_with(m_left->eval_string(c), m_right->eval_string(c).c_str());

			case ast_func_contains:
				return m_left->eval_string(c).find(m_right->eval_string(c)) != std::string::npos;

			case ast_func_boolean:
				return m_left->eval_boolean(c);
				
			case ast_func_not:
				return !m_left->eval_boolean(c);
				
			case ast_func_true:
				return true;
				
			case ast_func_false:
				return false;

			case ast_func_lang:
			{
				if (c.n.attribute()) return false;
				
				std::string lang = m_left->eval_string(c);
				std::string lang_prefixed = lang + "-";
				
				xml_node n = c.n.node();
				
				while (n.type() != node_document)
				{
					xml_attribute a = n.attribute("xml:lang");
					
					if (a)
					{
						const char* value = a.value();
						
						return !stricmp(value, lang.c_str()) || !strnicmp(value, lang_prefixed.c_str(), lang_prefixed.length());
					}
				}
				
				return false;
			}

			default:
			{
				switch (m_rettype)
				{
				case ast_type_number:
				{
					double r = eval_number(c);
					return r != 0 && !_isnan(r);
				}
				case ast_type_string:
					return !eval_string(c).empty();
				case ast_type_node_set:
					return !eval_node_set(c).empty();
				}
				
				throw std::exception("Unknown error");
			}
			}
		}

		double eval_number(xpath_context& c)
		{
			switch (m_type)
			{
			case ast_op_add:
				return m_left->eval_number(c) + m_right->eval_number(c);
				
			case ast_op_subtract:
				return m_left->eval_number(c) - m_right->eval_number(c);

			case ast_op_multiply:
				return m_left->eval_number(c) * m_right->eval_number(c);

			case ast_op_divide:
				return m_left->eval_number(c) / m_right->eval_number(c);

			case ast_op_mod:
				return fmod(m_left->eval_number(c), m_right->eval_number(c));

			case ast_op_negate:
				return -m_left->eval_number(c);

			case ast_number_constant:
				return atof(m_contents);

			case ast_func_last:
				return (double)c.size;
			
			case ast_func_position:
				return (double)c.position;

			case ast_func_count:
				return (double)m_left->eval_node_set(c).size();
			
			case ast_func_string_length_0:
				return (double)string_value(c.n).length();
			
			case ast_func_string_length_1:
				return (double)m_left->eval_string(c).length();
			
			case ast_func_number_0:
				return atof(string_value(c.n).c_str());
			
			case ast_func_number_1:
				return m_left->eval_number(c);

			case ast_func_sum:
			{
				double r = 0;
				
				xpath_node_set ns = m_left->eval_node_set(c);
				
				for (xpath_node_set::const_iterator it = ns.begin(); it != ns.end(); ++it)
					r += atof(string_value(*it).c_str());
			
				return r;
			}

			case ast_func_floor:
				return floor(m_left->eval_number(c));

			case ast_func_ceiling:
				return ceil(m_left->eval_number(c));

			case ast_func_round:
				return floor(m_left->eval_number(c) + 0.5);

			default:
			{
				switch (m_rettype)
				{
				case ast_type_boolean: return eval_boolean(c) ? 1 : 0;
				case ast_type_string: return atof(eval_string(c).c_str());
				case ast_type_node_set: return atof(eval_string(c).c_str());
				}
				
				throw std::exception("Unknown operation");
			}
			}
		}
		
		std::string eval_string(xpath_context& c)
		{
			switch (m_type)
			{
			case ast_string_constant:
				return m_contents;
			
			case ast_func_name_0:
			case ast_func_local_name_0:
			{
				xpath_node na = c.n;
				
				if (na.attribute()) return na.attribute().name();
				else return na.node().name();
			}

			case ast_func_name_1:
			case ast_func_local_name_1:
			{
				xpath_node_set ns = m_left->eval_node_set(c);
				if (ns.empty()) return "";
				
				xpath_node na = ns[0];
				
				if (na.attribute()) return na.attribute().name();
				else return na.node().name();
			}

			case ast_func_namespace_uri_0:
			case ast_func_namespace_uri_1:
				return "";

			case ast_func_string_0:
				return string_value(c.n);

			case ast_func_string_1:
				return m_left->eval_string(c);

			case ast_func_concat:
			{
				std::string r = m_left->eval_string(c);
				
				for (xpath_ast_node* n = m_right; n; n = n->m_concat_next)
					r += n->eval_string(c);
			
				return r;
			}

			case ast_func_substring_before:
			{
				std::string s = m_left->eval_string(c);
				std::string::size_type pos = s.find(m_right->eval_string(c));
				
				if (pos == std::string::npos) return "";
				else return std::string(s.begin(), s.begin() + pos);
			}
			
			case ast_func_substring_after:
			{
				std::string s = m_left->eval_string(c);
				std::string p = m_right->eval_string(c);
				
				std::string::size_type pos = s.find(p);
				
				if (pos == std::string::npos) return "";
				else return std::string(s.begin() + pos + p.length(), s.end());
			}

			case ast_func_substring_2:
			{
				std::string s = m_left->eval_string(c);
				int first = (int)m_right->eval_number(c);
				
				if (first < 1) first = 1;
				
				return s.substr(first);
			}
			
			case ast_func_substring_3:
			{
				std::string s = m_left->eval_string(c);
				int first = (int)m_right->eval_number(c);
				int last = (int)(first + m_third->eval_number(c));
				
				if (first < 1) first = 1;
				if (last > (int)s.length() + 1) last = (int)s.length() + 1;
				
				return s.substr(first, last - first);
			}

			case ast_func_normalize_space_0:
			case ast_func_normalize_space_1:
			{
				std::string s = m_type == ast_func_normalize_space_0 ? string_value(c.n) : m_left->eval_string(c);
				
				std::string r;
				r.reserve(s.size());
				
				for (std::string::const_iterator it = s.begin(); it != s.end(); ++it)
				{
					if (*it == ' ' || *it == '\r' || *it == '\n' || *it == '\t')
					{
						if (!r.empty() && r[r.size() - 1] != ' ')
							r += ' ';
					}
					else r += *it;
				}
				
				std::string::size_type pos = r.find_last_not_of(' ');
				if (pos == std::string::npos) r = "";
				else r.erase(r.begin() + pos + 1, r.end());
			
				return r;
			}

			case ast_func_translate:
			{
				std::string s = m_left->eval_string(c);
				std::string from = m_right->eval_string(c);
				std::string to = m_third->eval_string(c);
				
				for (std::string::iterator it = s.begin(); it != s.end(); )
				{
					std::string::size_type pos = from.find(*it);
					
					if (pos != std::string::npos && pos >= to.length())
						it = s.erase(it);
					else if (pos != std::string::npos)
						*it = to[pos];
				}
				
				return s;
			}

			default:
			{
				switch (m_rettype)
				{
				case ast_type_boolean: return eval_boolean(c) ? "true" : "false";
				case ast_type_number:
				{
					double r = eval_number(c);
					
					if (_isnan(r)) return "NaN";
					else if (_finite(r))
					{
						char buf[20];
						sprintf(buf, "%f", r);

						return buf;
					}
					else return r < 0 ? "-Infinity" : "Infinity";
				}
				case ast_type_node_set:
				{
					xpath_node_set ns = eval_node_set(c);
					return ns.empty() ? "" : string_value(ns[0]);
				}
				}
				
				throw std::exception("Unknown error");
			}
			}
		}

		xpath_node_set eval_node_set(xpath_context& c)
		{
			switch (m_type)
			{
			case ast_op_union:
			{
				xpath_node_set ls = m_left->eval_node_set(c);
				xpath_node_set rs = m_right->eval_node_set(c);
				
				ls.append(rs.begin(), rs.end());
				
				ls.sort();
				ls.truncate(std::unique(ls.begin(), ls.end()));
				
				return ls;
			}

			case ast_predicate:
			{
				xpath_node_set set = m_left->eval_node_set(c);
				
				xpath_context oc = c;
			
				size_t i = 0;
				
				xpath_node_set::iterator last = set.begin();
				
				// remove_if... or well, sort of
				for (xpath_node_set::const_iterator it = set.begin(); it != set.end(); ++it, ++i)
				{
					c.n = *it;
					c.position = i + 1;
					c.size = set.size();
				
					if (m_right->rettype() == ast_type_number)
					{
						if (m_right->eval_number(c) == i + 1)
							*last++ = *it;
					}
					else if (m_right->eval_boolean(c))
						*last++ = *it;
				}
			
				c = oc;
				
				set.truncate(last);
			
				return set;
			}
			
			case ast_func_id:
				return xpath_node_set();
			
			case ast_step:
			{
				xpath_node_set ns;
				
				switch (m_axis)
				{
				case axis_ancestor:
					step_do<axis_ancestor>(ns, c);
					break;
					
				case axis_ancestor_or_self:
					step_do<axis_ancestor_or_self>(ns, c);
					break;

				case axis_attribute:
					step_do<axis_attribute>(ns, c);
					break;

				case axis_child:
					step_do<axis_child>(ns, c);
					break;
				
				case axis_descendant:
					step_do<axis_descendant>(ns, c);
					break;

				case axis_descendant_or_self:
					step_do<axis_descendant_or_self>(ns, c);
					break;

				case axis_following:
					step_do<axis_following>(ns, c);
					break;
				
				case axis_following_sibling:
					step_do<axis_following_sibling>(ns, c);
					break;
				
				case axis_namespace:
					step_do<axis_namespace>(ns, c);
					break;
				
				case axis_parent:
					step_do<axis_parent>(ns, c);
					break;
				
				case axis_preceding:
					step_do<axis_preceding>(ns, c);
					break;

				case axis_preceding_sibling:
					step_do<axis_preceding_sibling>(ns, c);
					break;
				
				case axis_self:
					step_do<axis_self>(ns, c);
					break;

				default:
					throw std::exception("Not implemented");
				}
				
				ns.sort(m_axis == axis_ancestor || m_axis == axis_ancestor_or_self ||
					m_axis == axis_preceding || m_axis == axis_preceding_sibling);
				// ns.truncate(std::unique(ns.begin(), ns.end()));
				
				return ns;

				break;
			}

			case ast_step_root:
			{
				xpath_node_set ns;
			
				ns.push_back(c.root);
			
				return ns;
			}

			case ast_step_root_or_descendant:
			{
				xpath_node_set ns;
				
				struct tree_traverser: public xml_tree_walker
				{
					xpath_node_set& m_dest;
					
					tree_traverser(xpath_node_set& dest): m_dest(dest)
					{
					}
					
					virtual bool begin(const xml_node& n)
					{
						m_dest.push_back(n);

						return true;
					}
				};
			
				c.root.traverse(tree_traverser(ns));
			
				return ns;
			}

			default:
				throw std::exception("Unknown operation");
			}
		}
		
		void check_semantics()
		{
			switch (m_type)
			{
			case ast_op_or:
			case ast_op_and:
			case ast_op_equal:
			case ast_op_not_equal:
			case ast_op_less:
			case ast_op_greater:
			case ast_op_less_or_equal:
			case ast_op_greater_or_equal:
				m_left->check_semantics();
				m_right->check_semantics();
				m_rettype = ast_type_boolean;
				break;
				
			case ast_op_add:
			case ast_op_subtract:
			case ast_op_multiply:
			case ast_op_divide:
			case ast_op_mod:
				m_left->check_semantics();
				m_right->check_semantics();
				m_rettype = ast_type_number;
				break;
				
			case ast_op_negate:
				m_left->check_semantics();
				m_rettype = ast_type_number;
				break;
			
			case ast_op_union:
				m_left->check_semantics();
				m_right->check_semantics();
				if (m_left->rettype() != ast_type_node_set || m_right->rettype() != ast_type_node_set)
					throw std::exception("Semantics error: union operator has to be applied to node sets");
				m_rettype = ast_type_node_set;
				break;
			
			case ast_predicate:
				m_left->check_semantics();
				m_right->check_semantics();
				if (m_left->rettype() != ast_type_node_set)
					throw std::exception("Semantics error: predicate has to be applied to node set");
				m_rettype = ast_type_node_set;
				break;
			
			case ast_variable:
				throw std::exception("Semantics error: variable are not supported");
				
			case ast_string_constant:
				m_rettype = ast_type_string;
				break;
				
			case ast_number_constant:
				m_rettype = ast_type_number;
				break;
				
			case ast_func_last:
			case ast_func_position:
				m_rettype = ast_type_number;
				break;
		
			case ast_func_count:
				m_left->check_semantics();
				if (m_left->rettype() != ast_type_node_set)
					throw std::exception("Semantics error: count() has to be applied to node set");
				m_rettype = ast_type_number;
				break;
				
			case ast_func_id:
				m_left->check_semantics();
				m_rettype = ast_type_node_set;
				break;
				
			case ast_func_local_name_0:
			case ast_func_local_name_1:
			case ast_func_namespace_uri_0:
			case ast_func_namespace_uri_1:
			case ast_func_name_0:
			case ast_func_name_1:
				if (m_left)
				{
					m_left->check_semantics();
					if (m_left->rettype() != ast_type_node_set)
						throw std::exception("Semantics error: function has to be applied to node set");
				}
				m_rettype = ast_type_string;
				break;
				
			case ast_func_string_0:
			case ast_func_string_1:
				if (m_left) m_left->check_semantics();
				m_rettype = ast_type_string;
				break;
				
			case ast_func_concat:
				m_left->check_semantics();
				
				for (xpath_ast_node* n = m_right; n; n = n->m_concat_next)
					n->check_semantics();
					
				m_rettype = ast_type_string;
				break;
		
			case ast_func_starts_with:
			case ast_func_contains:
				m_left->check_semantics();
				m_right->check_semantics();
				m_rettype = ast_type_boolean;
				break;
				
			case ast_func_substring_before:
			case ast_func_substring_after:
			case ast_func_substring_2:
			case ast_func_substring_3:
				m_left->check_semantics();
				m_right->check_semantics();
				if (m_third) m_third->check_semantics();
				m_rettype = ast_type_string;
				break;
		
			case ast_func_string_length_0:
			case ast_func_string_length_1:
				if (m_left) m_left->check_semantics();
				m_rettype = ast_type_number;
				break;
				
			case ast_func_normalize_space_0:
			case ast_func_normalize_space_1:
			case ast_func_translate:
				if (m_left) m_left->check_semantics();
				if (m_right) m_right->check_semantics();
				if (m_third) m_third->check_semantics();
				m_rettype = ast_type_string;
				break;
		
			case ast_func_boolean:
			case ast_func_not:
			case ast_func_true:
			case ast_func_false:
			case ast_func_lang:
				if (m_left) m_left->check_semantics();
				m_rettype = ast_type_boolean;
				break;
			
			case ast_func_number_0:
			case ast_func_number_1:
				if (m_left) m_left->check_semantics();
				m_rettype = ast_type_number;
				break;
		
			case ast_func_sum:
				m_left->check_semantics();
				if (m_left->rettype() != ast_type_node_set)
					throw std::exception("Semantics error: sum() has to be applied to node set");
				m_rettype = ast_type_number;
				break;
				
			case ast_func_floor:
			case ast_func_ceiling:
			case ast_func_round:
				if (m_left) m_left->check_semantics();
				m_rettype = ast_type_number;
				break;
		
			case ast_step:
				if (m_left)
				{
					m_left->check_semantics();
					if (m_left->rettype() != ast_type_node_set)
						throw std::exception("Semantics error: step has to be applied to node set");
				}
				m_rettype = ast_type_node_set;
				break;
				
			case ast_step_root:
			case ast_step_root_or_descendant:
				m_rettype = ast_type_node_set;
				break;
			
			default:
				throw std::exception("Unknown semantics error");
			}
		}
		
		ast_rettype_t rettype() const
		{
			return m_rettype;
		}
	};

	class xpath_parser
	{
	private:
	    xpath_lexer m_lexer;
	    
	    std::vector<xpath_ast_node*> m_args;
	    std::string m_function;
	    
	    // PrimaryExpr ::= VariableReference | '(' Expr ')' | Literal | Number | FunctionCall
	    xpath_ast_node* parse_primary_expression()
	    {
	    	switch (m_lexer.current())
	    	{
	    	case lex_var_ref:
	    	{
	    		m_lexer.next();

	    		if (m_lexer.current() != lex_string)
	    			throw std::exception("incorrect variable reference");

				xpath_ast_node* n = new xpath_ast_node(ast_variable, m_lexer.contents());
				m_lexer.next();

				return n;
			}

			case lex_open_brace:
			{
				m_lexer.next();

				xpath_ast_node* n = parse_expression();

				if (m_lexer.current() != lex_close_brace)
					throw std::exception("unmatched braces");

				m_lexer.next();

				return n;
			}

			case lex_quoted_string:
			{
				xpath_ast_node* n = new xpath_ast_node(ast_string_constant, m_lexer.contents());
				m_lexer.next();

				return n;
			}

			case lex_number:
			{
				xpath_ast_node* n = new xpath_ast_node(ast_number_constant, m_lexer.contents());
				m_lexer.next();

				return n;
			}

			case lex_string:
			{
				m_args.erase(m_args.begin(), m_args.end());
				m_function = m_lexer.contents();
				m_lexer.next();
				
				if (m_lexer.current() != lex_open_brace)
					throw std::exception("Unrecognized function call");
				m_lexer.next();

				if (m_lexer.current() != lex_close_brace)
					m_args.push_back(parse_expression());

				while (m_lexer.current() != lex_close_brace)
				{
					if (m_lexer.current() != lex_comma)
						throw std::exception("no comma between function arguments");
					m_lexer.next();
					
					m_args.push_back(parse_expression());
				}
				
				m_lexer.next();
				
				ast_type_t type = ast_none;
				
				switch (m_function[0])
				{
				case 'b':
				{
					if (m_function == "boolean" && m_args.size() == 1)
						type = ast_func_boolean;
						
					break;
				}
				
				case 'c':
				{
					if (m_function == "count" && m_args.size() == 1)
						type = ast_func_count;
					else if (m_function == "contains" && m_args.size() == 2)
						type = ast_func_contains;
					else if (m_function == "concat" && m_args.size() >= 2)
					{
						for (size_t i = 1; i + 1 < m_args.size(); ++i)
							m_args[i]->set_concat_next(m_args[i+1]);

						return new xpath_ast_node(ast_func_concat, m_args[0], m_args[1]);
					}
					else if (m_function == "ceiling" && m_args.size() == 1)
						type = ast_func_ceiling;
						
					break;
				}
				
				case 'f':
				{
					if (m_function == "false" && m_args.size() == 0)
						type = ast_func_false;
					else if (m_function == "floor" && m_args.size() == 1)
						type = ast_func_floor;
						
					break;
				}
				
				case 'i':
				{
					if (m_function == "id" && m_args.size() == 1)
						type = ast_func_id;
						
					break;
				}
				
				case 'l':
				{
					if (m_function == "last" && m_args.size() == 0)
						type = ast_func_last;
					else if (m_function == "lang" && m_args.size() == 1)
						type = ast_func_lang;
					else if (m_function == "local-name" && m_args.size() <= 1)
						type = m_args.size() == 0 ? ast_func_local_name_0 : ast_func_local_name_1;
				
					break;
				}
				
				case 'n':
				{
					if (m_function == "name" && m_args.size() <= 1)
						type = m_args.size() == 0 ? ast_func_name_0 : ast_func_name_1;
					else if (m_function == "namespace-uri" && m_args.size() <= 1)
						type = m_args.size() == 0 ? ast_func_namespace_uri_0 : ast_func_namespace_uri_1;
					else if (m_function == "normalize-space" && m_args.size() <= 1)
						type = m_args.size() == 0 ? ast_func_normalize_space_0 : ast_func_normalize_space_1;
					else if (m_function == "not" && m_args.size() == 1)
						type = ast_func_not;
					else if (m_function == "number" && m_args.size() <= 1)
						type = m_args.size() == 0 ? ast_func_number_0 : ast_func_number_1;
				
					break;
				}
				
				case 'p':
				{
					if (m_function == "position" && m_args.size() == 0)
						type = ast_func_position;
					
					break;
				}
				
				case 'r':
				{
					if (m_function == "round" && m_args.size() == 1)
						type = ast_func_round;

					break;
				}
				
				case 's':
				{
					if (m_function == "string" && m_args.size() <= 1)
						type = m_args.size() == 0 ? ast_func_string_0 : ast_func_string_1;
					else if (m_function == "string-length" && m_args.size() <= 1)
						type = m_args.size() == 0 ? ast_func_string_length_0 : ast_func_string_length_1;
					else if (m_function == "starts-with" && m_args.size() == 2)
						type = ast_func_starts_with;
					else if (m_function == "substring-before" && m_args.size() == 2)
						type = ast_func_substring_before;
					else if (m_function == "substring-after" && m_args.size() == 2)
						type = ast_func_substring_after;
					else if (m_function == "substring" && (m_args.size() == 2 || m_args.size() == 3))
						type = m_args.size() == 2 ? ast_func_substring_2 : ast_func_substring_3;
					else if (m_function == "sum" && m_args.size() == 1)
						type = ast_func_sum;

					break;
				}
				
				case 't':
				{
					if (m_function == "translate" && m_args.size() == 3)
						type = ast_func_translate;
					else if (m_function == "true" && m_args.size() == 0)
						type = ast_func_true;
						
					break;
				}
				
				}
				
				if (type != ast_none)
				{
					switch (m_args.size())
					{
					case 0: return new xpath_ast_node(type);
					case 1: return new xpath_ast_node(type, m_args[0]);
					case 2: return new xpath_ast_node(type, m_args[0], m_args[1]);
					case 3: return new xpath_ast_node(type, m_args[0], m_args[1], m_args[2]);
					}
				}
				
				throw std::exception("Unrecognized function or wrong parameter count");
			}

	    	default:
	    		throw std::exception("unrecognizable primary expression");
	    	}
	    }
	    
	    // FilterExpr ::= PrimaryExpr | FilterExpr Predicate
	    // Predicate ::= '[' PredicateExpr ']'
	    // PredicateExpr ::= Expr
	    xpath_ast_node* parse_filter_expression()
	    {
	    	xpath_ast_node* n = parse_primary_expression();

	    	while (m_lexer.current() == lex_open_square_brace)
	    	{
	    		m_lexer.next();

	    		n = new xpath_ast_node(ast_predicate, n, parse_expression());

	    		if (m_lexer.current() != lex_close_square_brace)
	    			throw std::exception("Unmatched square brace");
	    	
	    		m_lexer.next();
	    	}
	    	
	    	return n;
	    }
	    
	    // Step ::= AxisSpecifier NodeTest Predicate* | AbbreviatedStep
	    // AxisSpecifier ::= AxisName '::' | '@'?
	    // NodeTest ::= NameTest | NodeType '(' ')' | 'processing-instruction' '(' Literal ')'
	    // NameTest ::= '*' | NCName ':' '*' | QName
	    // AbbreviatedStep ::= '.' | '..'
	    xpath_ast_node* parse_step(xpath_ast_node* set, bool override_descendant_or_self = false)
	    {
			axis_t axis;
			
			if (m_lexer.current() == lex_axis_attribute)
			{
				axis = axis_attribute;
				
				m_lexer.next();
			}
			else if (m_lexer.current() == lex_dot)
			{
				m_lexer.next();
				
				return new xpath_ast_node(ast_step_root);
			}
			else if (m_lexer.current() == lex_double_dot)
			{
				m_lexer.next();
				
				return new xpath_ast_node(ast_step, set, axis_parent, nodetest_all, 0);
			}
			else // implied child axis
				axis = override_descendant_or_self ? axis_descendant_or_self : axis_child;
	    
			nodetest_t nt_type;
			std::string nt_name;
			
			if (m_lexer.current() == lex_string)
			{
				// node name test
				nt_name = m_lexer.contents();
				m_lexer.next();
				
				// possible axis name here - check.
				if (nt_name.find("::") == std::string::npos && m_lexer.current() == lex_string && m_lexer.contents()[0] == ':' && m_lexer.contents()[1] == ':')
				{
					nt_name += m_lexer.contents();
					m_lexer.next();
				}
				
				bool axis_specified = true;
				
				switch (nt_name[0])
				{
				case 'a':
					if (starts_with(nt_name, "ancestor::")) axis = axis_ancestor;
					else if (starts_with(nt_name, "ancestor-or-self::")) axis = axis_ancestor_or_self;
					else if (starts_with(nt_name, "attribute::")) axis = axis_attribute;
					else axis_specified = false;
					
					break;
				
				case 'c':
					if (starts_with(nt_name, "child::")) axis = axis_child;
					else axis_specified = false;
					
					break;
				
				case 'd':
					if (starts_with(nt_name, "descendant::")) axis = axis_descendant;
					else if (starts_with(nt_name, "descendant-or-self::")) axis = axis_descendant_or_self;
					else axis_specified = false;
					
					break;
				
				case 'f':
					if (starts_with(nt_name, "following::")) axis = axis_following;
					else if (starts_with(nt_name, "following-sibling::")) axis = axis_following_sibling;
					else axis_specified = false;
					
					break;
				
				case 'n':
					if (starts_with(nt_name, "namespace::")) axis = axis_namespace;
					else axis_specified = false;
					
					break;
				
				case 'p':
					if (starts_with(nt_name, "parent::")) axis = axis_parent;
					else if (starts_with(nt_name, "preceding::")) axis = axis_preceding;
					else if (starts_with(nt_name, "preceding-sibling::")) axis = axis_preceding_sibling;
					else axis_specified = false;
					
					break;
				
				case 's':
					if (starts_with(nt_name, "self::")) axis = axis_ancestor_or_self;
					else axis_specified = false;
					
					break;

				default:
					axis_specified = false;
				}
				
				if (axis_specified)
				{
					nt_name.erase(0, nt_name.find("::") + 2);
				}
				
				if (nt_name.empty() && m_lexer.current() == lex_string)
				{
					nt_name += m_lexer.contents();
					m_lexer.next();
				}

				// node type test or processing-instruction
				if (m_lexer.current() == lex_open_brace)
				{
					m_lexer.next();
					
					if (nt_name == "processing-instruction")
					{
						if (m_lexer.current() != lex_quoted_string)
							throw std::exception("Only literals are allowed as arguments to processing-instruction()");
					
						nt_type = nodetest_pi;
						nt_name = m_lexer.contents();
						m_lexer.next();
					}
					else
					{
						nt_type = nodetest_type;
					}

					if (m_lexer.current() != lex_close_brace)
						throw std::exception("Unmatched brace near processing-instruction()");
					m_lexer.next();
				}
				// namespace *
				else if (m_lexer.current() == lex_multiply)
				{
					// Only strings of form 'namespace:*' are permitted
					if (nt_name.empty())
						nt_type = nodetest_all;
					else
					{
						if (nt_name.find(':') != nt_name.size() - 1)
							throw std::exception("Wrong namespace-like node test");
						
						nt_name.erase(nt_name.size() - 1);
						
						nt_type = nodetest_all_in_namespace;
					}
					
					m_lexer.next();
				}
				else nt_type = nodetest_name;
			}
			else if (m_lexer.current() == lex_multiply)
			{
				nt_type = nodetest_all;
				m_lexer.next();
			}
			else throw std::exception("Unrecognized node test");
			
			xpath_ast_node* n = new xpath_ast_node(ast_step, set, axis, nt_type, nt_name.c_str());
			
			while (m_lexer.current() == lex_open_square_brace)
			{
				m_lexer.next();
				
				n = new xpath_ast_node(ast_predicate, n, parse_expression());
				
				if (m_lexer.current() != lex_close_square_brace)
	    			throw std::exception("unmatched square brace");
				m_lexer.next();
			}
			
			return n;
	    }
	    
	    // RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
	    xpath_ast_node* parse_relative_location_path(xpath_ast_node* set, bool override_descendant_or_self = false)
	    {
			xpath_ast_node* n = parse_step(set, override_descendant_or_self);
			
			while (m_lexer.current() == lex_slash || m_lexer.current() == lex_double_slash)
			{
				lexeme_t l = m_lexer.current();
				m_lexer.next();

				n = parse_step(n, l == lex_double_slash);
			}
			
			return n;
	    }
	    
	    // LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
	    // AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
	    xpath_ast_node* parse_location_path()
	    {
			if (m_lexer.current() == lex_slash)
			{
				// Save state for next lexeme - that is, whatever follows '/'
				const char* state = m_lexer.state();
				
				m_lexer.next();
				
				xpath_ast_node* n = new xpath_ast_node(ast_step_root);
				
				try
				{
					n = parse_relative_location_path(n);
				}
				catch (const std::exception&)
				{
					m_lexer.reset(state);
				}
				
				return n;
			}
			else if (m_lexer.current() == lex_double_slash)
			{
				m_lexer.next();
				
				return parse_relative_location_path(new xpath_ast_node(ast_step_root_or_descendant));
			}
			else
			{
				return parse_relative_location_path(0);
			}
	    }
	    
	    // PathExpr ::= LocationPath
	    //				| FilterExpr
	    //				| FilterExpr '/' RelativeLocationPath
	    //				| FilterExpr '//' RelativeLocationPath
	    xpath_ast_node* parse_path_expression()
	    {
			// Clarification.
			// PathExpr begins with either LocationPath or FilterExpr.
			// FilterExpr begins with PrimaryExpr
			// PrimaryExpr begins with '$' in case of it being a variable reference,
			// '(' in case of it being an expression, string literal, number constant or
			// function call.

			if (m_lexer.current() == lex_var_ref || m_lexer.current() == lex_open_brace || 
				m_lexer.current() == lex_quoted_string || m_lexer.current() == lex_number ||
				m_lexer.current() == lex_string)
	    	{
	    		if (m_lexer.current() == lex_string)
	    		{
	    			// This is either a function call, or not - if not, we shall proceed with location path
	    			const char* state = m_lexer.state();
	    			
	    			while (*state && *state <= 32) ++state;
	    			
	    			if (*state != '(') return parse_location_path();
	    		}
	    		
	    		xpath_ast_node* n = parse_filter_expression();

	    		if (m_lexer.current() == lex_slash)
	    		{
	    			m_lexer.next();
	    			
	    			// just select from location path
	    			return parse_relative_location_path(n);
	    		}
	    		else if (m_lexer.current() == lex_double_slash)
	    		{
	    			m_lexer.next();
	    			
	    			// select from location path with axis override
	    			return parse_relative_location_path(n, true);
	    		}

	    		return n;
	    	}
	    	else return parse_location_path();
	    }

	    // UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
	    xpath_ast_node* parse_union_expression()
	    {
	    	xpath_ast_node* n = parse_path_expression();

	    	while (m_lexer.current() == lex_union)
	    	{
	    		m_lexer.next();

	    		n = new xpath_ast_node(ast_op_union, n, parse_union_expression());
	    	}

	    	return n;
	    }

	    // UnaryExpr ::= UnionExpr | '-' UnaryExpr
	    xpath_ast_node* parse_unary_expression()
	    {
	    	if (m_lexer.current() == lex_minus)
	    	{
	    		m_lexer.next();

	    		return new xpath_ast_node(ast_op_negate, parse_unary_expression());
	    	}
	    	else return parse_union_expression();
	    }
	    
	    // MultiplicativeExpr ::= UnaryExpr
	    //						  | MultiplicativeExpr '*' UnaryExpr
	    //						  | MultiplicativeExpr 'div' UnaryExpr
	    //						  | MultiplicativeExpr 'mod' UnaryExpr
	    xpath_ast_node* parse_multiplicative_expression()
	    {
	    	xpath_ast_node* n = parse_unary_expression();

	    	while (m_lexer.current() == lex_multiply || (m_lexer.current() == lex_string &&
	    		   (!strcmp(m_lexer.contents(), "mod") || !strcmp(m_lexer.contents(), "div"))))
	    	{
	    		ast_type_t op = m_lexer.current() == lex_multiply ? ast_op_multiply :
	    			!strcmp(m_lexer.contents(), "div") ? ast_op_divide : ast_op_mod;
	    		m_lexer.next();

	    		n = new xpath_ast_node(op, n, parse_unary_expression());
	    	}

	    	return n;
	    }

	    // AdditiveExpr ::= MultiplicativeExpr
	    //					| AdditiveExpr '+' MultiplicativeExpr
	    //					| AdditiveExpr '-' MultiplicativeExpr
	    xpath_ast_node* parse_additive_expression()
	    {
	    	xpath_ast_node* n = parse_multiplicative_expression();

	    	while (m_lexer.current() == lex_plus || m_lexer.current() == lex_minus)
	    	{
	    		lexeme_t l = m_lexer.current();

	    		m_lexer.next();

	    		n = new xpath_ast_node(l == lex_plus ? ast_op_add : ast_op_subtract, n, parse_multiplicative_expression());
	    	}

	    	return n;
	    }

	    // RelationalExpr ::= AdditiveExpr
	    //					  | RelationalExpr '<' AdditiveExpr
	    //					  | RelationalExpr '>' AdditiveExpr
	    //					  | RelationalExpr '<=' AdditiveExpr
	    //					  | RelationalExpr '>=' AdditiveExpr
	    xpath_ast_node* parse_relational_expression()
	    {
	    	xpath_ast_node* n = parse_additive_expression();

	    	while (m_lexer.current() == lex_less || m_lexer.current() == lex_less_or_equal || 
	    		   m_lexer.current() == lex_greater || m_lexer.current() == lex_greater_or_equal)
	    	{
	    		lexeme_t l = m_lexer.current();
	    		m_lexer.next();

	    		n = new xpath_ast_node(l == lex_less ? ast_op_less : l == lex_greater ? ast_op_greater :
	    						l == lex_less_or_equal ? ast_op_less_or_equal : ast_op_greater_or_equal,
	    						n, parse_additive_expression());
	    	}

	    	return n;
	    }
	    
	    // EqualityExpr ::= RelationalExpr
	    //					| EqualityExpr '=' RelationalExpr
	    //					| EqualityExpr '!=' RelationalExpr
	    xpath_ast_node* parse_equality_expression()
	    {
	    	xpath_ast_node* n = parse_relational_expression();

	    	while (m_lexer.current() == lex_equal || m_lexer.current() == lex_not_equal)
	    	{
	    		lexeme_t l = m_lexer.current();

	    		m_lexer.next();

	    		n = new xpath_ast_node(l == lex_equal ? ast_op_equal : ast_op_not_equal, n, parse_relational_expression());
	    	}

	    	return n;
	    }
	    
	    // AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
	    xpath_ast_node* parse_and_expression()
	    {
	    	xpath_ast_node* n = parse_equality_expression();

	    	while (m_lexer.current() == lex_string && !strcmp(m_lexer.contents(), "and"))
	    	{
	    		m_lexer.next();

	    		n = new xpath_ast_node(ast_op_and, n, parse_equality_expression());
	    	}

	    	return n;
	    }

	    // OrExpr ::= AndExpr | OrExpr 'or' AndExpr
	    xpath_ast_node* parse_or_expression()
	    {
	    	xpath_ast_node* n = parse_and_expression();

	    	while (m_lexer.current() == lex_string && !strcmp(m_lexer.contents(), "or"))
	    	{
	    		m_lexer.next();

	    		n = new xpath_ast_node(ast_op_or, n, parse_and_expression());
	    	}

	    	return n;
	    }
		
		// Expr ::= OrExpr
		xpath_ast_node* parse_expression()
		{
			return parse_or_expression();
		}

	public:
		explicit xpath_parser(const char* query): m_lexer(query)
		{
		}

		xpath_ast_node* parse()
		{
			return parse_expression();
		}
	};

	xpath_query::xpath_query(const char* query): m_root(0)
	{
		compile(query);
	}

	bool xpath_query::compile(const char* query)
	{
		delete m_root;
		m_root = 0;

		xpath_parser p(query);

		try
		{
			m_root = p.parse();
			m_root->check_semantics();
		}
		catch (const std::exception& e)
		{
			delete m_root;
			m_root = 0;

			std::cout << "Parse error: " << e.what() << std::endl;
		}

		return !!m_root;
	}

	xpath_result xpath_query::evaluate(const xml_node& n)
	{
		if (!m_root) return xpath_result(false);
		
		xpath_context c;
		
		c.root = n;
		while (c.root.parent())
			c.root = c.root.parent();
			
		c.n = n;
		c.position = 1;
		c.size = 1;
		
		switch (m_root->rettype())
		{
		case ast_type_boolean: return xpath_result(m_root->eval_boolean(c));
		case ast_type_number: return xpath_result(m_root->eval_number(c));
		case ast_type_string: return xpath_result(m_root->eval_string(c));
		case ast_type_node_set: return xpath_result(m_root->eval_node_set(c));
		default: return xpath_result(false);
		}
	}

	xpath_node xml_node::select_single_node(const char* query) const
	{
		std::auto_ptr<xpath_query> q(new xpath_query(query));
		return select_single_node(q.get());
	}

	xpath_node xml_node::select_single_node(xpath_query* query) const
	{
		xpath_result r = query->evaluate(*this);
		const xpath_node_set& s = r.as_node_set();
		return s.empty() ? xpath_node() : s[0];
	}

	xpath_node_set xml_node::select_nodes(const char* query) const
	{
		std::auto_ptr<xpath_query> q(new xpath_query(query));
		return select_nodes(q.get());
	}

	xpath_node_set xml_node::select_nodes(xpath_query* query) const
	{
		return query->evaluate(*this).as_node_set();
	}
		
	xpath_result xml_node::evaluate(const char* query) const
	{
		std::auto_ptr<xpath_query> q(new xpath_query(query));
		return evaluate(q.get());
	}

	xpath_result xml_node::evaluate(xpath_query* query) const
	{
		return query->evaluate(*this);
	}
}
