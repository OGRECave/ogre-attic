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
#include <boost/spirit/tree/ast.hpp>

using namespace boost;
using namespace boost::spirit;

namespace Ogre {

	/** This is the ogre skip parser grammar. The skip grammar defines what fragments of the
		input the parser will skip over when it is doing phase-level scanning.
	*/
	struct SkipGrammar : public grammar<SkipGrammar>
	{
		template<class ScannerT>
		struct definition
		{
			rule<ScannerT> r;

			definition(SkipGrammar const& self)
			{
				r = space_p | comment_p("//") | comment_p("/*", "*/");
			}

			rule<ScannerT> const& start(){ return r; }
		};
	};

	/** This is a fully-conforming spirit grammar which gather error information
		and throw a ParseErrorException.
	*/
	struct ErrorParser
	{
		// This is the error message which is thrown from this parser
		ParseError mError, mEofError;

		ErrorParser(ParseError error, ParseError eofError)
			:mError(error), mEofError(eofError){}

		// A simple copy constructor
		ErrorParser(const ErrorParser &rhs)
			:mError(rhs.mError), mEofError(rhs.mEofError){}

		// An assignment operator
		ErrorParser &operator = (const ErrorParser &rhs){
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

	/** This is the ogre script main grammar. It builds a custom AST as it parses
		the script input. It uses the custom-made ErrorParser to handle error conditions
		in the grammar.
	*/
	struct ScriptGrammar : public grammar<ScriptGrammar>
	{
		/** This is the AST generated from the definition in this grammar */
		struct AST{
			ScriptNodeListPtr nodes;
			ScriptNode *current;
		};
		mutable AST ast;

		template<class ScannerT>
		struct definition
		{
			typedef rule<ScannerT> rule_t;

			rule<ScannerT> quote, word, variable, block, import, statement, statement_list;

			// This parser generates specific warnings during the parsing process
			typedef functor_parser<ErrorParser> error_parser;
			error_parser error_p(ParseError err = PE_RESERVED, ParseError eofErr = PE_RESERVED){
				return ErrorParser(err, eofErr);
			}

			struct ast_action{
				AST &ast;
				ast_action(AST &rhs):ast(rhs){}
			};
			struct do_import : public ast_action{
				do_import(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					String token(first, last);

					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = SNT_IMPORT;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->file = first.get_position().file;
					node->parent = 0;
					ast.nodes->push_back(node);
					ast.current = node.get();
				}
			};
			struct do_import_target : public ast_action{
				do_import_target(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					String token(first, last);

					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = SNT_STRING;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->file = first.get_position().file;
					node->parent = ast.current;
					ast.current->children.push_back(node);
				}
			};
			struct do_import_path : public ast_action{
				do_import_path(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					String token(first, last);

					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = SNT_STRING;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->file = first.get_position().file;
					node->parent = ast.current;
					ast.current->children.push_back(node);
					ast.current = ast.current->parent;
				}
			};
			struct do_word : public ast_action{
				do_word(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					String token(first, last);

					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = SNT_STRING;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->file = first.get_position().file;
					node->parent = ast.current;

					if(ast.current)
						ast.current->children.push_back(node);
					else
						ast.nodes->push_back(node);
				}
			};
			struct do_variable : public ast_action{
				do_variable(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					String token(first, last);

					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = SNT_VAR;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->file = first.get_position().file;
					node->parent = ast.current;

					if(ast.current)
						ast.current->children.push_back(node);
					else
						ast.nodes->push_back(node);
				}
			};
			struct do_quote : public ast_action{
				do_quote(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					String token(first, last);

					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = SNT_QUOTE;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->file = first.get_position().file;
					node->parent = ast.current;

					if(ast.current)
						ast.current->children.push_back(node);
					else
						ast.nodes->push_back(node);
				}
			};
			struct do_number : public ast_action{
				do_number(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					String token(first, last);

					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = SNT_NUMBER;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->file = first.get_position().file;
					node->parent = ast.current;

					if(ast.current)
						ast.current->children.push_back(node);
					else
						ast.nodes->push_back(node);
				}
			};
			struct do_lbrace : public ast_action{
				do_lbrace(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					String token(first, last);

					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = SNT_LBRACE;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->file = first.get_position().file;
					node->parent = ast.current;

					if(ast.current)
						ast.current->children.push_back(node);
					else
						ast.nodes->push_back(node);
					ast.current = node.get();
				}
			};
			struct do_rbrace : public ast_action{
				do_rbrace(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					String token(first, last);

					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = SNT_RBRACE;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->file = first.get_position().file;

					ast.current = ast.current->parent;
					node->parent = ast.current;
					if(ast.current)
						ast.current->children.push_back(node);
					else
						ast.nodes->push_back(node);
				}
			};

			definition(ScriptGrammar const &self)
			{
				self.ast.nodes = ScriptNodeListPtr(new ScriptNodeList());
				self.ast.current = 0;

				word = lexeme_d[((alnum_p | punct_p) - (ch_p('$')|'*'|'{'|'}'|'\"')) 
					>> *(alnum_p | (punct_p - (ch_p('{')|'}')))];
				variable = lexeme_d[ch_p('$') >> *(alnum_p | (punct_p - (ch_p('{')|'}')))];
				quote = ch_p('\"') >> *(print_p - '\"') >> ("\"" | error_p(PE_ENDQUOTEEXPECTED));
				block = str_p("{")[do_lbrace(self.ast)] 
					>> *statement 
						>> (str_p("}")[do_rbrace(self.ast)] | error_p(PE_CLOSEBRACEEXPECTED));
				import = str_p("import")[do_import(self.ast)] 
					>> ((word | '*')[do_import_target(self.ast)] | error_p(PE_IMPORTTARGETEXPECTED)) 
						>> ("from" | error_p(PE_FROMEXPECTED))
							>> (word[do_import_path(self.ast)] | error_p(PE_IMPORTPATHEXPECTED));
				statement = 
					(eps_p >> real_p)[do_number(self.ast)]
					|word[do_word(self.ast)]
					|variable[do_variable(self.ast)]
					|quote[do_quote(self.ast)]
					|block
					;
				statement_list = *(import|statement);
			}

			rule<ScannerT> const &start(){return statement_list;}
		};
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
	ScriptNodeListPtr parse(const String &script, const String &source)
	{
		SkipGrammar skip;
		ScriptGrammar g;

		typedef position_iterator<String::const_iterator> iter_t;
		iter_t first(script.begin(), script.end(), source), last;
		parse_info<iter_t> info = boost::spirit::parse(first, last, g, skip);

		return g.ast.nodes;
	}
	/** End parse */
}
