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
#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <list>

#include <boost/signal.hpp>

#include "OgreMaterial.h"
#include "OgreString.h"

namespace Ogre
{
	class SceneManager;
}

class MaterialController;
class Project;

using Ogre::Material;
using Ogre::MaterialPtr;
using Ogre::String;

typedef std::list<MaterialController*> MaterialControllerList;


typedef boost::signal<void (Project*, const String&)> NameChangedDelegate;
typedef boost::signal<void (Project*, MaterialController*)> MaterialAddedDelegate;
typedef boost::signal<void (Project*, MaterialController*)> MaterialRemovedDelegate;
typedef boost::signal<void (Project*, MaterialController*)> ActiveMaterialChangedDelegate;

enum ProjectEvent
{
	NamedChanged,
	MaterialAdded,
	MaterialRemoved,
	ActiveMaterialChanged
};

class Project
{
public:
	Project();
	virtual ~Project();

	const String& getName() const;
	void setName(const String& name);

	void addMaterial(MaterialPtr materialPtr);

	void removeMaterial(MaterialController* controller);
	void removeMaterial(Material* material);
	void removeMaterial(const String& name);
	
	MaterialController* getActiveMaterial() const;
	void setActiveMaterial(MaterialController* controller);
	void setActiveMaterial(Material* material);
	void setActiveMaterial(const String& name);
	
	MaterialController* getMaterialController(const String& name);
	
	const MaterialControllerList* getMaterials() const;

	void open();
	void close();

	bool isOpen();
	bool isClosed();

	void generateScene(Ogre::SceneManager* sceneManager);
	
	// use boost::bind for the functor so tracking is enabled
	template<class T>
	void subscribe(ProjectEvent event, T& functor)
	{
		switch(event)
		{
		case ProjectEvent::NameChanged:
				mNameChanged.connect(functor);
			break;
			
		case ProjectEvent::MaterialAdded:
				mMaterialAdded.connect(functor);
		break;
			
		case ProjectEvent::MaterialRemoved:
				mMaterialRemoved.connect(functor);
		break;
			
		case ProjectEvent::ActiveMaterialChanged:
				mActiveMaterialChanged.connect(functor);
		break;
		}
	}

protected:
	String mName;
	bool mOpen;
	MaterialController* mActiveMaterial;
	MaterialControllerList mMaterialControllers;
	
	NameChangedDelegate mNameChanged;
	MaterialAddedDelegate mMaterialAdded;
	MaterialRemovedDelegate mMaterialRemoved;
	ActiveMaterialChangedDelegate mActiveMaterialChanged;
};

#endif // _PROJECT_H_