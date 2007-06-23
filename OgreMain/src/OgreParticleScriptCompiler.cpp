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
#include "OgreParticleScriptCompiler.h"
#include "OgreParticleSystemManager.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"

namespace Ogre{

	// ParticleScriptCompilerListener
	ParticleScriptCompilerListener::ParticleScriptCompilerListener()
	{
	}

	bool ParticleScriptCompilerListener::processNode(ScriptNodeList::iterator &iter, ScriptNodeList::iterator &end, Ogre::ParticleScriptCompiler *)
	{
		return false;
	}

	ParticleSystem *ParticleScriptCompilerListener::getParticleSystem(const Ogre::String &name, const Ogre::String &group)
	{
		// By default create a new template
		return ParticleSystemManager::getSingleton().createTemplate(name, group);
	}

	// ParticleScriptCompiler
	ParticleScriptCompiler::ParticleScriptCompiler()
		:mListener(0), mSystem(0)
	{
		mAllowNontypedObjects = true;
	}

	void ParticleScriptCompiler::setListener(ParticleScriptCompilerListener *listener)
	{
		mListener = listener;
	}

	ParticleSystem *ParticleScriptCompiler::getParticleSystem() const
	{
		return mSystem;
	}

	bool ParticleScriptCompiler::compileImpl(ScriptNodeListPtr nodes)
	{
		ScriptNodeList::iterator i = nodes->begin();
		while(i != nodes->end())
		{
			// Delegate some processing to the listener
			if(!processNode(i, nodes->end()))
			{
				// At this point we expect either an abstract object
				// Or a ParticleSystem name
				if((*i)->token == "abstract")
				{
					// This is here just for inheriting from, so jump ahead
					i = findNode(i, nodes->end(), SNT_RBRACE);
					++i;
				}
				else
				{
					compileParticleSystem(i, nodes->end());
				}
			}
		}
		return mErrors.empty();
	}

	bool ParticleScriptCompiler::processNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		if(mListener)
			return mListener->processNode(i, end, this);
		return false;
	}

	ScriptNodeListPtr ParticleScriptCompiler::loadImportPath(const Ogre::String &name)
	{
		ScriptNodeListPtr nodes;

		// Try the listener
		if(mListener)
			nodes = mListener->importFile(name);

		// Try the base version
		if(nodes.isNull())
			nodes = ScriptCompiler::loadImportPath(name);

		return nodes;
	}

	void ParticleScriptCompiler::compileParticleSystem(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		// We expect this token to be the name of the system to compile
		if(!(*i)->type == SNT_STRING)
		{
			addError(CE_OBJECTNAMEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		// Use the listener to get the particle system object
		if(mListener)
			mSystem = mListener->getParticleSystem((*i)->token, mGroup);
		else
			mSystem = ParticleSystemManager::getSingleton().createTemplate((*i)->token, mGroup);

		++i;

		// At this point there is either extra data, or the '{' to start the system body
		while((*i)->type != SNT_LBRACE && i != end)
		{
			if(!processNode(i, end))
				++i;
		}

		// We hit the '{', so descend into it to continue compilation
		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				// Each property in the particle system has only 1 value associated with it
				String name = (*j)->token, value;

				if(name == "emitter")
				{
					++j; // Advance past the emitter tag
					compileEmitter(j, (*i)->children.end());
				}
				else if(name == "affector")
				{
					++j; // Advance past the affector tag
					compileAffector(j, (*i)->children.end());
				}
				else
				{
					ScriptNodeList::iterator k = j;
					++k; // Move forward to check for a value
					value = getPropertyValue(k, (*i)->children.end());

					// There needs to be a value set before we move on
					if(!value.empty())
					{
						bool propertySet = false;
						// Attempt to set it on the main system
						if(!mSystem->setParameter(name, value))
						{
							// Attempt to set the value on the renderer
							if(mSystem->getRenderer())
								propertySet = mSystem->getRenderer()->setParameter(name, value);
						}
						else
							propertySet = true;

						// Add an error if the property couldn't be set
						if(!propertySet)
							addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
					}
					else
					{
						addError(CE_VALUEEXPECTED, (*j)->file, (*j)->line, (*j)->column);
					}
					j = k;
				}
			}
		}

		// We are finished with the object, so consume the '}'
		++i; // '{'
		++i; // '}'
	}

	void ParticleScriptCompiler::compileEmitter(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		// Get the next token, which should be the emitter type
		if(i != end)
		{
			ParticleEmitter *emitter = 0;
			String type = (*i)->token;
			try{
				emitter = mSystem->addEmitter(type);
			}catch(...){
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			}

			i = findNode(i, end, SNT_LBRACE);
			if(emitter != 0 && i != end)
			{
				// Use the children of the '{' node to set the properties
				ScriptNodeList::iterator j = (*i)->children.begin();
				while(j != (*i)->children.end())
				{
					if(!processNode(j, (*i)->children.end()))
					{
						String name = (*j)->token, value;

						ScriptNodeList::iterator k = j;
						++k; // Move forward to check for a value
						value = getPropertyValue(k, (*i)->children.end());

						// There must be a value
						if(!value.empty())
						{
							if(!emitter->setParameter(name, value))
								addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
						}
						else
						{
							addError(CE_VALUEEXPECTED, (*j)->file, (*j)->line, (*j)->column);
						}
						j = k;
					}
				}
			}
		}
	}

	void ParticleScriptCompiler::compileAffector(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		// Get the next token, which should be the emitter type
		if(i != end)
		{
			ParticleAffector *affector = 0;
			String type = (*i)->token;
			try{
				affector = mSystem->addAffector(type);
			}catch(...){
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			}

			i = findNode(i, end, SNT_LBRACE);
			if(affector != 0 && i != end)
			{
				// Use the children of the '{' node to set the properties
				ScriptNodeList::iterator j = (*i)->children.begin();
				while(j != (*i)->children.end())
				{
					if(!processNode(j, (*i)->children.end()))
					{
						String name = (*j)->token, value;

						ScriptNodeList::iterator k = j;
						++k; // Move forward to check for a value
						value = getPropertyValue(k, (*i)->children.end());

						// There must be a value
						if(!value.empty())
						{
							if(!affector->setParameter(name, value))
								addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
						}
						else
						{
							addError(CE_VALUEEXPECTED, (*j)->file, (*j)->line, (*j)->column);
						}
						j = k;
					}
				}
			}
		}
	}

	String ParticleScriptCompiler::getPropertyValue(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		String value;

		if(i != end)
		{
			// Get the first part of this value
			value = (*i)->token;
			++i; // Next

			while(i != end && (*i)->type == SNT_NUMBER) // Multiple numbers may be grouped together into one value
			{
				value = value + " " + (*i)->token;
				++i;
			}
		}

		return value;
	}
}