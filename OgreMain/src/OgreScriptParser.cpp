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
#include "OgreScriptCompiler.h"
#include "OgreScriptParser.h"

#if OGRE_USE_NEW_COMPILERS
#include <boost/spirit.hpp>
using namespace boost;
using namespace boost::spirit;
#endif

namespace Ogre {
#if OGRE_USE_NEW_COMPILERS
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

	/** This enum contains identifiers for the possible parsing errors */
	enum ParseError
	{
		PE_RESERVED,
		PE_OPENBRACEEXPECTED,
		PE_CLOSEBRACEEXPECTED,
		PE_IMPORTPATHEXPECTED,
		PE_IMPORTTARGETEXPECTED,
		PE_ENDQUOTEEXPECTED,
		PE_FROMEXPECTED,
		PE_PARENTOBJECTEXPECTED,
		PE_VARIABLEEXPECTED,
		PE_VARIABLEVALUEEXPECTED,
		PE_UNKNOWN
	};

	/// Returns a string from the parse error
	static String getErrorString(ParseError err)
	{
		String error = "";
		switch(err)
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

	/** This is a fully-conforming spirit grammar which gathers error information
		and throws a ParseErrorException.
	*/
	struct error_parser
	{
		// This is the error message which is thrown from this parser
		ParseError mError;

		error_parser(ParseError error)
			:mError(error){}

		// A simple copy constructor
		error_parser(const error_parser &rhs)
			:mError(rhs.mError){}

		// An assignment operator
		error_parser &operator = (const error_parser &rhs){
			mError = rhs.mError;
			return *this;
		}

		// This is required by the functor_parser type in spirit
		typedef nil_t result_t;

		template<class ScannerT>
		int operator()(ScannerT const &scanner, nil_t&) const{
			const file_position &fp = scanner.first.get_position();
			String desc = getErrorString(mError);
			throw Ogre::InvalidStateException((int)mError, desc, "OgreScriptParser", fp.file.c_str(), fp.line);

			// Always return -1, which is a no-match condition
			return -1;
		}
	};

	// This parser generates specific warnings during the parsing process
	typedef functor_parser<error_parser> functor_error_parser;
	functor_error_parser error_p(ParseError err = PE_RESERVED){
		return error_parser(err);
	}

	/** This is a store which holds the CST as it is built */
	struct cst_store
	{
		ConcreteNode *current;
		ConcreteNodeListPtr nodes;
		cst_store():current(0), nodes(new ConcreteNodeList()){}
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

	// Matches a word
	struct word_grammar : public sub_grammar<word_grammar>
	{
		typedef boost::spirit::positive<boost::spirit::alternative<boost::spirit::alnum_parser,boost::spirit::difference<boost::spirit::punct_parser,boost::spirit::alternative<boost::spirit::alternative<boost::spirit::alternative<boost::spirit::alternative<boost::spirit::chlit<char>,boost::spirit::chlit<char> >,boost::spirit::chlit<char> >,boost::spirit::chlit<char> >,boost::spirit::chlit<char> > > > > start_t;

		word_grammar()
			:start(+(alnum_p|(punct_p - (ch_p('*')|':'|'$'|'{'|'}'))))
		{}

		start_t start;
	};

	// Matches a variable
	struct variable_grammar : public sub_grammar<variable_grammar>
	{
		typedef boost::spirit::sequence<boost::spirit::chlit<char>,Ogre::word_grammar> start_t;

		variable_grammar()
			:start(ch_p('$') >> word)
		{}

		start_t start;
		word_grammar word;
	};

	// Matches quoted phrase
	struct quote_grammar : public sub_grammar<quote_grammar>
	{
		typedef boost::spirit::confix_parser<boost::spirit::impl::char_as_parser::type,boost::spirit::kleene_star<boost::spirit::anychar_parser>,boost::spirit::impl::char_as_parser::type,boost::spirit::kleene_star<boost::spirit::anychar_parser>::parser_category_t,boost::spirit::non_nested,boost::spirit::non_lexeme> start_t;

		quote_grammar()
			:start(confix_p('\"', *anychar_p, '\"'))
		{}

		start_t start;
	};

	// Matches an import statement
	struct import_grammar : public sub_grammar<import_grammar>
	{
		struct import_action
		{
			cst_store *cst;
			import_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->token.assign(first, last);
				node->type = CNT_IMPORT;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
				cst->current = node.get();
			}
		};
		struct word_action
		{
			cst_store *cst;
			word_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->token.assign(first, last);
				node->type = CNT_WORD;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
			}
		};
		struct quote_action
		{
			cst_store *cst;
			quote_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				first++;
				last--;
				node->token.assign(first, last);
				node->type = CNT_QUOTE;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
			}
		};
		struct endimport_action
		{
			cst_store *cst;
			endimport_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				cst->current = cst->current->parent;
			}
		};

		typedef boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::action<boost::spirit::strlit<const char *>,Ogre::import_grammar::import_action>,Ogre::whitespace_grammar>,boost::spirit::alternative<boost::spirit::alternative<boost::spirit::action<Ogre::quote_grammar,Ogre::import_grammar::quote_action>,boost::spirit::action<Ogre::word_grammar,Ogre::import_grammar::word_action> >,boost::spirit::action<boost::spirit::strlit<const char *>,Ogre::import_grammar::word_action> > >,Ogre::whitespace_grammar>,boost::spirit::strlit<const char *> >,Ogre::whitespace_grammar>,boost::spirit::alternative<boost::spirit::action<Ogre::quote_grammar,Ogre::import_grammar::quote_action>,boost::spirit::action<Ogre::word_grammar,Ogre::import_grammar::word_action> > >,boost::spirit::sequence<boost::spirit::optional<Ogre::whitespace_grammar>,boost::spirit::optional<boost::spirit::action<Ogre::newline_grammar,Ogre::import_grammar::endimport_action> > > > start_t;

		import_grammar(cst_store *cst)
			:cst(cst),
			 start(str_p("import")[import_action(cst)] >> ws >> 
				((quote[quote_action(cst)])|(word[word_action(cst)])|(str_p("*")[word_action(cst)])) >> ws >> 
				"from" >> ws >> 
				((quote[quote_action(cst)])|(word[word_action(cst)])) >>
				(!ws >> !(nl[endimport_action(cst)])))
		{}

		start_t start;
		quote_grammar quote;
		word_grammar word;
		whitespace_grammar ws;
		newline_grammar nl;
		cst_store *cst;
	};

	// Matches a variable assign
	struct variable_assign_grammar : public sub_grammar<variable_assign_grammar>
	{
		struct set_action
		{
			cst_store *cst;
			set_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->token.assign(first, last);
				node->type = CNT_VARIABLE_ASSIGN;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
				cst->current = node.get();
			}
		};
		struct variable_action
		{
			cst_store *cst;
			variable_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->token.assign(first, last);
				node->type = CNT_VARIABLE;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
			}
		};
		struct word_action
		{
			cst_store *cst;
			word_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->token.assign(first, last);
				node->type = CNT_WORD;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
			}
		};
		struct quote_action
		{
			cst_store *cst;
			quote_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				first++;
				last--;
				node->token.assign(first, last);
				node->type = CNT_QUOTE;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
			}
		};
		struct endset_action
		{
			cst_store *cst;
			endset_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				cst->current = cst->current->parent;
			}
		};

		typedef boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::sequence<boost::spirit::action<boost::spirit::strlit<const char *>,Ogre::variable_assign_grammar::set_action>,Ogre::whitespace_grammar>,boost::spirit::action<Ogre::variable_grammar,Ogre::variable_assign_grammar::variable_action>>,Ogre::whitespace_grammar>,boost::spirit::alternative<boost::spirit::action<Ogre::quote_grammar,Ogre::variable_assign_grammar::quote_action>,boost::spirit::action<Ogre::word_grammar,Ogre::variable_assign_grammar::word_action> > >,boost::spirit::sequence<boost::spirit::optional<Ogre::whitespace_grammar>,boost::spirit::optional<boost::spirit::action<Ogre::newline_grammar,Ogre::variable_assign_grammar::endset_action> > > > start_t;

		variable_assign_grammar(cst_store *cst)
			:cst(cst),
			 start((str_p("set")[set_action(cst)]) >> ws >> 
				(var[variable_action(cst)]) >> ws >> 
				((quote[quote_action(cst)])|(word[word_action(cst)])) >>
				(!ws >> !(nl[endset_action(cst)])))
		{}

		start_t start;
		whitespace_grammar ws;
		newline_grammar nl;
		quote_grammar quote;
		word_grammar word;
		variable_grammar var;
		cst_store *cst;
	};

	// Matches an object or property
	struct object_grammar : public grammar<object_grammar>
	{
		struct object_action
		{
			cst_store *cst;
			object_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->token.assign(first, last);
				node->type = CNT_WORD;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
				cst->current = node.get();
			}
		};
		struct variable_action
		{
			cst_store *cst;
			variable_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->token.assign(first, last);
				node->type = CNT_VARIABLE;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
			}
		};
		struct word_action
		{
			cst_store *cst;
			word_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->token.assign(first, last);
				node->type = CNT_WORD;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
			}
		};
		struct colon_action
		{
			cst_store *cst;
			colon_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = CNT_COLON;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
				cst->current = node.get();
			}
		};
		struct parent_action
		{
			cst_store *cst;
			parent_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->token.assign(first, last);
				node->type = CNT_WORD;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
				cst->current = cst->current->parent;
			}
		};
		struct quote_action
		{
			cst_store *cst;
			quote_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				first++;
				last--;
				node->token.assign(first, last);
				node->type = CNT_QUOTE;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
			}
		};
		struct lbrace_action
		{
			cst_store *cst;
			lbrace_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = CNT_LBRACE;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
				cst->current = node.get();
			}
		};
		struct rbrace_action
		{
			cst_store *cst;
			rbrace_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				ConcreteNodePtr node(new ConcreteNode());
				node->file = first.get_position().file;
				node->line = first.get_position().line;
				node->column = first.get_position().column;
				node->type = CNT_RBRACE;

				cst->current = cst->current->parent;
				node->parent = cst->current;
				if(cst->current)
					cst->current->children.push_back(node);
				else
					cst->nodes->push_back(node);
				cst->current = cst->current->parent;
			}
		};
		struct endproperty_action
		{
			cst_store *cst;
			endproperty_action(cst_store *cst):cst(cst){}
			template<class IterT>
			void operator()(IterT first, IterT last) const{
				cst->current = cst->current->parent;
			}
		};
		mutable cst_store *cst;
		object_grammar(cst_store *cst):cst(cst){}
		template<class ScannerT>
		struct definition
		{
			definition(object_grammar const& self):object(self.cst), var_assign(self.cst){
				r = word[object_action(self.cst)] >> +(ws >> ((word[word_action(self.cst)])|(var[variable_action(self.cst)])|(quote[quote_action(self.cst)]))) >>
					!(ws >> (str_p(":")[colon_action(self.cst)]) >> ws >> (word[parent_action(self.cst)])) >>  
					((*(ws|nl) >> (str_p("{")[lbrace_action(self.cst)]) >> *(ws|nl) >> *(var_assign|(var[variable_action(self.cst)])|object) >> *(ws|nl) >> (str_p("}")[rbrace_action(self.cst)]))
					|
					(!ws >> nl[endproperty_action(self.cst)]));
			}

			rule<ScannerT> const& start() const{return r;}
			rule<ScannerT> r;
			object_grammar object;
			variable_assign_grammar var_assign;
			variable_grammar var;
			word_grammar word;
			quote_grammar quote;
			whitespace_grammar ws;
			newline_grammar nl;
		};
	};

	// Matches a whole script
	struct script_grammar : public sub_grammar<script_grammar>
	{
		typedef boost::spirit::sequence<boost::spirit::kleene_star<boost::spirit::sequence<boost::spirit::kleene_star<boost::spirit::alternative<Ogre::whitespace_grammar,Ogre::newline_grammar>>,boost::spirit::alternative<boost::spirit::alternative<Ogre::import_grammar,Ogre::variable_assign_grammar>,Ogre::object_grammar> > >,boost::spirit::kleene_star<boost::spirit::alternative<Ogre::whitespace_grammar,Ogre::newline_grammar> > > start_t;

		script_grammar(cst_store *cst)
			:object(cst),
			 import(cst),
			 variable_assign(cst),
			 start(
				*(*(ws|nl) >>
					(import|variable_assign|object))
				>>
				*(ws|nl)
			)
		{}

		start_t start;
		object_grammar object;
		newline_grammar nl;
		whitespace_grammar ws;
		import_grammar import;
		variable_assign_grammar variable_assign;
	};

	// Matches a value list of quotes, words, and variables
	struct value_list_grammar : public sub_grammar<value_list_grammar>
	{
		typedef boost::spirit::sequence<boost::spirit::kleene_star<boost::spirit::sequence<boost::spirit::alternative<Ogre::whitespace_grammar,Ogre::newline_grammar>,boost::spirit::alternative<boost::spirit::alternative<Ogre::word_grammar,Ogre::quote_grammar>,Ogre::variable_grammar> > >,boost::spirit::kleene_star<boost::spirit::alternative<Ogre::whitespace_grammar,Ogre::newline_grammar> > > start_t;

		value_list_grammar()
			:start(
				*((ws|nl) >>
				(word|quote|var))
				>>
				*(ws|nl)
			)
		{}

		start_t start;
		newline_grammar nl;
		whitespace_grammar ws;
		word_grammar word;
		quote_grammar quote;
		variable_grammar var;
	};
#endif

	/** Begin parse */
	_OgreExport ConcreteNodeListPtr parse(const String &script, const String &source)
	{
#if OGRE_USE_NEW_COMPILERS
		cst_store cstore;
		skip_grammar skip;
		script_grammar g(&cstore);

		typedef position_iterator<const char*> iter_t;
		iter_t first(script.c_str(), script.c_str()+script.size(), source), last;

		parse_info<iter_t> info = boost::spirit::parse(first, last, g, skip);

		return cstore.nodes;
#else
		return ConcreteNodeListPtr();
#endif
	}
	/** End parse */

	/** Begin parseChunk */
	_OgreExport ConcreteNodeListPtr parseChunk(const String &script, const String &source)
	{
#if OGRE_USE_NEW_COMPILERS
		cst_store cstore;
		value_list_grammar g1;
		object_grammar g2(&cstore);

		typedef position_iterator<const char*> iter_t;
		iter_t first(script.c_str(), script.c_str()+script.size(), source), last;

		parse_info<iter_t> info = boost::spirit::parse(first, last, g2|g1);

		return cstore.nodes;
#else
		return ConcreteNodeListPtr();
#endif
	}
	/** End parseChunk */

	/** Begin parseNumber */
	_OgreExport bool parseNumber(const String &script, Ogre::Real &num)
	{
#if OGRE_USE_NEW_COMPILERS
		return boost::spirit::parse(script.c_str(), boost::spirit::real_p[boost::spirit::assign_a(num)]).full;
#else
		return false;
#endif
	}
	/** End parseNumber */

}
