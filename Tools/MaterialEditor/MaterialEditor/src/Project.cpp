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
#include "Project.h"

#include "OgreSceneManager.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"

#include "MaterialController.h"
#include "ProjectEventArgs.h"

using Ogre::MaterialManager;

Project::Project() : mActiveMaterial(NULL)
{
}

Project::Project(const String& name) : mActiveMaterial(NULL), mName(name)
{
}

Project::~Project()
{
	MaterialControllerList::iterator it;
	for(it = mMaterialControllers.begin(); it != mMaterialControllers.end(); ++it)
	{
		delete *it;
	}
	
	mMaterialControllers.clear();
}

const String& Project::getName() const
{
	return mName;
}

void Project::setName(const String& name)
{
	mName = name;
	
	fireEvent(NameChanged, ProjectEventArgs(this));
}

void Project::addMaterial(MaterialPtr materialPtr)
{
	MaterialController* controller = new MaterialController(materialPtr);
	mMaterialControllers.push_back(controller);
	
	fireEvent(MaterialAdded, ProjectEventArgs(this));
}

void Project::removeMaterial(MaterialController* controller)
{
	MaterialControllerList::iterator it;
	for(it = mMaterialControllers.begin(); it != mMaterialControllers.end(); ++it)
	{
		if(*it == controller) 
		{
			mMaterialControllers.erase(it);
			break;
		}
	}
	
	fireEvent(MaterialRemoved, ProjectEventArgs(this));
}

void Project::removeMaterial(Material* material)
{
	removeMaterial(getMaterialController(material->getName()));
}

void Project::removeMaterial(const String& name)
{
	removeMaterial(getMaterialController(name));
}

MaterialController* Project::getActiveMaterial() const
{
	return mActiveMaterial;
}

void Project::setActiveMaterial(MaterialController* controller)
{
	assert(controller);
	
	if(controller == mActiveMaterial) return;

	mActiveMaterial = controller;
	
	fireEvent(ActiveMaterialChanged, ProjectEventArgs(this));
}

void Project::setActiveMaterial(Material* material)
{
	setActiveMaterial(getMaterialController(material->getName()));
}

void Project::setActiveMaterial(const String& name)
{
	setActiveMaterial(getMaterialController(name));
}

MaterialController* Project::getMaterialController(const String& name)
{
	MaterialController* mc;
	MaterialControllerList::iterator it;
	for(it = mMaterialControllers.begin(); it != mMaterialControllers.end(); ++it)
	{
		mc = (*it);
		if(mc->getMaterial()->getName() == name) return mc;
	}

	return NULL;
}

void Project::open()
{
}

void Project::close()
{
}

void Project::generateScene(Ogre::SceneManager* sceneManager)
{
}
