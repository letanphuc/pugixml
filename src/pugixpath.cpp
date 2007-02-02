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

#include <cfloat>
#include <cmath>

#define NOMINMAX
#include <windows.h>

namespace pugi
{
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

	static bool starts_with(const std::string& s, const char* pattern)
	{
		return s.compare(0, strlen(pattern), pattern) == 0;
	}

	class xpath_lexer
	{
	private:
		const char* m_cur;

		std::string m_cur_lexeme_contents;
		lexeme_t m_cur_lexeme;

	public:
		explicit xpath_lexer(const char* query): m_cur(query)
		{
			next();
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
			m_cur_lexeme_contents.erase(m_cur_lexeme_contents.begin(), m_cur_lexeme_contents.end());

			while (*m_cur && *m_cur <= 32) ++m_cur;

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
				else if (*(m_cur+1) >= '0' && *(m_cur+1) <= '9')
				{
					m_cur_lexeme_contents += "0.";

					++m_cur;

					while (*m_cur >= '0' && *m_cur <= '9')
						m_cur_lexeme_contents += *m_cur++;
					
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
					m_cur_lexeme_contents += *m_cur++;
				
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
				if (*m_cur >= '0' && *m_cur <= '9')
				{
					while (*m_cur >= '0' && *m_cur <= '9')
						m_cur_lexeme_contents += *m_cur++;
				
					if (*m_cur == '.' && (*(m_cur+1) >= '0' && *(m_cur+1) <= '9'))
					{
						m_cur_lexeme_contents += *m_cur++;

						while (*m_cur >= '0' && *m_cur <= '9')
							m_cur_lexeme_contents += *m_cur++;
					}

					m_cur_lexeme = lex_number;
				}
				else if ((*m_cur >= 'A' && *m_cur <= 'Z') || (*m_cur >= 'a' && *m_cur <= 'z') || *m_cur < 0 || *m_cur == '_' || *m_cur == ':')
				{
					while ((*m_cur >= 'A' && *m_cur <= 'Z') || (*m_cur >= 'a' && *m_cur <= 'z') || *m_cur < 0
							|| *m_cur == '_' || *m_cur == ':' || *m_cur == '-' || *m_cur == '.'
							|| (*m_cur >= '0' && *m_cur <= '9'))
						m_cur_lexeme_contents += *m_cur++;
				
					while (*m_cur && *m_cur <= 32) ++m_cur;

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
			return m_cur_lexeme_contents.c_str();
		}
	};

	class xpath_node
	{
	private:
		xml_node m_node;
		xml_attribute m_attribute;
	
	public:
		xpath_node()
		{
		}
		
		xpath_node(const xml_node& node): m_node(node)
		{
		}
		
		xpath_node(const xml_attribute& attribute): m_attribute(attribute)
		{
		}

		const xml_node& node() const
		{
			return m_node;
		}
		
		const xml_attribute& attribute() const
		{
			return m_attribute;
		}
	};

	struct xpath_context
	{
		xml_node root;
		xpath_node n;
		size_t position, size;
	};

	struct xpath_node_set
	{
		typedef std::vector<xpath_node> nodes_type;
		
		nodes_type nodes;
		bool forward;

		xpath_node_set(): forward(false)
		{
		}
		
		bool exists(const xpath_node& n) const
		{
			for (nodes_type::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
				if (it->node() == n.node() && it->attribute() == n.attribute())
					return true;
		
			return false;
		}
		
		void push(const xpath_node& n)
		{
			if (!exists(n)) nodes.push_back(n);
		}
		
		void push_fast(const xpath_node& n)
		{
			nodes.push_back(n);
		}
	};

	class xpath_result
	{
	public:
		enum type_t {t_node_set, t_number, t_string, t_boolean};

	private:
		type_t m_type;

		xpath_node_set m_nodeset_value;
		std::string m_string_value;
		double m_number_value;
		bool m_boolean_value;
	
	public:
		explicit xpath_result(const xpath_node_set& value): m_type(t_node_set), m_nodeset_value(value)
		{
		}

		explicit xpath_result(double value): m_type(t_number), m_number_value(value)
		{
		}

		explicit xpath_result(bool value): m_type(t_boolean), m_boolean_value(value)
		{
		}

		explicit xpath_result(const char* value): m_type(t_string), m_string_value(value)
		{
		}

		explicit xpath_result(const std::string& value): m_type(t_string), m_string_value(value)
		{
		}

		const xpath_node_set& as_node_set() const
		{
			if (m_type == t_node_set) return m_nodeset_value;
			else throw std::exception("Can't convert to node set from anything else");
		}

		bool as_boolean() const
		{
			if (m_type == t_boolean) return m_boolean_value;
			else if (m_type == t_number) return m_number_value != 0 && !_isnan(m_number_value);
			else if (m_type == t_string) return !m_string_value.empty();
			else if (m_type == t_node_set) return !m_nodeset_value.nodes.empty();
			else throw std::exception("Unknown type");
		}

		double as_number() const
		{
			if (m_type == t_boolean) return m_boolean_value ? 1.f : 0.f;
			else if (m_type == t_number) return m_number_value;
			else if (m_type == t_string) return atof(m_string_value.c_str());
			else if (m_type == t_node_set) return atof(as_string().c_str());
			else throw std::exception("Unknown type");
		}
		
		static std::string string_value(const xpath_node& na)
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

					struct string_value_concatenator: public xml_tree_walker
					{
						std::string& m_result;

						string_value_concatenator(std::string& result): m_result(result)
						{
						}

						virtual bool begin(const xml_node& n)
						{
							if (n.type() == node_pcdata || n.type() == node_cdata)
								m_result += n.value();
						
							return true;
						}
					};

					string_value_concatenator svc(result);
					n.traverse(svc);
					
					return result;
				}
				else throw std::exception("Unknown node type");
			}
		}

		std::string as_string() const
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
				if (m_nodeset_value.nodes.empty()) return "";
				else
				{
					return string_value(m_nodeset_value.nodes[0]);
				}
			}
			else throw std::exception("Unknown type");
		}
		
		template <template <class> class C> static bool compare(const xpath_result& lhs, const xpath_result& rhs)
		{
			if (lhs.type() == t_node_set && rhs.type() == t_node_set)
			{
				for (xpath_node_set::nodes_type::const_iterator li = lhs.as_node_set().nodes.begin(); li != lhs.as_node_set().nodes.end(); ++li)
				for (xpath_node_set::nodes_type::const_iterator ri = lhs.as_node_set().nodes.begin(); ri != lhs.as_node_set().nodes.end(); ++ri)
				{
					if (C<std::string>()(string_value(*li), string_value(*ri)) == true)
						return true;
				}
				
				return false;
			}
			else if (lhs.type() != t_node_set && rhs.type() == t_node_set)
			{
				if (lhs.type() == t_boolean)
					return C<bool>()(lhs.as_boolean(), rhs.as_boolean());
				else if (lhs.type() == t_number)
				{
					for (xpath_node_set::nodes_type::const_iterator ri = rhs.as_node_set().nodes.begin(); ri != rhs.as_node_set().nodes.end(); ++ri)
					{
						if (C<double>()(lhs.as_number(), atof(string_value(*ri).c_str())) == true)
							return true;
					}
					
					return false;
				}
				else if (lhs.type() == t_string)
				{
					for (xpath_node_set::nodes_type::const_iterator ri = rhs.as_node_set().nodes.begin(); ri != rhs.as_node_set().nodes.end(); ++ri)
					{
						if (C<std::string>()(lhs.as_string(), string_value(*ri)) == true)
							return true;
					}
					
					return false;
				}
				else throw std::exception("Wrong types");
			}
			else if (lhs.type() == t_node_set && rhs.type() != t_node_set)
			{
				if (rhs.type() == t_boolean)
					return C<bool>()(lhs.as_boolean(), rhs.as_boolean());
				else if (rhs.type() == t_number)
				{
					for (xpath_node_set::nodes_type::const_iterator li = lhs.as_node_set().nodes.begin(); li != lhs.as_node_set().nodes.end(); ++li)
					{
						if (C<double>()(atof(string_value(*li).c_str()), rhs.as_number()) == true)
							return true;
					}
					
					return false;
				}
				else if (rhs.type() == t_string)
				{
					for (xpath_node_set::nodes_type::const_iterator li = lhs.as_node_set().nodes.begin(); li != lhs.as_node_set().nodes.end(); ++li)
					{
						if (C<std::string>()(string_value(*li), rhs.as_string()) == true)
							return true;
					}
					
					return false;
				}
				else throw std::exception("Wrong types");
			}
			else throw std::exception("Wrong types");
		}

		bool operator==(const xpath_result& r) const
		{
			if (type() != t_node_set && r.type() != t_node_set)
			{
				if (type() == t_boolean || r.type() == t_boolean)
					return as_boolean() == r.as_boolean();
				else if (type() == t_number || r.type() == t_number)
					return as_number() == r.as_number();
				else if (type() == t_string || r.type() == t_string)
					return as_string() == r.as_string();
				else
					throw std::exception("Wrong types");
			}
			else return compare<std::equal_to>(*this, r);
		}
		
		bool operator!=(const xpath_result& r) const
		{
			if (type() != t_node_set && r.type() != t_node_set)
			{
				if (type() == t_boolean || r.type() == t_boolean)
					return as_boolean() != r.as_boolean();
				else if (type() == t_number || r.type() == t_number)
					return as_number() != r.as_number();
				else if (type() == t_string || r.type() == t_string)
					return as_string() != r.as_string();
				else
					throw std::exception("Wrong types");
			}
			else return compare<std::not_equal_to>(*this, r);
		}

		bool operator<(const xpath_result& r) const
		{
			if (type() != t_node_set && r.type() != t_node_set)
			{
				return as_number() < r.as_number();
			}
			else return compare<std::less>(*this, r);
		}

		bool operator>(const xpath_result& r) const
		{
			if (type() != t_node_set && r.type() != t_node_set)
			{
				return as_number() > r.as_number();
			}
			else return compare<std::greater>(*this, r);
		}

		bool operator<=(const xpath_result& r) const
		{
			if (type() != t_node_set && r.type() != t_node_set)
			{
				return as_number() <= r.as_number();
			}
			else return compare<std::less_equal>(*this, r);
		}

		bool operator>=(const xpath_result& r) const
		{
			if (type() != t_node_set && r.type() != t_node_set)
			{
				return as_number() >= r.as_number();
			}
			else return compare<std::greater_equal>(*this, r);
		}

		type_t type() const
		{
			return m_type;
		}
	};

	enum ast_type_t
	{
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
		ast_constant,					// constant
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

		void push(xpath_node_set& ns, const xml_attribute& a)
		{
			switch (m_test)
			{
			case nodetest_name:
				if (!strcmp(a.name(), m_contents)) ns.push(a);
				break;
				
			case nodetest_all:
				ns.push(a);
				break;
				
			case nodetest_all_in_namespace:
				if (!strncmp(a.name(), m_contents, strlen(m_contents)) && a.name()[strlen(m_contents)] == ':')
					ns.push(a);
				break;
			}
		}
		
		void push(xpath_node_set& ns, const xml_node& n)
		{
			switch (m_test)
			{
			case nodetest_name:
				if (!strcmp(n.name(), m_contents)) ns.push(n);
				break;
				
			case nodetest_type:
				if ((n.type() == node_comment && !strcmp(m_contents, "comment")) ||
					(n.type() == node_pcdata && !strcmp(m_contents, "text")) ||
					(n.type() == node_cdata && !strcmp(m_contents, "text")) ||
					(n.type() == node_pi && !strcmp(m_contents, "processing-instruction")) ||
					(n.type() == node_element && !strcmp(m_contents, "node")))
					ns.push(n);
					break;
					
			case nodetest_pi:
				if (n.type() == node_pi && !strcmp(n.name(), m_contents))
					ns.push(n);
				break;
				
			case nodetest_all:
				ns.push(n);
				break;
				
			case nodetest_all_in_namespace:
				if (!strncmp(n.name(), m_contents, strlen(m_contents)) && n.name()[strlen(m_contents)] == ':')
					ns.push(n);
				break;
			} 
		}

		void fill_attribute(xpath_node_set& ns, const xml_node& n)
		{
			for (xml_attribute a = n.first_attribute(); a; a = a.next_attribute())
				push(ns, a);
		}
		
		void fill_child(xpath_node_set& ns, const xml_node& n)
		{
			fill_attribute(ns, n);

			for (xml_node c = n.first_child(); c; c = c.next_sibling())
				push(ns, c);
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
			m_third(0), m_concat_next(0), m_contents(0)
		{
			set_contents(contents);
		}
		
		xpath_ast_node(ast_type_t type, xpath_ast_node* left = 0, xpath_ast_node* right = 0, xpath_ast_node* third = 0): m_type(type),
			m_left(left), m_right(right), m_third(third), m_concat_next(0), m_contents(0)
		{
		}

		xpath_ast_node(ast_type_t type, xpath_ast_node* left, axis_t axis, nodetest_t test, const char* contents):
			m_type(type), m_left(left), m_right(0), m_third(0), m_concat_next(0), m_contents(0),
			m_axis(axis), m_test(test)
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

		xpath_result evaluate(xpath_context& c)
		{
			switch (m_type)
			{
			case ast_op_or:
				if (m_left->evaluate(c).as_boolean()) return xpath_result(true);
				else return xpath_result(m_right->evaluate(c).as_boolean());
				
			case ast_op_and:
				if (!m_left->evaluate(c).as_boolean()) return xpath_result(false);
				else return xpath_result(m_right->evaluate(c).as_boolean());
				
			case ast_op_equal:
				return xpath_result(m_left->evaluate(c) == m_right->evaluate(c));

			case ast_op_not_equal:
				return xpath_result(m_left->evaluate(c) != m_right->evaluate(c));

			case ast_op_less:
				return xpath_result(m_left->evaluate(c) < m_right->evaluate(c));

			case ast_op_greater:
				return xpath_result(m_left->evaluate(c) > m_right->evaluate(c));

			case ast_op_less_or_equal:
				return xpath_result(m_left->evaluate(c) <= m_right->evaluate(c));

			case ast_op_greater_or_equal:
				return xpath_result(m_left->evaluate(c) >= m_right->evaluate(c));

			case ast_op_add:
				return xpath_result(m_left->evaluate(c).as_number() + m_right->evaluate(c).as_number());

			case ast_op_subtract:
				return xpath_result(m_left->evaluate(c).as_number() - m_right->evaluate(c).as_number());

			case ast_op_multiply:
				return xpath_result(m_left->evaluate(c).as_number() * m_right->evaluate(c).as_number());

			case ast_op_divide:
				return xpath_result(m_left->evaluate(c).as_number() / m_right->evaluate(c).as_number());

			case ast_op_mod:
				return xpath_result(fmod(m_left->evaluate(c).as_number(), m_right->evaluate(c).as_number()));

			case ast_op_negate:
				return xpath_result(-m_left->evaluate(c).as_number());

			case ast_op_union:
				throw std::exception("not implemented for now");

			case ast_predicate:
			{
				xpath_result set = m_left->evaluate(c);
				
				xpath_node_set ns;
			
				const xpath_node_set& nodes = set.as_node_set();
				size_t size = nodes.nodes.size();
			
				xpath_context oc = c;
			
				for (xpath_node_set::nodes_type::const_iterator it = nodes.nodes.begin(); it != nodes.nodes.end(); ++it)
				{
					c.n = *it;
					c.position = std::distance(nodes.nodes.begin(), it) + 1;
					c.size = size;
				
					if (m_right->evaluate(c).as_boolean())
						ns.push(*it);
				}
			
				c = oc;
			
				return xpath_result(ns);
			}

			case ast_variable:
				throw std::exception("Variables not implemented");

			case ast_constant:
				return xpath_result(m_contents);
			
			case ast_func_last:
				return xpath_result((double)c.size);

			case ast_func_position:
				return xpath_result((double)c.position);

			case ast_func_count:
				return xpath_result((double)m_left->evaluate(c).as_node_set().nodes.size());

			case ast_func_id:
				return xpath_result(xpath_node_set());

			case ast_func_name_0:
			case ast_func_local_name_0:
			{
				xpath_node na = c.n;
				
				if (na.attribute()) return xpath_result(na.attribute().name());
				else return xpath_result(na.node().name());
			}

			case ast_func_name_1:
			case ast_func_local_name_1:
			{
				xpath_node na = m_left->evaluate(c).as_node_set().nodes[0];
				
				if (na.attribute()) return xpath_result(na.attribute().name());
				else return xpath_result(na.node().name());
			}

			case ast_func_namespace_uri_0:
				return xpath_result("");

			case ast_func_namespace_uri_1:
				return xpath_result("");

			case ast_func_string_0:
				return xpath_result(xpath_result::string_value(c.n));

			case ast_func_string_1:
				return xpath_result(m_left->evaluate(c).as_string());

			case ast_func_concat:
			{
				std::string r = m_left->evaluate(c).as_string();
				
				for (xpath_ast_node* n = m_right; n; n = n->m_concat_next)
					r += n->evaluate(c).as_string();
			
				return xpath_result(r);
			}
			
			case ast_func_starts_with:
				return xpath_result(starts_with(m_left->evaluate(c).as_string(), m_right->evaluate(c).as_string().c_str()));

			case ast_func_contains:
				return xpath_result(m_left->evaluate(c).as_string().find(m_right->evaluate(c).as_string()) != std::string::npos);

			case ast_func_substring_before:
			{
				std::string s = m_left->evaluate(c).as_string();
				std::string::size_type pos = s.find(m_right->evaluate(c).as_string());
				
				if (pos == std::string::npos) return xpath_result("");
				else return xpath_result(std::string(s.begin(), s.begin() + pos));
			}
			
			case ast_func_substring_after:
			{
				std::string s = m_left->evaluate(c).as_string();
				std::string p = m_right->evaluate(c).as_string();
				
				std::string::size_type pos = s.find(p);
				
				if (pos == std::string::npos) return xpath_result("");
				else return xpath_result(std::string(s.begin() + pos + p.length(), s.end()));
			}

			case ast_func_substring_2:
			{
				std::string s = m_left->evaluate(c).as_string();
				int first = (int)m_right->evaluate(c).as_number();
				
				if (first < 1) first = 1;
				
				return xpath_result(s.substr(first));
			}
			
			case ast_func_substring_3:
			{
				std::string s = m_left->evaluate(c).as_string();
				int first = (int)m_right->evaluate(c).as_number();
				int last = (int)(first + m_third->evaluate(c).as_number());
				
				if (first < 1) first = 1;
				if (last > (int)s.length() + 1) last = (int)s.length() + 1;
				
				return xpath_result(s.substr(first, last - first));
			}

			case ast_func_string_length_0:
				return xpath_result((double)xpath_result::string_value(c.n).length());
			
			case ast_func_string_length_1:
				return xpath_result((double)m_left->evaluate(c).as_string().length());

			case ast_func_normalize_space_0:
			case ast_func_normalize_space_1:
			{
				std::string s = m_type == ast_func_normalize_space_0 ? xpath_result::string_value(c.n) : m_left->evaluate(c).as_string();
				
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
			
				return xpath_result(r);
			}

			case ast_func_translate:
			{
				std::string s = m_left->evaluate(c).as_string();
				std::string from = m_right->evaluate(c).as_string();
				std::string to = m_third->evaluate(c).as_string();
				
				for (std::string::iterator it = s.begin(); it != s.end(); )
				{
					std::string::size_type pos = from.find(*it);
					
					if (pos != std::string::npos && pos >= to.length())
						it = s.erase(it);
					else if (pos != std::string::npos)
						*it = to[pos];
				}
				
				return xpath_result(s);
			}

			case ast_func_boolean:
				return xpath_result(m_left->evaluate(c).as_boolean());

			case ast_func_not:
				return xpath_result(!m_left->evaluate(c).as_boolean());

			case ast_func_true:
				return xpath_result(true);

			case ast_func_false:
				return xpath_result(false);

			case ast_func_lang:
			{
				if (c.n.attribute()) return xpath_result(false);
				
				std::string lang = m_left->evaluate(c).as_string();
				std::string lang_prefixed = lang + "-";
				
				xml_node n = c.n.node();
				
				while (n.type() != node_document)
				{
					xml_attribute a = n.attribute("xml:lang");
					
					if (a)
					{
						const char* value = a.value();
						
						return xpath_result(!stricmp(value, lang.c_str()) || !strnicmp(value, lang_prefixed.c_str(), lang_prefixed.length()));
					}
				}
				
				return xpath_result(false);
			}
			
			case ast_func_number_0:
				return xpath_result(atof(xpath_result::string_value(c.n).c_str()));
			
			case ast_func_number_1:
				return xpath_result(m_left->evaluate(c).as_number());

			case ast_func_sum:
			{
				double r = 0;
				
				xpath_result v = m_left->evaluate(c);
				const xpath_node_set& ns = v.as_node_set();
				
				for (xpath_node_set::nodes_type::const_iterator it = ns.nodes.begin(); it != ns.nodes.end(); ++it)
					r += atof(xpath_result::string_value(*it).c_str());
			
				return xpath_result(r);
			}
			
			case ast_func_floor:
				return xpath_result(floor(m_left->evaluate(c).as_number()));

			case ast_func_ceiling:
				return xpath_result(ceil(m_left->evaluate(c).as_number()));

			case ast_func_round:
				return xpath_result(floor(m_left->evaluate(c).as_number() + 0.5));

			case ast_step:
			{
				xpath_node_set ns;
			
				switch (m_axis)
				{
				case axis_child:
					if (m_left)
					{
						xpath_result r = m_left->evaluate(c);
						const xpath_node_set& s = r.as_node_set();
						
						for (xpath_node_set::nodes_type::const_iterator it = s.nodes.begin(); it != s.nodes.end(); ++it)
							if (it->node())
								fill_child(ns, it->node());
					}
					else if (c.n.node()) fill_child(ns, c.n.node());
					
					break;
				
				case axis_attribute:
					if (m_left)
					{
						xpath_result r = m_left->evaluate(c);
						const xpath_node_set& s = r.as_node_set();
						
						for (xpath_node_set::nodes_type::const_iterator it = s.nodes.begin(); it != s.nodes.end(); ++it)
							if (it->node())
								fill_attribute(ns, it->node());
					}
					else if (c.n.node()) fill_attribute(ns, c.n.node());
					
					break;
	
				default:
					throw std::exception("Not implemented");
				}
				
				return xpath_result(ns);

				break;
			}

			case ast_step_root:
			{
				xpath_node_set ns;
			
				ns.push(c.root);
			
				return xpath_result(ns);
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
						m_dest.push(n);
						
						for (xml_attribute a = n.first_attribute(); a; a = a.next_attribute())
							m_dest.push(a);
						
						return true;
					}
				};
			
				c.root.traverse(tree_traverser(ns));
			
				return xpath_result(ns);
			}

			default:
				throw std::exception("Unknown operation");
			}
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
			case lex_number:
			{
				xpath_ast_node* n = new xpath_ast_node(ast_constant, m_lexer.contents());
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
				
				switch (m_function[0])
				{
				case 'b':
				{
					if (m_function == "boolean" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_boolean, m_args[0]);
						
					break;
				}
				
				case 'c':
				{
					if (m_function == "count" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_count, m_args[0]);
					else if (m_function == "contains" && m_args.size() == 2)
						return new xpath_ast_node(ast_func_contains, m_args[0], m_args[1]);
					else if (m_function == "concat" && m_args.size() >= 2)
					{
						for (size_t i = 1; i + 1 < m_args.size(); ++i)
							m_args[i]->set_concat_next(m_args[i+1]);

						return new xpath_ast_node(ast_func_concat, m_args[0], m_args[1]);
					}
					else if (m_function == "ceiling" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_ceiling, m_args[0]);
						
					break;
				}
				
				case 'f':
				{
					if (m_function == "false" && m_args.size() == 0)
						return new xpath_ast_node(ast_func_false);
					else if (m_function == "floor" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_floor, m_args[0]);
						
					break;
				}
				
				case 'i':
				{
					if (m_function == "id" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_id, m_args[0]);
						
					break;
				}
				
				case 'l':
				{
					if (m_function == "last" && m_args.size() == 0)
						return new xpath_ast_node(ast_func_last);
					else if (m_function == "lang" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_lang, m_args[0]);
					else if (m_function == "local-name" && m_args.size() == 0)
						return new xpath_ast_node(ast_func_local_name_0);
					else if (m_function == "local-name" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_local_name_1, m_args[0]);
				
					break;
				}
				
				case 'n':
				{
					if (m_function == "name" && m_args.size() == 0)
						return new xpath_ast_node(ast_func_name_0);
					else if (m_function == "name" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_name_1, m_args[0]);
					else if (m_function == "namespace-uri" && m_args.size() == 0)
						return new xpath_ast_node(ast_func_namespace_uri_0);
					else if (m_function == "namespace-uri" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_namespace_uri_1, m_args[0]);
					else if (m_function == "normalize-space" && m_args.size() == 0)
						return new xpath_ast_node(ast_func_normalize_space_0);
					else if (m_function == "normalize-space" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_normalize_space_1, m_args[0]);
					else if (m_function == "not" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_not, m_args[0]);
					else if (m_function == "number" && m_args.size() == 0)
						return new xpath_ast_node(ast_func_number_0);
					else if (m_function == "number" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_number_1, m_args[0]);
				
					break;
				}
				
				case 'p':
				{
					if (m_function == "position" && m_args.size() == 0)
						return new xpath_ast_node(ast_func_position);
					
					break;
				}
				
				case 'r':
				{
					if (m_function == "round" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_round, m_args[0]);

					break;
				}
				
				case 's':
				{
					if (m_function == "string" && m_args.size() == 0)
						return new xpath_ast_node(ast_func_string_0);
					else if (m_function == "string" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_string_1, m_args[0]);
					else if (m_function == "string-length" && m_args.size() == 0)
						return new xpath_ast_node(ast_func_string_length_0);
					else if (m_function == "string-length" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_string_length_1, m_args[0]);
					else if (m_function == "starts-with" && m_args.size() == 2)
						return new xpath_ast_node(ast_func_starts_with, m_args[0], m_args[1]);
					else if (m_function == "substring-before" && m_args.size() == 2)
						return new xpath_ast_node(ast_func_substring_before, m_args[0], m_args[1]);
					else if (m_function == "substring-after" && m_args.size() == 2)
						return new xpath_ast_node(ast_func_substring_after, m_args[0], m_args[1]);
					else if (m_function == "substring" && m_args.size() == 2)
						return new xpath_ast_node(ast_func_substring_2, m_args[0], m_args[1]);
					else if (m_function == "substring" && m_args.size() == 3)
						return new xpath_ast_node(ast_func_substring_3, m_args[0], m_args[1], m_args[2]);
					else if (m_function == "sum" && m_args.size() == 1)
						return new xpath_ast_node(ast_func_sum, m_args[0]);

					break;
				}
				
				case 't':
				{
					if (m_function == "translate" && m_args.size() == 3)
						return new xpath_ast_node(ast_func_translate, m_args[0], m_args[1], m_args[2]);
					else if (m_function == "true" && m_args.size() == 0)
						return new xpath_ast_node(ast_func_true);
						
					break;
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
				
				return set;
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
				
				if (starts_with(nt_name, "ancestor::")) axis = axis_ancestor;
				else if (starts_with(nt_name, "ancestor-or-self::")) axis = axis_ancestor_or_self;
				else if (starts_with(nt_name, "attribute::")) axis = axis_attribute;
				else if (starts_with(nt_name, "child::")) axis = axis_child;
				else if (starts_with(nt_name, "descendant::")) axis = axis_descendant;
				else if (starts_with(nt_name, "descendant-or-self::")) axis = axis_descendant_or_self;
				else if (starts_with(nt_name, "following::")) axis = axis_following;
				else if (starts_with(nt_name, "following-sibling::")) axis = axis_following_sibling;
				else if (starts_with(nt_name, "namespace::")) axis = axis_namespace;
				else if (starts_with(nt_name, "parent::")) axis = axis_parent;
				else if (starts_with(nt_name, "preceding::")) axis = axis_preceding;
				else if (starts_with(nt_name, "preceding-sibling::")) axis = axis_preceding_sibling;
				else if (starts_with(nt_name, "self::")) axis = axis_ancestor_or_self;
				else axis_specified = false;
				
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

	class xpath_query
	{
	private:
		// Noncopyable semantics
		xpath_query(const xpath_query&);
		xpath_query& operator=(const xpath_query&);

		xpath_ast_node* m_root;

	public:
		explicit xpath_query(const char* query): m_root(0)
		{
			compile(query);
		}

		bool compile(const char* query)
		{
			delete m_root;

			xpath_parser p(query);

			try
			{
				m_root = p.parse();
			}
			catch (const std::exception& e)
			{
				m_root = 0;

				std::cout << "Parse error: " << e.what() << std::endl;
			}

			return !!m_root;
		}

		xpath_result evaluate(const xml_node& n)
		{
			if (!m_root) return xpath_result(false);
			
			xpath_context c;
			
			c.root = n;
			c.n = n;
			c.position = 1;
			c.size = 1;
			
			return m_root->evaluate(c);
		}
	};
}

size_t g_size = 0;
size_t g_max_size = 0;
size_t g_count = 0;

void* operator new(size_t size)
{
	g_size += size;
	g_max_size = std::max(g_size, g_max_size);
	++g_count;
	return malloc(size);
}

void* operator new[](size_t size)
{
	g_size += size;
	g_max_size = std::max(g_size, g_max_size);
	++g_count;
	return malloc(size);
}

void operator delete(void* ptr)
{
	if (ptr)
	{
		g_size -= _msize(ptr);
		free(ptr);
	}
}

void operator delete[](void* ptr)
{
	if (ptr)
	{
		g_size -= _msize(ptr);
		free(ptr);
	}
}

int main()
{
	std::ifstream in("xpath.xml", std::ios::in | std::ios::binary);
	
	std::cout << "Opened file: " << g_count << " allocs, " << g_size << " bytes, peak " << g_max_size << " bytes" << std::endl;
	g_count = g_size = g_max_size = 0;
	
	pugi::xml_parser parser(in);
	
	std::cout << "Parsed file: " << g_count << " allocs, " << g_size << " bytes, peak " << g_max_size << " bytes" << std::endl;
	g_count = g_size = g_max_size = 0;
	
	pugi::xpath_query* q = new pugi::xpath_query("sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)");

	std::cout << "Compiled query: " << g_count << " allocs, " << g_size << " bytes, peak " << g_max_size << " bytes" << std::endl;
	g_count = g_size = g_max_size = 0;
	
	// q->dump();
	
	std::cout << "Dumped query: " << g_count << " allocs, " << g_size << " bytes, peak " << g_max_size << " bytes" << std::endl;
	g_count = g_size = g_max_size = 0;

	pugi::xpath_result r = q->evaluate(parser.document());
	
	std::cout << "Computed result: " << g_count << " allocs, " << g_size << " bytes, peak " << g_max_size << " bytes" << std::endl;
	g_count = g_size = g_max_size = 0;

	std::cout << r.as_number() << std::endl;
	
	std::cout << "Total node count: " << (new pugi::xpath_query("count(//*)"))->evaluate(parser.document()).as_number() << std::endl;
	std::cout << "Total tag count: " << (new pugi::xpath_query("count(//node())"))->evaluate(parser.document()).as_number() << std::endl;
	
	// sum(/bookstore/book[author/last-name = 'Bob']/price)
	// result should be 12 + 55 + 6.50 + 29.50 = 103

#ifndef NDEBUG
	return 0;
#endif

	// benchmark
	unsigned __int64 time = 0;
	
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		
	for (int i = 0; i < 128; ++i)
	{
		LARGE_INTEGER start, end;
		
		QueryPerformanceCounter(&start);
		for (int j = 0; j < 128; ++j)
		{
			delete new pugi::xpath_query("sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)+sum(/bookstore/book[author/last-name = 'Bob']/price)");
		}
		QueryPerformanceCounter(&end);
		
		time += end.QuadPart - start.QuadPart;
	}
	
	std::cout << "Time to compile: " << time / 128 << std::endl;

	time = 0;
	
	for (int i = 0; i < 16; ++i)
	{
		LARGE_INTEGER start, end;
		
		QueryPerformanceCounter(&start);
		for (int j = 0; j < 128; ++j)
		{
			q->evaluate(parser.document());
		}
		QueryPerformanceCounter(&end);
		
		time += end.QuadPart - start.QuadPart;
	}

	std::cout << "Time to execute: " << time / 16 << std::endl;

	delete q;
}
