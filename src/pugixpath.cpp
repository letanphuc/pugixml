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

#include <iostream>
#include <fstream>
#include <vector>

namespace pugi
{
	namespace xpath
	{
		enum lexeme_t
		{
			lex_none = 0,
			lex_or,
			lex_and,
			lex_equal,
			lex_not_equal,
			lex_less,
			lex_greater,
			lex_less_or_equal,
			lex_greater_or_equal,
			lex_plus,
			lex_minus,
			lex_multiply,
			lex_div,
			lex_mod,
			lex_union,
			lex_var_ref,
			lex_open_brace,
			lex_close_brace,
			lex_quoted_string,
			lex_number,
			lex_function_call,
			lex_slash,
			lex_double_slash,
			lex_open_square_brace,
			lex_close_square_brace,
			lex_string,
			lex_comma,
			lex_axis,
			lex_axis_attribute,
			lex_dot,
			lex_double_dot
		};

		class lexer
		{
		private:
			const char* m_cur;
	
			std::string m_cur_lexeme_contents;
			lexeme_t m_cur_lexeme;
	
		public:
			explicit lexer(const char* query): m_cur(query)
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
					else if (*m_cur == '.' && (*(m_cur+1) >= '0' && *(m_cur+1) <= '9'))
					{
						m_cur_lexeme_contents += "0.";

						++m_cur;

						while (*m_cur >= '0' && *m_cur <= '9')
							m_cur_lexeme_contents += *m_cur++;
					}
					else if ((*m_cur >= 'A' && *m_cur <= 'Z') || (*m_cur >= 'a' && *m_cur <= 'z') || *m_cur < 0 || *m_cur == '_' || *m_cur == ':')
					{
						while ((*m_cur >= 'A' && *m_cur <= 'Z') || (*m_cur >= 'a' && *m_cur <= 'z') || *m_cur < 0
								|| *m_cur == '_' || *m_cur == ':' || *m_cur == '-' || *m_cur == '.'
								|| (*m_cur >= '0' && *m_cur <= '9'))
							m_cur_lexeme_contents += *m_cur++;
					
						while (*m_cur && *m_cur <= 32) ++m_cur;

						if (m_cur_lexeme_contents == "or")
							m_cur_lexeme = lex_or;
						else if (m_cur_lexeme_contents == "and")
							m_cur_lexeme = lex_and;
						else if (m_cur_lexeme_contents == "div")
							m_cur_lexeme = lex_div;
						else if (m_cur_lexeme_contents == "mod")
							m_cur_lexeme = lex_mod;
						else if (*m_cur == '(')
						{
							++m_cur;

							m_cur_lexeme = lex_function_call;
						}
						else if (*m_cur == ':' && *(m_cur + 1) == ':')
						{
							m_cur += 2;
							
							m_cur_lexeme = lex_axis;
						}
						else
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

		class node
		{
		private:
			node* m_left;
			node* m_right;

			node(const node&);
			node& operator=(const node&);

		protected:
			node(node* left, node* right): m_left(left), m_right(right)
			{
			}

		public:
			node* left() const
			{
				return m_left;
			}

			node* right() const
			{
				return m_right;
			}

			virtual void dump(const std::string& indent)
			{
				std::cout << indent << "node" << std::endl;
				if (m_left) m_left->dump(indent + "\t");
				if (m_right) m_right->dump(indent + "\t");
			}
			
			void dump()
			{
				dump("");
			}
		};

		enum op_t
		{
			op_or,
			op_and,
			op_equal,
			op_not_equal,
			op_less,
			op_greater,
			op_less_or_equal,
			op_greater_or_equal,
			op_add,
			op_subtract,
			op_multiply,
			op_divide,
			op_mod,
			op_negate,
			op_union
		};

		class op_node: public node
		{
		private:
			op_t m_op;

		public:
			op_node(op_t op, node* left, node* right): node(left, right), m_op(op)
			{
			}
			
			virtual void dump(const std::string& indent)
			{
				std::cout << indent << "op_node " << m_op << std::endl;
				if (left()) left()->dump(indent + "\t");
				if (right()) right()->dump(indent + "\t");
			}
		};

		class predicate_node: public node
		{
		public:
			predicate_node(node* left, node* right): node(left, right)
			{
			}
			
			virtual void dump(const std::string& indent)
			{
				std::cout << indent << "predicate_node" << std::endl;
				if (left()) left()->dump(indent + "\t");
				if (right()) right()->dump(indent + "\t");
			}
		};

		class variable_node: public node
		{
		private:
			std::string m_name;

		public:
			explicit variable_node(const char* name): node(0, 0), m_name(name)
			{
			}
			
			virtual void dump(const std::string& indent)
			{
				std::cout << indent << "variable_node $" << m_name << std::endl;
			}
		};

		class constant_node: public node
		{
		private:
			std::string m_value;

		public:
			explicit constant_node(const char* value): node(0, 0), m_value(value)
			{
			}
			
			virtual void dump(const std::string& indent)
			{
				std::cout << indent << "constant_node =" << m_value << std::endl;
			}
		};

		class function_node: public node
		{
		private:
			std::string m_name;
			std::vector<node*> m_arguments;

		public:
			explicit function_node(const char* name): node(0, 0), m_name(name)
			{
			}

			void add(node* n)
			{
				m_arguments.push_back(n);
			}
			
			virtual void dump(const std::string& indent)
			{
				std::cout << indent << "function_node " << m_name << std::endl;
				
				for (size_t i = 0; i < m_arguments.size(); ++i)
				{
					std::cout << indent << "arg" << i << std::endl;
					m_arguments[i]->dump(indent + "\t");
				}
			}
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
		
		struct node_test
		{
			enum type_t {nt_name, nt_type, nt_pi};
			
			type_t type;
			std::string name;
		};
		
		class step_node: public node
		{
		private:
			axis_t m_axis;
			node_test m_test;
			
		public:
			step_node(axis_t axis, const node_test& test, node* set): node(set, 0), m_axis(axis), m_test(test)
			{
			}
			
			virtual void dump(const std::string& indent)
			{
				std::cout << indent << "step_node axis " << m_axis << " test " << m_test.type << "," << m_test.name << std::endl;
				if (left()) left()->dump(indent + "\t");
			}
		};

		class root_step_node: public node
		{
		public:
			root_step_node(): node(0, 0)
			{
			}
			
			virtual void dump(const std::string& indent)
			{
				std::cout << indent << "root_step_node" << std::endl;
			}
		};

		class root_or_descendant_step_node: public node
		{
		public:
			root_or_descendant_step_node(): node(0, 0)
			{
			}
			
			virtual void dump(const std::string& indent)
			{
				std::cout << indent << "root_or_descendant_step_node" << std::endl;
			}
		};

		class parser
		{
		private:
		    lexer m_lexer;

		    // PrimaryExpr ::= VariableReference | '(' Expr ')' | Literal | Number | FunctionCall
		    node* parse_primary_expression()
		    {
		    	switch (m_lexer.current())
		    	{
		    	case lex_var_ref:
		    	{
		    		m_lexer.next();

		    		if (m_lexer.current() != lex_string)
		    			throw std::exception("incorrect variable reference");

					node* n = new variable_node(m_lexer.contents());
					m_lexer.next();

					return n;
				}

				case lex_open_brace:
				{
					m_lexer.next();

					node* n = parse_expression();

					if (m_lexer.current() != lex_close_brace)
						throw std::exception("unmatched braces");

					m_lexer.next();

					return n;
				}

				case lex_quoted_string:
				case lex_number:
				{
					node* n = new constant_node(m_lexer.contents());
					m_lexer.next();

					return n;
				}

				case lex_function_call:
				{
					function_node* f = new function_node(m_lexer.contents());
					m_lexer.next();

					if (m_lexer.current() != lex_close_brace)
						f->add(parse_expression());

					while (m_lexer.current() != lex_close_brace)
					{
						if (m_lexer.current() != lex_comma)
							throw std::exception("no comma between function arguments");
						m_lexer.next();
						
						f->add(parse_expression());
					}
					
					m_lexer.next();

					return f;
				}

		    	default:
		    		throw std::exception("unrecognizable primary expression");
		    	}
		    }
		    
		    // FilterExpr ::= PrimaryExpr | FilterExpr Predicate
		    // Predicate ::= '[' PredicateExpr ']'
		    // PredicateExpr ::= Expr
		    node* parse_filter_expression()
		    {
		    	node* n = parse_primary_expression();

		    	while (m_lexer.current() == lex_open_square_brace)
		    	{
		    		m_lexer.next();

		    		n = new predicate_node(n, parse_expression());

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
		    node* parse_step(node* set, bool override_descendant_or_self = false)
		    {
				axis_t axis;
				
				if (m_lexer.current() == lex_axis)
				{
					// Explicitly specified axis
					const char* s = m_lexer.contents();
			
					if (!strcmp(s, "ancestor")) axis = axis_ancestor;
					else if (!strcmp(s, "ancestor-or-self")) axis = axis_ancestor_or_self;
					else if (!strcmp(s, "attribute")) axis = axis_attribute;
					else if (!strcmp(s, "child")) axis = axis_child;
					else if (!strcmp(s, "descendant")) axis = axis_descendant;
					else if (!strcmp(s, "descendant-or-self")) axis = axis_descendant_or_self;
					else if (!strcmp(s, "following")) axis = axis_following;
					else if (!strcmp(s, "following-sibling")) axis = axis_following_sibling;
					else if (!strcmp(s, "namespace")) axis = axis_namespace;
					else if (!strcmp(s, "parent")) axis = axis_parent;
					else if (!strcmp(s, "preceding")) axis = axis_preceding;
					else if (!strcmp(s, "preceding-sibling")) axis = axis_preceding_sibling;
					else if (!strcmp(s, "self")) axis = axis_ancestor_or_self;
					else throw std::exception("Unknown axis name");
					
					m_lexer.next();
				}
				else if (m_lexer.current() == lex_axis_attribute)
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
					
					node_test nt = {node_test::nt_name, "*"};
					
					return new step_node(axis_parent, nt, set);
				}
				else // implied child axis
					axis = override_descendant_or_self ? axis_descendant_or_self : axis_child;
		    
				node_test nt;
				
				if (m_lexer.current() == lex_function_call)
				{
					// node type test or processing-instruction
					
					if (!strcmp(m_lexer.contents(), "processing-instruction"))
					{
						m_lexer.next();
						
						if (m_lexer.current() != lex_quoted_string)
							throw std::exception("Only literals are allowed as arguments to processing-instruction()");
					
						nt.type = node_test::nt_pi;
						nt.name = m_lexer.contents();
						m_lexer.next();
						
						if (m_lexer.current() != lex_close_brace)
							throw std::exception("Unmatched brace near processing-instruction()");
						m_lexer.next();
					}
					else
					{
						nt.type = node_test::nt_type;
						nt.name = m_lexer.contents();
						m_lexer.next();
						
						if (m_lexer.current() != lex_close_brace)
							throw std::exception("Unmatched brace near node type test");
						m_lexer.next();
					}
				}
				else if (m_lexer.current() == lex_string)
				{
					// node name test
					nt.type = node_test::nt_name;
					nt.name = m_lexer.contents();
					m_lexer.next();
					
					// namespace *
					if (m_lexer.current() == lex_multiply)
					{
						nt.name += '*';
						m_lexer.next();
					}
				}
				else if (m_lexer.current() == lex_multiply)
				{
					nt.type = node_test::nt_name;
					nt.name = "*";
					m_lexer.next();
				}
				else throw std::exception("Unrecognized node test");
				
				node* n = new step_node(axis, nt, set);
				
				while (m_lexer.current() == lex_open_square_brace)
				{
					m_lexer.next();
					
					n = new predicate_node(n, parse_expression());
					
					if (m_lexer.current() != lex_close_square_brace)
		    			throw std::exception("unmatched square brace");
					m_lexer.next();
				}
				
				return n;
		    }
		    
		    // RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
		    node* parse_relative_location_path(node* set, bool override_descendant_or_self = false)
		    {
				node* n = parse_step(set, override_descendant_or_self);
				
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
		    node* parse_location_path()
		    {
				if (m_lexer.current() == lex_slash)
				{
					// Save state for next lexeme - that is, whatever follows '/'
					const char* state = m_lexer.state();
					
					m_lexer.next();
					
					node* n = new root_step_node;
					
					try
					{
						n = parse_relative_location_path(n);
					}
					catch (const std::exception& e)
					{
						m_lexer.reset(state);
					}
					
					return n;
				}
				else if (m_lexer.current() == lex_double_slash)
				{
					return parse_relative_location_path(new root_or_descendant_step_node);
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
		    node* parse_path_expression()
		    {
				// Clarification.
				// PathExpr begins with either LocationPath or FilterExpr.
				// FilterExpr begins with PrimaryExpr
				// PrimaryExpr begins with '$' in case of it being a variable reference,
				// '(' in case of it being an expression, string literal, number constant or
				// function call.

				if (m_lexer.current() == lex_var_ref || m_lexer.current() == lex_open_brace || 
					m_lexer.current() == lex_quoted_string || m_lexer.current() == lex_number ||
					m_lexer.current() == lex_function_call)
		    	{
		    		node* n = parse_filter_expression();

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
		    node* parse_union_expression()
		    {
		    	node* n = parse_path_expression();

		    	while (m_lexer.current() == lex_union)
		    	{
		    		m_lexer.next();

		    		n = new op_node(op_union, n, parse_union_expression());
		    	}

		    	return n;
		    }

		    // UnaryExpr ::= UnionExpr | '-' UnaryExpr
		    node* parse_unary_expression()
		    {
		    	if (m_lexer.current() == lex_minus)
		    	{
		    		m_lexer.next();

		    		return new op_node(op_negate, parse_unary_expression(), 0);
		    	}
		    	else return parse_union_expression();
		    }
		    
		    // MultiplicativeExpr ::= UnaryExpr
		    //						  | MultiplicativeExpr '*' UnaryExpr
		    //						  | MultiplicativeExpr 'div' UnaryExpr
		    //						  | MultiplicativeExpr 'mod' UnaryExpr
		    node* parse_multiplicative_expression()
		    {
		    	node* n = parse_unary_expression();

		    	while (m_lexer.current() == lex_multiply || m_lexer.current() == lex_div ||
		    		   m_lexer.current() == lex_mod)
		    	{
		    		lexeme_t l = m_lexer.current();
		    		m_lexer.next();

		    		n = new op_node(l == lex_multiply ? op_multiply : l == lex_div ? op_divide : op_mod,
		    						n, parse_unary_expression());
		    	}

		    	return n;
		    }

		    // AdditiveExpr ::= MultiplicativeExpr
		    //					| AdditiveExpr '+' MultiplicativeExpr
		    //					| AdditiveExpr '-' MultiplicativeExpr
		    node* parse_additive_expression()
		    {
		    	node* n = parse_multiplicative_expression();

		    	while (m_lexer.current() == lex_plus || m_lexer.current() == lex_minus)
		    	{
		    		lexeme_t l = m_lexer.current();

		    		m_lexer.next();

		    		n = new op_node(l == lex_plus ? op_add : op_subtract, n, parse_multiplicative_expression());
		    	}

		    	return n;
		    }

		    // RelationalExpr ::= AdditiveExpr
		    //					  | RelationalExpr '<' AdditiveExpr
		    //					  | RelationalExpr '>' AdditiveExpr
		    //					  | RelationalExpr '<=' AdditiveExpr
		    //					  | RelationalExpr '>=' AdditiveExpr
		    node* parse_relational_expression()
		    {
		    	node* n = parse_additive_expression();

		    	while (m_lexer.current() == lex_less || m_lexer.current() == lex_less_or_equal || 
		    		   m_lexer.current() == lex_greater || m_lexer.current() == lex_greater_or_equal)
		    	{
		    		lexeme_t l = m_lexer.current();
		    		m_lexer.next();

		    		n = new op_node(l == lex_less ? op_less : l == lex_greater ? op_greater :
		    						l == lex_less_or_equal ? op_less_or_equal : op_greater_or_equal,
		    						n, parse_additive_expression());
		    	}

		    	return n;
		    }
		    
		    // EqualityExpr ::= RelationalExpr
		    //					| EqualityExpr '=' RelationalExpr
		    //					| EqualityExpr '!=' RelationalExpr
		    node* parse_equality_expression()
		    {
		    	node* n = parse_relational_expression();

		    	while (m_lexer.current() == lex_equal || m_lexer.current() == lex_not_equal)
		    	{
		    		lexeme_t l = m_lexer.current();

		    		m_lexer.next();

		    		n = new op_node(l == lex_equal ? op_equal : op_not_equal, n, parse_relational_expression());
		    	}

		    	return n;
		    }
		    
		    // AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
		    node* parse_and_expression()
		    {
		    	node* n = parse_equality_expression();

		    	while (m_lexer.current() == lex_and)
		    	{
		    		m_lexer.next();

		    		n = new op_node(op_and, n, parse_equality_expression());
		    	}

		    	return n;
		    }

		    // OrExpr ::= AndExpr | OrExpr 'or' AndExpr
		    node* parse_or_expression()
		    {
		    	node* n = parse_and_expression();

		    	while (m_lexer.current() == lex_or)
		    	{
		    		m_lexer.next();

		    		n = new op_node(op_or, n, parse_and_expression());
		    	}

		    	return n;
		    }
			
			// Expr ::= OrExpr
			node* parse_expression()
			{
				return parse_or_expression();
			}
	
		public:
			explicit parser(const char* query): m_lexer(query)
			{
			}
	
			node* parse()
			{
				return parse_expression();
			}
		};
	
		class query
		{
		private:
			// Noncopyable semantics
			query(const query&);
			query& operator=(const query&);
	
			node* m_root;
	
		public:
			explicit query(const char* query): m_root(0)
			{
				compile(query);
			}
	
			bool compile(const char* query)
			{
				delete m_root;
	
				parser p(query);

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

			void dump()
			{
				if (m_root) m_root->dump();
			}
		};
	};
}

int main()
{
	std::ifstream in("xpath.xml", std::ios::in | std::ios::binary);
	pugi::xml_parser parser(in);

	pugi::xpath::query* q = new pugi::xpath::query("sum(/bookstore/book[author/last-name = 'Bob']/price)");

	q->dump();
	
	// sum(/bookstore/book[author/last-name = 'Bob']/price)
	// result should be 12 + 55 + 6.50 = 73.5

	delete q;
}
