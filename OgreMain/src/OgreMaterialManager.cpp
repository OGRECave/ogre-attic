/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
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
#include "OgreArchive.h"
#include "OgreStringConverter.h"
#include "OgreBlendMode.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"
#include "OgreException.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> MaterialManager* Singleton<MaterialManager>::ms_Singleton = 0;
    MaterialManager* MaterialManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    MaterialManager& MaterialManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-----------------------------------------------------------------------
    MaterialManager::MaterialManager()
    {
	    mDefaultMinFilter = FO_LINEAR;
	    mDefaultMagFilter = FO_LINEAR;
	    mDefaultMipFilter = FO_POINT;
		mDefaultMaxAniso = 1;

		// Scripting is supported by this manager
		mScriptPatterns.push_back("*.program");
		mScriptPatterns.push_back("*.material");
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);

		// Loading order
		mLoadOrder = 100.0f;
		// Resource type
		mResourceType = "Material";

		// Register with resource group manager
		ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

    }
    //-----------------------------------------------------------------------
    MaterialManager::~MaterialManager()
    {
        mDefaultSettings.setNull();
	    // Resources cleared by superclass
		// Unregister with resource group manager
		ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
		ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
    }
	//-----------------------------------------------------------------------
	Resource* MaterialManager::createImpl(const String& name, ResourceHandle handle, 
		const String& group, bool isManual, ManualResourceLoader* loader,
        const NameValuePairList* params)
	{
		return new Material(this, name, handle, group, isManual, loader);
	}
    //-----------------------------------------------------------------------
	void MaterialManager::initialise(void)
	{
		// Set up default material - don't use name contructor as we want to avoid applying defaults
		mDefaultSettings = create("DefaultSettings", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        // Add a single technique and pass, non-programmable
        mDefaultSettings->createTechnique()->createPass();

	    // Set up a lit base white material
	    create("BaseWhite", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	    // Set up an unlit base white material
        MaterialPtr baseWhiteNoLighting = create("BaseWhiteNoLighting", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        baseWhiteNoLighting->setLightingEnabled(false);

	}
    //-----------------------------------------------------------------------
    void MaterialManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
        // Delegate to serializer
        mSerializer.parseScript(stream, groupName);
    }
    //-----------------------------------------------------------------------
	void MaterialManager::setDefaultTextureFiltering(TextureFilterOptions fo)
	{
        switch (fo)
        {
        case TFO_NONE:
            setDefaultTextureFiltering(FO_POINT, FO_POINT, FO_NONE);
            break;
        case TFO_BILINEAR:
            setDefaultTextureFiltering(FO_LINEAR, FO_LINEAR, FO_POINT);
            break;
        case TFO_TRILINEAR:
            setDefaultTextureFiltering(FO_LINEAR, FO_LINEAR, FO_LINEAR);
            break;
        case TFO_ANISOTROPIC:
            setDefaultTextureFiltering(FO_ANISOTROPIC, FO_ANISOTROPIC, FO_LINEAR);
            break;
        }
	}
    //-----------------------------------------------------------------------
	void MaterialManager::setDefaultAnisotropy(unsigned int maxAniso)
	{
		mDefaultMaxAniso = maxAniso;
	}
    //-----------------------------------------------------------------------
	unsigned int MaterialManager::getDefaultAnisotropy() const
	{
		return mDefaultMaxAniso;
	}
    //-----------------------------------------------------------------------
    void MaterialManager::setDefaultTextureFiltering(FilterType ftype, FilterOptions opts)
    {
        switch (ftype)
        {
        case FT_MIN:
            mDefaultMinFilter = opts;
            break;
        case FT_MAG:
            mDefaultMagFilter = opts;
            break;
        case FT_MIP:
            mDefaultMipFilter = opts;
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialManager::setDefaultTextureFiltering(FilterOptions minFilter, 
        FilterOptions magFilter, FilterOptions mipFilter)
    {
        mDefaultMinFilter = minFilter;
        mDefaultMagFilter = magFilter;
        mDefaultMipFilter = mipFilter;
    }
    //-----------------------------------------------------------------------
    FilterOptions MaterialManager::getDefaultTextureFiltering(FilterType ftype) const
    {
        switch (ftype)
        {
        case FT_MIN:
            return mDefaultMinFilter;
        case FT_MAG:
            return mDefaultMagFilter;
        case FT_MIP:
            return mDefaultMipFilter;
        }
        // to keep compiler happy
        return mDefaultMinFilter;
    }
}
