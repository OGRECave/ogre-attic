/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreStableHeaders.h"
#include "OgreScriptParser.h"
#include <boost/spirit.hpp>

using namespace boost;
using namespace boost::spirit;

namespace Ogre {

	/** This class is meant to stand-in for regular spirit grammars. It is much faster,
		using no virtual functions and storing the sub-parser by reference.
		It is based on code in the boost spirit documentation.
	*/
	template<class DerivedT>
	struct sub_grammar : parser<DerivedT>
	{
		typedef sub_grammar self_t;
		typedef DerivedT const& embed_t;

		template<class ScannerT>
		struct result
		{
			typedef typename parser_result<typename DerivedT::start_t, ScannerT>::type type;
		};

		DerivedT const& derived() const{
			return *static_cast<DerivedT const*>(this);
		}

		template<class ScannerT>
		typename parser_result<self_t, ScannerT>::type parse(ScannerT const& scan) const{
			return derived().start.parse(scan);
		}
	};
	
	/** This is the ogre skip parser grammar. The skip grammar defines what fragments of the
		input the parser will skip over when it is doing phase-level scanning.
	*/
	struct skip_grammar : public sub_grammar<skip_grammar>
	{
		typedef boost::spirit::alternative<
			boost::spirit::confix_parser<
				boost::spirit::impl::string_as_parser::type,
					boost::spirit::kleene_star<boost::spirit::anychar_parser>,
					boost::spirit::alternative<boost::spirit::eol_parser,boost::spirit::end_parser>,
					boost::spirit::unary_parser_category,boost::spirit::non_nested,boost::spirit::is_lexeme>,
			boost::spirit::confix_parser<
				boost::spirit::impl::string_as_parser::type,
					boost::spirit::kleene_star<boost::spirit::anychar_parser>,
					boost::spirit::impl::string_as_parser::type,boost::spirit::unary_parser_category,
					boost::spirit::non_nested,boost::spirit::is_lexeme> > 
		start_t;

		skip_grammar()
			:start(comment_p("//") | comment_p("/*", "*/"))
		{}

		start_t start;
	};

	/** This is a fully-conforming spirit grammar which gathers error information
		and throws a ParseErrorException.
	*/
	struct error_parser
	{
		// This is the error message which is thrown from this parser
		ParseError mError, mEofError;

		error_parser(ParseError error, ParseError eofError)
			:mError(error), mEofError(eofError){}

		// A simple copy constructor
		error_parser(const error_parser &rhs)
			:mError(rhs.mError), mEofError(rhs.mEofError){}

		// An assignment operator
		error_parser &operator = (const error_parser &rhs){
			mError = rhs.mError;
			mEofError = rhs.mEofError;
			return *this;
		}

		// This is required by the functor_parser type in spirit
		typedef nil_t result_t;

		template<class ScannerT>
		int operator()(ScannerT const &scanner, nil_t&) const{
			const file_position &fp = scanner.first.get_position();
			if(scanner.at_end()){
				throw ParseErrorException(fp.file, fp.line, fp.column, mEofError != PE_RESERVED ? mEofError : mError);
			}else{
				throw ParseErrorException(fp.file, fp.line, fp.column, mError);
			}

			// Always return -1, which is a no-match condition
			return -1;
		}
	};

	// This parser generates specific warnings during the parsing process
	typedef functor_parser<error_parser> functor_error_parser;
	functor_error_parser error_p(ParseError err = PE_RESERVED, ParseError eofErr = PE_RESERVED){
		return error_parser(err, eofErr);
	}

	// This represents the AST as it is built by the parser
	struct script_ast
	{
		script_ast()
			:nodes(ScriptNodeListPtr(new ScriptNodeList())), current(0), ids(0)
		{}
		ScriptNodeListPtr nodes;
		ScriptNode *current;
		WordIDMap *ids;
	};

	// Matches any length of adjacent whitespace characters
	struct whitespace_grammar : public sub_grammar<whitespace_grammar>
	{
		// This is type definition of the rule which this grammar defines
		typedef boost::spirit::positive<boost::spirit::blank_parser> start_t;

		whitespace_grammar()
			:start(+blank_p)
		{}

		start_t start;
	};

	// Matches any number of newlines as one match
	struct newline_grammar : public sub_grammar<newline_grammar>
	{
		typedef boost::spirit::positive<boost::spirit::eol_parser> start_t;

		newline_grammar()
			:start(+eol_p)
		{}

		start_t start;
	};

	// Matches a word, which is a basic unit of script code
	struct word_grammar : public sub_grammar<word_grammar>
	{
		typedef boost::spirit::positive<boost::spirit::alternative<boost::spirit::alnum_parser,boost::spirit::difference<boost::spirit::punct_parser,boost::spirit::alternative<boost::spirit::alternative<boost::spirit::alternative<boost::spirit::alternative<boost::spirit::alternative<boost::spirit::chlit<char>,boost::spirit::chlit<char> >,boost::spirit::chlit<char> >,boost::spirit::chlit<char> >,boost::spirit::chlit<char> >,boost::spirit::chlit<char> > > > > start_t;

		word_grammar()
			:start(+(alnum_p|(punct_p - (ch_p('{')|'}'|'\"'|':'|'\''|'$'))))
		{}

		start_t start;
	};

	// Matches a variable, starting with '$'
	struct variable_grammar : public sub_grammar<variable_grammar>
	{
		typedef boost::spirit::sequence<boost::spirit::chlit<char>,boost::spirit::kleene_star<boost::spirit::alternative<boost::spirit::alnum_parser,boost::spirit::difference<boost::spirit::punct_parser,boost::spirit::alternative<boost::spirit::alternative<boost::spirit::alternative<boost::spirit::chlit<char>,boost::spirit::chlit<char> >,boost::spirit::chlit<char> >,boost::spirit::chlit<char> > > > > > start_t;

		variable_grammar()
			:start(ch_p('$') >> (*(alnum_p|(punct_p - (ch_p('{')|'}'|'\"'|':')))))
		{}

		start_t start;
	};

	// Matches a quoted string (embedded quotes not supported)
	struct quote_grammar : public sub_grammar<quote_grammar>
	{
		typedef boost::spirit::alternative<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::chlit<char>,boost::spirit::kleene_star<boost::spirit::difference<boost::spirit::alternative<boost::spirit::print_parser,boost::spirit::eol_parser>,boost::spirit::chlit<char> > > >,boost::spirit::alternative<boost::spirit::strlit<const char *>,boost::spirit::functor_parser<Ogre::error_parser> > >,boost::spirit::sequence<boost::spirit::sequence<boost::spirit::chlit<char>,boost::spirit::kleene_star<boost::spirit::difference<boost::spirit::alternative<boost::spirit::print_parser,boost::spirit::eol_parser>,boost::spirit::chlit<char> > > >,boost::spirit::alternative<boost::spirit::strlit<const char *>,boost::spirit::functor_parser<Ogre::error_parser> > > > start_t;

		quote_grammar()
			:start
			(
				(ch_p('\"') >> *((print_p|eol_p) - ch_p('\"')) >> ("\"" | error_p(PE_ENDQUOTEEXPECTED)))
				|
				(ch_p('\'') >> *((print_p|eol_p) - ch_p('\'')) >> ("\'" | error_p(PE_ENDQUOTEEXPECTED)))
			)
		{}

		start_t start;
	};

	// Matches a property declaration
	struct property_grammar : public sub_grammar<property_grammar>
	{
		struct word_action
		{
			bool isProperty;
			script_ast **ast;
			word_action(script_ast **rhs, bool prop)
				:ast(rhs),isProperty(prop){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				ScriptNodePtr node(new ScriptNode());

				bool isNumber = false;
				if(boost::spirit::parse(first, last, real_p[assign_a(node->data)]).full)
					isNumber = true;
				
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = isNumber ? SNT_NUMBER : SNT_WORD;
				node->token.assign(first, last);
				if(!isNumber)
				{
					WordIDMap::const_iterator i = (*ast)->ids->find(node->token);
					if(i != (*ast)->ids->end())
						node->wordID = i->second;
				}
				node->isObject = false;
				node->isProperty = isProperty;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				if(isProperty)
					(*ast)->current = node.get();
			}
		};
		struct variable_action
		{
			bool isProperty;
			script_ast **ast;
			variable_action(script_ast **rhs, bool prop):ast(rhs),isProperty(prop){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				ScriptNodePtr node(new ScriptNode());
				node->token.assign(first, last);
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_VARIABLE;
				node->isObject = false;
				node->isProperty = isProperty;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				if(isProperty)
					(*ast)->current = node.get();
			}
		};
		struct end_action
		{
			script_ast **ast;
			end_action(script_ast **rhs):ast(rhs){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				(*ast)->current = (*ast)->current->parent;
			}
		};
		typedef boost::spirit::sequence<spirit::sequence<boost::spirit::sequence<boost::spirit::alternative<boost::spirit::action<Ogre::word_grammar,Ogre::property_grammar::word_action>,boost::spirit::action<Ogre::variable_grammar,Ogre::property_grammar::variable_action> >,boost::spirit::positive<boost::spirit::sequence<Ogre::whitespace_grammar,boost::spirit::alternative<boost::spirit::action<Ogre::word_grammar,Ogre::property_grammar::word_action>,boost::spirit::action<Ogre::variable_grammar,Ogre::property_grammar::variable_action> > > > >,boost::spirit::optional<Ogre::whitespace_grammar> >,boost::spirit::optional<boost::spirit::action<Ogre::newline_grammar,Ogre::property_grammar::end_action> > > start_t;

		property_grammar(script_ast *rhs)
			:ast(rhs),
			 start
			 (
				((word[word_action(&ast,true)])|(variable[variable_action(&ast,true)]))
				>>
				+(ws
					>>
					((word[word_action(&ast,false)])|(variable[variable_action(&ast,false)])))
				>>
				!ws
				>>
				!(nl[end_action(&ast)])
			 )
		{}

		// These are this grammar's dependent grammars
		whitespace_grammar ws;
		newline_grammar nl;
		word_grammar word;
		variable_grammar variable;

		start_t start;
		script_ast *ast;
	};

	// Matches an import statement
	struct import_grammar : public sub_grammar<import_grammar>
	{
		struct import_action
		{
			script_ast **ast;
			import_action(script_ast **rhs):ast(rhs){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				ScriptNodePtr node(new ScriptNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_IMPORT;
				node->isObject = false;
				node->isProperty = false;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				(*ast)->current = node.get();
			}
		};
		struct target_action
		{
			script_ast **ast;
			target_action(script_ast **rhs):ast(rhs){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				ScriptNodePtr node(new ScriptNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_WORD;
				node->token.assign(first, last);
				node->isObject = false;
				node->isProperty = false;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
			}
		};
		struct script_action
		{
			script_ast **ast;
			script_action(script_ast **rhs):ast(rhs){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				String token(first, last);
				ScriptNodePtr node(new ScriptNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_WORD;
				node->token = token.size() == 2 ? "" : token.substr(1, token.size() - 2);
				node->isObject = false;
				node->isProperty = false;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				(*ast)->current = (*ast)->current->parent;
			}
		};
		typedef boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::action<boost::spirit::strlit<const char *>,Ogre::import_grammar::import_action>,Ogre::whitespace_grammar>,boost::spirit::action<boost::spirit::alternative<Ogre::word_grammar,boost::spirit::strlit<const char *> >,Ogre::import_grammar::target_action> >,Ogre::whitespace_grammar>,boost::spirit::alternative<boost::spirit::strlit<const char *>,boost::spirit::functor_parser<Ogre::error_parser> > >,Ogre::whitespace_grammar>,boost::spirit::alternative<boost::spirit::action<Ogre::quote_grammar,Ogre::import_grammar::script_action>,boost::spirit::functor_parser<Ogre::error_parser> > > start_t;

		import_grammar(script_ast *rhs)
			:ast(rhs),
			start
			(
				(str_p("import")[import_action(&ast)])
				>>
				ws
				>>
				((word|str_p("*"))[target_action(&ast)])
				>>
				ws
				>>
				(str_p("from")|error_p(PE_FROMEXPECTED))
				>> 
				ws
				>>
				((quote[script_action(&ast)])|error_p(PE_IMPORTPATHEXPECTED))
			)
		{}

		whitespace_grammar ws;
		word_grammar word;
		quote_grammar quote;

		start_t start;
		script_ast *ast;
	};

	// Matches a variable assignment operation
	struct variable_assign_grammar : public sub_grammar<variable_assign_grammar>
	{
		struct set_action
		{
			script_ast **ast;
			set_action(script_ast **rhs):ast(rhs){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				ScriptNodePtr node(new ScriptNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_VARIABLE_ASSIGN;
				node->isObject = false;
				node->isProperty = false;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				(*ast)->current = node.get();
			}
		};
		struct variable_action
		{
			script_ast **ast;
			variable_action(script_ast **rhs):ast(rhs){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				ScriptNodePtr node(new ScriptNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_VARIABLE;
				node->token.assign(first, last);
				node->isObject = false;
				node->isProperty = false;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
			}
		};
		struct value_action
		{
			script_ast **ast;
			value_action(script_ast **rhs):ast(rhs){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				String token(first, last);
				ScriptNodePtr node(new ScriptNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_QUOTE;
				node->token = token.size() == 2 ? "" : token.substr(1, token.size() - 2);
				node->isObject = false;
				node->isProperty = false;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				(*ast)->current = (*ast)->current->parent;
			}
		};
		typedef boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::action<boost::spirit::strlit<const char *>,Ogre::variable_assign_grammar::set_action>,Ogre::whitespace_grammar>,boost::spirit::alternative<boost::spirit::action<Ogre::variable_grammar,Ogre::variable_assign_grammar::variable_action>,boost::spirit::functor_parser<Ogre::error_parser> > >,boost::spirit::positive<boost::spirit::alternative<Ogre::whitespace_grammar,Ogre::newline_grammar> > >,boost::spirit::alternative<boost::spirit::action<Ogre::quote_grammar,Ogre::variable_assign_grammar::value_action>,boost::spirit::functor_parser<Ogre::error_parser> > > start_t;

		variable_assign_grammar(script_ast *rhs)
			:ast(rhs),
			start
			(
				(str_p("set")[set_action(&ast)])
				>>
				ws
				>>
				((variable[variable_action(&ast)])|error_p(PE_VARIABLEEXPECTED))
				>>
				+(ws|nl)
				>>
				((quote[value_action(&ast)])|error_p(PE_VARIABLEVALUEEXPECTED))
			)
		{}

		whitespace_grammar ws;
		newline_grammar nl;
		variable_grammar variable;
		quote_grammar quote;

		start_t start;
		script_ast *ast;
	};

	// This matches an object header as a look-ahead mechanism
	struct object_header_grammar : public sub_grammar<object_header_grammar>
	{
		typedef boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::alternative<boost::spirit::alternative<Ogre::quote_grammar,Ogre::word_grammar>,Ogre::variable_grammar>,boost::spirit::kleene_star<boost::spirit::sequence<Ogre::whitespace_grammar,boost::spirit::alternative<boost::spirit::alternative<Ogre::quote_grammar,Ogre::word_grammar>,Ogre::variable_grammar> > > >,boost::spirit::optional<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<Ogre::whitespace_grammar,boost::spirit::strlit<const char *> >,Ogre::whitespace_grammar>,boost::spirit::alternative<boost::spirit::alternative<Ogre::quote_grammar,Ogre::word_grammar>,Ogre::variable_grammar> > > >,boost::spirit::kleene_star<boost::spirit::alternative<Ogre::whitespace_grammar,Ogre::newline_grammar> > >,boost::spirit::strlit<const char *> > start_t;

		object_header_grammar()
			:start
			(
				(quote|word|variable)
				>>
				*(ws >> (quote|word|variable))
				>>
				!(ws >> str_p(":") >> ws >> (quote|word|variable))
				>>
				*(ws|nl)
				>>
				str_p("{")
			)
		{}

		whitespace_grammar ws;
		newline_grammar nl;
		quote_grammar quote;
		variable_grammar variable;
		word_grammar word;

		start_t start;
	};

	// This recognized an object declaration
	struct object_grammar : public grammar<object_grammar>
	{
		mutable script_ast *ast;
		object_grammar(script_ast *rhs):ast(rhs){}

		struct variable_action
		{
			script_ast **ast;
			bool isObject;
			variable_action(script_ast **rhs, bool obj):ast(rhs), isObject(obj){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				ScriptNodePtr node(new ScriptNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_VARIABLE;
				node->token.assign(first, last);
				node->isObject = isObject;
				node->isProperty = false;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				if(isObject)
					(*ast)->current = node.get();
			}
		};
		struct word_action
		{
			script_ast **ast;
			bool isObject;
			word_action(script_ast **rhs, bool obj):ast(rhs),isObject(obj){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				ScriptNodePtr node(new ScriptNode());

				bool isNumber = false;
				if(boost::spirit::parse(first, last, real_p[assign_a(node->data)]).full)
					isNumber = true;

				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = isNumber ? SNT_NUMBER : SNT_WORD;
				node->token.assign(first, last);
				if(!isNumber)
				{
					WordIDMap::const_iterator i = (*ast)->ids->find(node->token);
					if(i != (*ast)->ids->end())
						node->wordID = i->second;
				}
				node->isObject = isObject;
				node->isProperty = false;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				if(isObject)
					(*ast)->current = node.get();
			}
		};
		struct quote_action
		{
			script_ast **ast;
			bool isObject;
			quote_action(script_ast **rhs, bool obj):ast(rhs), isObject(obj){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				String token(first, last);
				ScriptNodePtr node(new ScriptNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_QUOTE;
				node->token = token.size() == 2 ? "" : token.substr(1, token.size() - 2);
				node->isObject = isObject;
				node->isProperty = false;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				if(isObject)
					(*ast)->current = node.get();
			}
		};
		struct colon_action
		{
			script_ast **ast;
			colon_action(script_ast **rhs):ast(rhs){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				ScriptNodePtr node(new ScriptNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_COLON;
				node->isObject = false;
				node->isProperty = false;
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				(*ast)->current = node.get();
			}
		};
		struct lbrace_action
		{
			script_ast **ast;
			lbrace_action(script_ast **rhs):ast(rhs){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				ScriptNodePtr node(new ScriptNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_LBRACE;
				node->isObject = false;
				node->isProperty = false;

				// Remove the colon if it is the current parent
				if((*ast)->current && (*ast)->current->type == SNT_COLON)
					(*ast)->current = (*ast)->current->parent;

				// Set up the new hierarchy for the children
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				(*ast)->current = node.get();
			}
		};
		struct rbrace_action
		{
			script_ast **ast;
			rbrace_action(script_ast **rhs):ast(rhs){}
			template<class IterT>void operator()(IterT first, IterT last) const{
				ScriptNodePtr node(new ScriptNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = SNT_RBRACE;
				node->isObject = false;
				node->isProperty = false;
				
				// End a property if it is still going
				if((*ast)->current && (*ast)->current->isProperty)
					(*ast)->current = (*ast)->current->parent;

				// Next should be the lbrace. End it
				if((*ast)->current)
					(*ast)->current = (*ast)->current->parent;

				// Add ourselves in here
				node->parent = (*ast)->current;
				if((*ast)->current)
					(*ast)->current->children.push_back(node);
				else
					(*ast)->nodes->push_back(node);
				
				// Finally, end the object
				if((*ast)->current)
					(*ast)->current = (*ast)->current->parent;
			}
		};

		template<typename ScannerT>
		struct definition
		{
			definition(object_grammar const& self)
				:prop(self.ast), variable_assign(self.ast)
			{
				object = 
					eps_p(object_header)
					>>
					((quote[quote_action(&self.ast,true)])|(word[word_action(&self.ast,true)])|(variable[variable_action(&self.ast,true)]))
					>>
					*(ws >> ((quote[quote_action(&self.ast,false)])|(word[word_action(&self.ast,false)])|(variable[variable_action(&self.ast,false)])))
					>>
					!(ws >> (str_p(":")[colon_action(&self.ast)]) >> ws >>
						((quote[quote_action(&self.ast,false)])|(word[word_action(&self.ast,false)])|(variable[variable_action(&self.ast,false)])))
					>>
					*(ws|nl)
					>>
					(str_p("{")[lbrace_action(&self.ast)])
					>>
					*(*(ws|nl) >> (variable_assign|(variable[variable_action(&self.ast,false)])|object|prop))
					>>
					*(ws|nl)
					>>
					((str_p("}")[rbrace_action(&self.ast)])|error_p(PE_CLOSEBRACEEXPECTED))
					;
			}

			whitespace_grammar ws;
			newline_grammar nl;
			word_grammar word;
			variable_grammar variable;
			property_grammar prop;
			variable_assign_grammar variable_assign;
			quote_grammar quote;
			object_header_grammar object_header;
			rule<ScannerT> object;
			rule<ScannerT> const& start() const{return object;}
		};
	};

	/** This is the ogre script main grammar. It builds a custom AST as it parses
		the script input. It uses the custom-made error_parser to handle error conditions
		in the grammar.
	*/
	struct script_grammar : public sub_grammar<script_grammar>
	{
		// This is type definition of the rule which this grammar defines
		typedef boost::spirit::sequence<boost::spirit::kleene_star<boost::spirit::alternative<Ogre::whitespace_grammar,Ogre::newline_grammar> >,boost::spirit::kleene_star<boost::spirit::sequence<boost::spirit::alternative<boost::spirit::alternative<boost::spirit::alternative<Ogre::import_grammar,Ogre::variable_assign_grammar>,Ogre::object_grammar>,Ogre::property_grammar>,boost::spirit::kleene_star<boost::spirit::alternative<Ogre::whitespace_grammar,Ogre::newline_grammar> > > > > start_t;

		script_grammar(const WordIDMap &ids)
			:prop(&ast), import(&ast), variable_assign(&ast), object(&ast),
			 start
			 (
				*(ws|nl)
				>>
				*((import|variable_assign|object|prop) >> *(ws|nl))
			 )
		{
			ast.ids = const_cast<WordIDMap*>(&ids);
		}

		// These are this grammar's dependent grammars
		whitespace_grammar ws;
		newline_grammar nl;
		import_grammar import;
		property_grammar prop;
		variable_assign_grammar variable_assign;
		object_grammar object;

		start_t start;
		script_ast ast;
	};

	/** Begin ParseErrorException definition */
	ParseErrorException::ParseErrorException(const Ogre::String &file, int line, int column, Ogre::ParseError err)
		:mFile(file), mLine(line), mColumn(column), mError(err)
	{}

	const char *ParseErrorException::what() const
	{
		if(mErrorStr.empty())
		{
			std::stringstream stream;
			stream << "error \"" << getErrorString() << "\" found at line " << mLine << ", column " 
				<< mColumn << " in " << mFile;
			mErrorStr = stream.str();
		}

		return mErrorStr.c_str();
	}

	const String &ParseErrorException::getFile() const
	{
		return mFile;
	}

	int ParseErrorException::getLine() const
	{
		return mLine;
	}

	ParseError ParseErrorException::getError() const
	{
		return mError;
	}

	String ParseErrorException::getErrorString() const
	{
		String error = "";
		switch(mError)
		{
		case PE_IMPORTPATHEXPECTED:
			error = "import file path or file name expected";
			break;
		case PE_OPENBRACEEXPECTED:
			error = "open brace \'{\' expected";
			break;
		case PE_CLOSEBRACEEXPECTED:
			error = "close brace \'}\' expected";
			break;
		case PE_ENDQUOTEEXPECTED:
			error = "end of quote \'\"\' expected";
			break;
		case PE_IMPORTTARGETEXPECTED:
			error = "import target expected";
			break;
		case PE_FROMEXPECTED:
			error = "keyword \"from\" expected";
			break;
		case PE_UNKNOWN:
		default:
			error = "unknown";
		}
		return error;
	}
	/** End ParseErrorException */

	/** Begin parse */
	ScriptNodeListPtr parse(const String &script, const String &source, const WordIDMap &ids)
	{
		skip_grammar skip;
		script_grammar g(ids);

		typedef position_iterator<String::const_iterator> iter_t;
		iter_t first(script.begin(), script.end(), source), last;
		parse_info<iter_t> info = boost::spirit::parse(first, last, g, skip);

		return g.ast.nodes;
	}
	/** End parse */
}
