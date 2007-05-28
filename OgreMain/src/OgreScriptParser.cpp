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

			// stat_list ::= (import_stat|obj)*
			rule<ScannerT> stat_list;
			// import_stat ::= "import" script_path
			rule<ScannerT> import_stat;
			// script_path ::= ('A-Za-z0-9'|punct)* | ('\"' >> ('A-Za-z0-9'|punct|' ')* >> '\"')
			rule<ScannerT> script_path;
			// obj ::= 'abstract'! word ident? (':' ident)? '{' (obj|expr)* '}'
			rule<ScannerT> obj, top_obj;
			// expr ::= (word (word|variable|number)* '\n')+
			rule<ScannerT> expr;
			// word ::= 'A-Za-z'|'_' 'A-Za-z0-9'|'_'
			rule<ScannerT> word;
			// ident ::= 'A-Za-z'|'_' 'A-Za-z0-9'|'_'|'/'|'.'
			rule<ScannerT> ident;

			// This parser handles mismatches in the script_path rule
			// when parsing import commands.
			typedef functor_parser<ErrorParser> error_parser;
			error_parser error_p(ParseError err = PE_RESERVED, ParseError eofErr = PE_RESERVED){
				return ErrorParser(err, eofErr);
			}

			struct ast_action{
				AST &ast;
				ast_action(AST &rhs):ast(rhs){}
			};
			// Action that puts a root node into the tree when an import statement is found
			struct do_import_stat : public ast_action
			{
				do_import_stat(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					std::string token(first, last);

					// Add a new node to the top-level list
					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->file = first.get_position().file;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->type = TOK_IMPORT;
					node->parent = 0;
					ast.nodes->push_back(node);
					ast.current = node.get(); 
				}
			};
			// This action takes a script path after an import statement and adds it to the current node.
			// It also resets the current node to blank.
			struct do_script_path : public ast_action
			{
				do_script_path(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					std::string token(first, last);

					// Add it to the current node
					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->file = first.get_position().file;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->type = TOK_IMPORTPATH;
					node->parent = ast.current;
					node->parent->children.push_back(node);
					ast.current = 0;
				}
			};
			// Takes the abstract keyword and places it into the ast
			struct do_abstract : public ast_action
			{
				do_abstract(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					std::string token(first, last);

					// Add a new node to the top-level list
					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->file = first.get_position().file;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					node->type = TOK_STRING;
					node->parent = 0;
					ast.nodes->push_back(node);
				}
			};
			// This action adds a new object node to the current node
			struct do_object : public ast_action
			{
				do_object(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					std::string token(first, last);

					// Add it to the current node, and set ourselves as the current
					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = TOK_STRING;
					node->file = first.get_position().file;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					if(ast.current)
					{
						node->parent = ast.current;
						node->parent->children.push_back(node);
					}
					else
					{
						node->parent = 0;
						ast.nodes->push_back(node);
					}
					ast.current = node.get();
				}
			};
			// This action adds the name identifier to the current node's children
			struct do_ident : public ast_action
			{
				do_ident(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					if(!ast.current)
						throw ParseErrorException(first.get_position().file,
							first.get_position().line, first.get_position().column, PE_PRECEDINGTAGEXPECTED);
					
					std::string token(first, last);

					// Add it to the current node
					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->file = first.get_position().file;
					node->line = first.get_position().line;
					node->type = TOK_STRING;
					node->column = first.get_position().column;
					node->parent = ast.current;
					node->parent->children.push_back(node);
				}
			};
			// This action handles the colon which dictates an inheritance of objects
			struct do_colon : public ast_action
			{
				do_colon(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					std::string token(first, last);

					// Add it to the current node and set ourselves as current
					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = TOK_STRING;
					node->file = first.get_position().file;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					if(ast.current)
					{
						node->parent = ast.current;
						node->parent->children.push_back(node);
					}
					else
					{
						node->parent = 0;
						ast.nodes->push_back(node);
					}
					ast.current = node.get();
				}
			};
			// This action starts a new property with its name and sets itself as the current.
			// This starts a new branch
			struct do_property : public ast_action
			{
				do_property(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					std::string token(first, last);

					// Add it to the current node and set ourselves as current
					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->type = TOK_STRING;
					node->file = first.get_position().file;
					node->line = first.get_position().line;
					node->column = first.get_position().column;
					if(ast.current)
					{
						node->parent = ast.current;
						node->parent->children.push_back(node);
					}
					else
					{
						node->parent = 0;
						ast.nodes->push_back(node);
					}
					ast.current = node.get();
				}
			};
			// This action adds a variable to the current parent
			struct do_variable : public ast_action
			{
				do_variable(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					if(!ast.current)
						throw ParseErrorException(first.get_position().file,
							first.get_position().line, first.get_position().column, PE_PRECEDINGTAGEXPECTED);
					
					std::string token(first, last);

					// Add it to the current node
					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->file = first.get_position().file;
					node->line = first.get_position().line;
					node->type = TOK_VAR;
					node->column = first.get_position().column;
					node->parent = ast.current;
					node->parent->children.push_back(node);
				}
			};
			// This action adds a number to the current parent
			struct do_number : public ast_action
			{
				do_number(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					if(!ast.current)
						throw ParseErrorException(first.get_position().file,
							first.get_position().line, first.get_position().column, PE_PRECEDINGTAGEXPECTED);
					
					std::string token(first, last);

					// Add it to the current node
					ScriptNodePtr node(new ScriptNode());
					node->token = token;
					node->file = first.get_position().file;
					node->line = first.get_position().line;
					node->type = TOK_NUMBER;
					node->column = first.get_position().column;
					node->parent = ast.current;
					node->parent->children.push_back(node);
				}
			};
			// This action is invoked when some closing sequence is found.
			// It walks one up the tree branch (}, \n or ;)
			struct do_close : public ast_action
			{
				do_close(AST &rhs):ast_action(rhs){}
				template<class IterT>
				void operator()(IterT first, IterT last) const{
					if(ast.current)
					{
						ast.current = ast.current->parent;
					}
				}
			};

			definition(ScriptGrammar const &self)
			{
				self.ast.nodes = ScriptNodeListPtr(new ScriptNodeList());
				self.ast.current = 0;

				stat_list = 
					*(import_stat|top_obj);

				import_stat =
					str_p("import")[do_import_stat(self.ast)] 
					>> 
					(script_path[do_script_path(self.ast)] | error_p(PE_SCRIPTPATHEXPECTED));

				script_path =
					lexeme_d[+(alnum_p|punct_p)]
					|
					confix_p('\"', lexeme_d[+(alnum_p|punct_p|' ')], '\"');

				top_obj =
					// Performs a "look-ahead" to see if we are dealing with an object
					// START syntax check
					eps_p(
						!str_p("abstract")
						>>
						lexeme_d[(alpha_p|'_') >> *(alnum_p|'_')] 
						>>
						!lexeme_d[(alpha_p|'_') >> *(alnum_p|'-'|'_'|'/'|'.')]
						>>
						!(ch_p(':') >> lexeme_d[(alpha_p|'_') >> *(alnum_p|'-'|'_'|'/'|'.')])
						>>
						'{'
					)
					// END syntax check
					>>
					!str_p("abstract")[do_abstract(self.ast)]
					>>
					word[do_object(self.ast)]
					>> 
					!(ident[do_ident(self.ast)])
						>> 
						!(str_p(":")[do_colon(self.ast)] 
							>> 
							(ident[do_ident(self.ast)] | error_p(PE_IDENTIFIEREXPECTED)))
					>> 
					(ch_p('{') | error_p(PE_OPENBRACEEXPECTED)) 
					>> 
					*(obj|expr) 
					>> 
					(str_p("}")[do_close(self.ast)] | error_p(PE_CLOSEBRACEEXPECTED));

				obj =
					// Performs a "look-ahead" to see if we are dealing with an object
					// START syntax check
					eps_p(
						lexeme_d[(alpha_p|'_') >> *(alnum_p|'_')] 
						>>
						!lexeme_d[(alpha_p|'_') >> *(alnum_p|'-'|'_'|'/'|'.')]
						>>
						!(ch_p(':') >> lexeme_d[(alpha_p|'_') >> *(alnum_p|'-'|'_'|'/'|'.')])
						>>
						'{'
					)
					// END syntax check
					>>
					word[do_object(self.ast)]
					>> 
					!(ident[do_ident(self.ast)])
						>> 
						!(str_p(":")[do_colon(self.ast)] 
							>> 
							(ident[do_ident(self.ast)] | error_p(PE_IDENTIFIEREXPECTED)))
					>> 
					(ch_p('{') | error_p(PE_OPENBRACEEXPECTED)) 
					>> 
					*(obj|expr) 
					>> 
					(str_p("}")[do_close(self.ast)] | error_p(PE_CLOSEBRACEEXPECTED));

				expr = 
					lexeme_d
					[
						// word
						((alpha_p|'_') >> *(alnum_p|'_'))[do_property(self.ast)]
						>>
						// whitespace
						*(ch_p(' ')|'\t'|'\f')
						>>
						// ident|variable|real_p
						*(
							(((alpha_p|'_') >> *(alnum_p|'-'|'_'|'/'|'.'))[do_ident(self.ast)]
							|
							(ch_p('$') >> *(alnum_p|'_'))[do_variable(self.ast)]
							|
							(eps_p >> real_p)[do_number(self.ast)])
							>>
							*(ch_p(' ')|'\t'|'\f')
						)
						>>
						// newline|';'|'}'
						((str_p("\n")|";"|"}")[do_close(self.ast)] | error_p(PE_NEWLINEEXPECTED))
					];

				word = lexeme_d[(alpha_p|'_') >> *(alnum_p|'_')];
				ident = lexeme_d[(alpha_p|'_') >> *(alnum_p|'-'|'_'|'/'|'.')];
			}

			rule<ScannerT> const &start(){return stat_list;}
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
		case PE_PRECEDINGTAGEXPECTED:
			error = "preceding tag expected";
			break;
		case PE_SCRIPTPATHEXPECTED:
			error = "script file path or file name expected";
			break;
		case PE_IDENTIFIEREXPECTED:
			error = "identifier expected";
			break;
		case PE_OPENBRACEEXPECTED:
			error = "open brace \'{\' expected";
			break;
		case PE_CLOSEBRACEEXPECTED:
			error = "close brace \'}\' expected";
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