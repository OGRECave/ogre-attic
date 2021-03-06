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
#include "OgreException.h"


namespace Ogre
{

	ScriptParser::ScriptParser()
	{
	}

	ConcreteNodeListPtr ScriptParser::parse(const ScriptTokenListPtr &tokens)
	{
		ConcreteNodeListPtr nodes(new ConcreteNodeList());

		enum{READY, OBJECT};
		uint32 state = READY;

		ConcreteNode *parent = 0;
		ConcreteNodePtr node;
		ScriptToken *token = 0;
		ScriptTokenList::iterator i = tokens->begin(), end = tokens->end();
		while(i != end)
		{
			token = (*i).get();

			switch(state)
			{
			case READY:
				if(token->type == TID_WORD)
				{
					if(token->lexeme == "import")
					{
						node = ConcreteNodePtr(new ConcreteNode());
						node->token = token->lexeme;
						node->file = token->file;
						node->line = token->line;
						node->type = CNT_IMPORT;

						// The next token is the target
						++i;
						if(i == end || ((*i)->type != TID_WORD && (*i)->type != TID_QUOTE))
							OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
								Ogre::String("expected import target at line ") + 
									Ogre::StringConverter::toString(node->line),
								"ScriptParser::parse");
						ConcreteNodePtr temp(new ConcreteNode());
						temp->parent = node.get();
						temp->file = (*i)->file;
						temp->line = (*i)->line;
						temp->type = (*i)->type == TID_WORD ? CNT_WORD : CNT_QUOTE;
						if(temp->type == CNT_QUOTE)
							temp->token = (*i)->lexeme.substr(1, token->lexeme.size() - 2);
						else
							temp->token = (*i)->lexeme;
						node->children.push_back(temp);

						// The second-next token is the source
						++i;
						++i;
						if(i == end || ((*i)->type != TID_WORD && (*i)->type != TID_QUOTE))
							OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
								Ogre::String("expected import source at line ") + 
									Ogre::StringConverter::toString(node->line),
								"ScriptParser::parse");
						temp = ConcreteNodePtr(new ConcreteNode());
						temp->parent = node.get();
						temp->file = (*i)->file;
						temp->line = (*i)->line;
						temp->type = (*i)->type == TID_WORD ? CNT_WORD : CNT_QUOTE;
						if(temp->type == CNT_QUOTE)
							temp->token = (*i)->lexeme.substr(1, (*i)->lexeme.size() - 2);
						else
							temp->token = (*i)->lexeme;
						node->children.push_back(temp);

						// Consume all the newlines
						i = skipNewlines(i, end);

						// Insert the node
						if(parent)
						{
							node->parent = parent;
							parent->children.push_back(node);
						}
						else
						{
							node->parent = 0;
							nodes->push_back(node);
						}
						node = ConcreteNodePtr();
					}
					else if(token->lexeme == "set")
					{
						node = ConcreteNodePtr(new ConcreteNode());
						node->token = token->lexeme;
						node->file = token->file;
						node->line = token->line;
						node->type = CNT_VARIABLE_ASSIGN;

						// The next token is the variable
						++i;
						if(i == end || (*i)->type != TID_VARIABLE)
							OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
								Ogre::String("expected variable name at line ") + 
									Ogre::StringConverter::toString(node->line),
								"ScriptParser::parse");
						ConcreteNodePtr temp(new ConcreteNode());
						temp->parent = node.get();
						temp->file = (*i)->file;
						temp->line = (*i)->line;
						temp->type = CNT_VARIABLE;
						temp->token = (*i)->lexeme;
						node->children.push_back(temp);

						// The next token is the assignment
						++i;
						if(i == end || ((*i)->type != TID_WORD && (*i)->type != TID_QUOTE))
							OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
								Ogre::String("expected variable value at line ") + 
									Ogre::StringConverter::toString(node->line),
								"ScriptParser::parse");
						temp = ConcreteNodePtr(new ConcreteNode());
						temp->parent = node.get();
						temp->file = (*i)->file;
						temp->line = (*i)->line;
						temp->type = (*i)->type == TID_WORD ? CNT_WORD : CNT_QUOTE;
						if(temp->type == CNT_QUOTE)
							temp->token = (*i)->lexeme.substr(1, (*i)->lexeme.size() - 2);
						else
							temp->token = (*i)->lexeme;
						node->children.push_back(temp);

						// Consume all the newlines
						i = skipNewlines(i, end);

						// Insert the node
						if(parent)
						{
							node->parent = parent;
							parent->children.push_back(node);
						}
						else
						{
							node->parent = 0;
							nodes->push_back(node);
						}
						node = ConcreteNodePtr();
					}
					else
					{
						node = ConcreteNodePtr(new ConcreteNode());
						node->file = token->file;
						node->line = token->line;
						node->type = token->type == TID_WORD ? CNT_WORD : CNT_QUOTE;
						if(node->type == CNT_QUOTE)
							node->token = token->lexeme.substr(1, token->lexeme.size() - 2);
						else
							node->token = token->lexeme;

						// Insert the node
						if(parent)
						{
							node->parent = parent;
							parent->children.push_back(node);
						}
						else
						{
							node->parent = 0;
							nodes->push_back(node);
						}

						// Set the parent
						parent = node.get();

						// Switch states
						state = OBJECT;

						node = ConcreteNodePtr();
					}
				}
				else if(token->type == TID_RBRACKET)
				{
					// Go up one level if we can
					if(parent)
						parent = parent->parent;

					node = ConcreteNodePtr(new ConcreteNode());
					node->token = token->lexeme;
					node->file = token->file;
					node->line = token->line;
					node->type = CNT_RBRACE;

					// Consume all the newlines
					i = skipNewlines(i, end);

					// Insert the node
					if(parent)
					{
						node->parent = parent;
						parent->children.push_back(node);
					}
					else
					{
						node->parent = 0;
						nodes->push_back(node);
					}

					// Move up another level
					if(parent)
						parent = parent->parent;

					node = ConcreteNodePtr();
				}
				break;
			case OBJECT:
				if(token->type == TID_NEWLINE)
				{
					// Look ahead to the next non-newline token and if it isn't an {, this was a property
					ScriptTokenList::iterator next = skipNewlines(i, end);
					if(next == end || (*next)->type != TID_LBRACKET)
					{
						// Ended a property here
						if(parent)
							parent = parent->parent;
						state = READY;
					}
				}
				else if(token->type == TID_COLON)
				{
					node = ConcreteNodePtr(new ConcreteNode());
					node->token = token->lexeme;
					node->file = token->file;
					node->line = token->line;
					node->type = CNT_COLON;

					// The next token is the parent object
					++i;
					if(i == end || ((*i)->type != TID_WORD && (*i)->type != TID_QUOTE))
						OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
							Ogre::String("expected object identifier at line ") + 
								Ogre::StringConverter::toString(node->line),
							"ScriptParser::parse");
					ConcreteNodePtr temp = ConcreteNodePtr(new ConcreteNode());
					temp->parent = node.get();
					temp->file = (*i)->file;
					temp->line = (*i)->line;
					temp->type = (*i)->type == TID_WORD ? CNT_WORD : CNT_QUOTE;
					if(temp->type == CNT_QUOTE)
						temp->token = (*i)->lexeme.substr(1, (*i)->lexeme.size() - 2);
					else
						temp->token = (*i)->lexeme;
					node->children.push_back(temp);

					// Consume all the newlines
					i = skipNewlines(i, end);

					// Insert the node
					if(parent)
					{
						node->parent = parent;
						parent->children.push_back(node);
					}
					else
					{
						node->parent = 0;
						nodes->push_back(node);
					}
					node = ConcreteNodePtr();
				}
				else if(token->type == TID_LBRACKET)
				{
					node = ConcreteNodePtr(new ConcreteNode());
					node->token = token->lexeme;
					node->file = token->file;
					node->line = token->line;
					node->type = CNT_LBRACE;

					// Consume all the newlines
					i = skipNewlines(i, end);

					// Insert the node
					if(parent)
					{
						node->parent = parent;
						parent->children.push_back(node);
					}
					else
					{
						node->parent = 0;
						nodes->push_back(node);
					}

					// Set the parent
					parent = node.get();

					// Change the state
					state = READY;

					node = ConcreteNodePtr();
				}
				else if(token->type == TID_RBRACKET)
				{
					// Go up one level if we can
					if(parent)
						parent = parent->parent;

					// If the parent is currently a { then go up again
					if(parent && parent->type == CNT_LBRACE && parent->parent)
						parent = parent->parent;

					node = ConcreteNodePtr(new ConcreteNode());
					node->token = token->lexeme;
					node->file = token->file;
					node->line = token->line;
					node->type = CNT_RBRACE;

					// Consume all the newlines
					i = skipNewlines(i, end);

					// Insert the node
					if(parent)
					{
						node->parent = parent;
						parent->children.push_back(node);
					}
					else
					{
						node->parent = 0;
						nodes->push_back(node);
					}

					// Move up another level
					if(parent)
						parent = parent->parent;

					node = ConcreteNodePtr();
					state = READY;
				}
				else if(token->type == TID_VARIABLE)
				{
					node = ConcreteNodePtr(new ConcreteNode());
					node->token = token->lexeme;
					node->file = token->file;
					node->line = token->line;
					node->type = CNT_VARIABLE;

					// Insert the node
					if(parent)
					{
						node->parent = parent;
						parent->children.push_back(node);
					}
					else
					{
						node->parent = 0;
						nodes->push_back(node);
					}
					node = ConcreteNodePtr();
				}
				else if(token->type == TID_QUOTE)
				{
					node = ConcreteNodePtr(new ConcreteNode());
					node->token = token->lexeme.substr(1, token->lexeme.size() - 2);
					node->file = token->file;
					node->line = token->line;
					node->type = CNT_QUOTE;

					// Insert the node
					if(parent)
					{
						node->parent = parent;
						parent->children.push_back(node);
					}
					else
					{
						node->parent = 0;
						nodes->push_back(node);
					}
					node = ConcreteNodePtr();
				}
				else if(token->type == TID_WORD)
				{
					node = ConcreteNodePtr(new ConcreteNode());
					node->token = token->lexeme;
					node->file = token->file;
					node->line = token->line;
					node->type = CNT_WORD;

					// Insert the node
					if(parent)
					{
						node->parent = parent;
						parent->children.push_back(node);
					}
					else
					{
						node->parent = 0;
						nodes->push_back(node);
					}
					node = ConcreteNodePtr();
				}
				break;
			}

			++i;
		}

		return nodes;
	}

	ConcreteNodeListPtr ScriptParser::parseChunk(const ScriptTokenListPtr &tokens)
	{
		ConcreteNodeListPtr nodes(new ConcreteNodeList());

		ConcreteNodePtr node;
		ScriptToken *token = 0;
		for(ScriptTokenList::const_iterator i = tokens->begin(); i != tokens->end(); ++i)
		{
			token = (*i).get();

			switch(token->type)
			{
			case TID_VARIABLE:
				node = ConcreteNodePtr(new ConcreteNode());
				node->file = token->file;
				node->line = token->line;
				node->parent = 0;
				node->token = token->lexeme;
				node->type = CNT_VARIABLE;
				break;
			case TID_WORD:
				node = ConcreteNodePtr(new ConcreteNode());
				node->file = token->file;
				node->line = token->line;
				node->parent = 0;
				node->token = token->lexeme;
				node->type = CNT_WORD;
				break;
			case TID_QUOTE:
				node = ConcreteNodePtr(new ConcreteNode());
				node->file = token->file;
				node->line = token->line;
				node->parent = 0;
				node->token = token->lexeme.substr(1, token->lexeme.size() - 2);
				node->type = CNT_QUOTE;
			default:
				OGRE_EXCEPT(Exception::ERR_INVALID_STATE, 
					Ogre::String("unexpected token") + token->lexeme + " at line " + 
						Ogre::StringConverter::toString(token->line),
					"ScriptParser::parseChunk");
			}

			if(!node.isNull())
				nodes->push_back(node);
		}

		return nodes;
	}

	ScriptToken *ScriptParser::getToken(ScriptTokenList::iterator i, ScriptTokenList::iterator end, int offset)
	{
		ScriptToken *token = 0;
		ScriptTokenList::iterator iter = i + offset;
		if(iter != end)
			token = (*i).get();
		return token;
	}

	ScriptTokenList::iterator ScriptParser::skipNewlines(ScriptTokenList::iterator i, ScriptTokenList::iterator end)
	{
		while(i != end && (*i)->type == TID_NEWLINE)
			++i;
		return i;
	}

}
