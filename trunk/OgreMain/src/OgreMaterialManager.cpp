/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreMaterialManager.h"

#include "OgreMaterial.h"
#include "OgreStringVector.h"
#include "OgreLogManager.h"
#include "OgreSDDataChunk.h"
#include "OgreArchiveEx.h"
#include "OgreStringConverter.h"
#include "OgreBlendMode.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"
#include "OgreException.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> MaterialManager* Singleton<MaterialManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    MaterialManager::MaterialManager()
    {
	    mDefTextureFiltering = TFO_BILINEAR;
		mDefAniso = 1;

		// Set up default material - don't use name contructor as we want to avoid applying defaults
	    Material::mDefaultSettings = new Material();
	    Material::mDefaultSettings->mName = "DefaultSettings";
        // Add a single technique and pass, non-programmable
        Material::mDefaultSettings->createTechnique()->createPass();

	    // Set up a lit base white material
	    this->create("BaseWhite");
	    // Set up an unlit base white material
        Material* baseWhiteNoLighting = (Material*)this->create("BaseWhiteNoLighting");
        baseWhiteNoLighting->setLightingEnabled(false);

    }
    //-----------------------------------------------------------------------
    MaterialManager::~MaterialManager()
    {
        delete Material::mDefaultSettings;
	    // Resources cleared by superclass
    }
    //-----------------------------------------------------------------------
    void MaterialManager::parseScript(DataChunk& chunk)
    {
        // Delegate to serializer
        mSerializer.parseScript(chunk);
    }
    //-----------------------------------------------------------------------
    void MaterialManager::parseAllSources(const String& extension)
    {
	    StringVector materialFiles;
	    DataChunk* pChunk;

	    std::vector<ArchiveEx*>::iterator i = mVFS.begin();

	    // Specific archives
	    for (; i != mVFS.end(); ++i)
	    {
		    materialFiles = (*i)->getAllNamesLike( "./", extension);
		    for (StringVector::iterator si = materialFiles.begin(); si != materialFiles.end(); ++si)
		    {
			    SDDataChunk dat; pChunk = &dat;
			    (*i)->fileRead(si[0], &pChunk );
			    LogManager::getSingleton().logMessage("Parsing material script: " + si[0]);
			    mSerializer.parseScript(dat, si[0]);
		    }

	    }
	    // search common archives
	    for (i = mCommonVFS.begin(); i != mCommonVFS.end(); ++i)
	    {
		    materialFiles = (*i)->getAllNamesLike( "./", extension);
		    for (StringVector::iterator si = materialFiles.begin(); si != materialFiles.end(); ++si)
		    {
			    SDDataChunk dat; pChunk = &dat;
			    (*i)->fileRead(si[0], &pChunk );
			    LogManager::getSingleton().logMessage("Parsing material script: " + si[0]);
			    parseScript(dat);
		    }
	    }


    }
    //-----------------------------------------------------------------------
    MaterialManager& MaterialManager::getSingleton(void)
    {
	    return Singleton<MaterialManager>::getSingleton();
    }
    //-----------------------------------------------------------------------
    Resource* MaterialManager::create( const String& name)
    {
	    // Check name not already used
	    if (getByName(name) != 0)
		    Except(Exception::ERR_DUPLICATE_ITEM, "Material " + name + " already exists.",
			    "MaterialManager::create");

	    Material* m = new Material(name);
        this->add(m);

	    return m;
    }
    //-----------------------------------------------------------------------
	void MaterialManager::setDefaultTextureFiltering(TextureFilterOptions fo)
	{
		mDefTextureFiltering = fo;
	}
    //-----------------------------------------------------------------------
	TextureFilterOptions MaterialManager::getDefaultTextureFiltering() const
	{
		return mDefTextureFiltering;
	}
    //-----------------------------------------------------------------------
	void MaterialManager::setDefaultAnisotropy(int maxAniso)
	{
		mDefAniso = maxAniso;
	}
    //-----------------------------------------------------------------------
	int MaterialManager::getDefaultAnisotropy() const
	{
		return mDefAniso;
	}
}
