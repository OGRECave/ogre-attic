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

#include "OgreMaterial.h"

#include "OgreSceneManagerEnumerator.h"
#include "OgreMaterialManager.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

    Material* Material::mDefaultSettings = 0;

    //-----------------------------------------------------------------------
    Material::Material()
    {
	    static unsigned short num = 1;
	    char name[14];

	    sprintf(name, "Undefined%d", num++);
	    mName = name;
        mCompilationRequired = true;

	    mHandle = -1;

    }
    //-----------------------------------------------------------------------
    Material::Material( const String& name )
    {
	    applyDefaults();

	    // Assign name
	    mName = name;
	    mHandle = -1;
        mCompilationRequired = true;
		
		/** TODO: MOVE TO TEXTUREUNITSTATE
        // Apply default texture filtering & anisotropy level
		mTextureFiltering = MaterialManager::getSingleton().getDefaultTextureFiltering();
		mMaxAniso = MaterialManager::getSingleton().getDefaultAnisotropy();
        */
    }
    //-----------------------------------------------------------------------
    Material& Material::operator=(const Material& rhs)
    {
	    mName = rhs.mName;
	    mHandle = rhs.mHandle;
        mCompilationRequired = rhs.mCompilationRequired;
        mSize = rhs.mSize;
        mLastAccess = rhs.mLastAccess;



        // Copy Techniques
        this->removeAllTechniques();
        Techniques::const_iterator i, iend;
        iend = rhs.mTechniques.end();
        for(i = rhs.mTechniques.begin(); i != iend; ++i)
        {
            Technique* t = this->createTechnique();
            *t = *(*i);
        }

	    return *this;
    }


    //-----------------------------------------------------------------------
    // Single definition of method to retrieve next handle, to avoid duplication of static member value
    void Material::assignNextHandle(void)
    {
	    static int nextHandle = 1;

	    mHandle = nextHandle++;
    }
    //-----------------------------------------------------------------------
    const String& Material::getName(void) const
    {
	    return mName;
    }
    //-----------------------------------------------------------------------
    int Material::getHandle(void) const
    {
	    return mHandle;
    }
    //-----------------------------------------------------------------------
    void Material::load(void)
    {
	    if (!mIsLoaded)
	    {
			// compile if required
            compile();

            // Load all supported techniques
            Techniques::iterator i, iend;
            iend = mSupportedTechniques.end();
            for (i = mSupportedTechniques.begin(); i != iend; ++i)
            {
                (*i)->_load();
            }

            mIsLoaded = true;

	    }
    }
    //-----------------------------------------------------------------------
    void Material::unload(void)
    {
        if (mIsLoaded)
        {
            // Unload all supported techniques
            Techniques::iterator i, iend;
            iend = mSupportedTechniques.end();
            for (i = mSupportedTechniques.begin(); i != iend; ++i)
            {
                (*i)->_unload();
            }
            mIsLoaded = false;
        }
    }
    //-----------------------------------------------------------------------
    Material* Material::clone(const String& newName)
    {
        Material* newMat = (Material*)MaterialManager::getSingleton().create(newName);

        // Keep handle (see below, copy overrides everything)
        int newHandle = newMat->getHandle();
        // Assign values from this
        *newMat = *this;
		newMat->mIsLoaded = this->mIsLoaded;
        // Correct the name & handle, they get copied too
        newMat->mName = newName;
        newMat->mHandle = newHandle;

        return newMat;



    }
    //-----------------------------------------------------------------------
    void Material::copyDetailsTo(Material* mat)
    {
        // Keep handle (see below, copy overrides everything)
        int savedHandle = mat->mHandle;
        String savedName = mat->mName;
        // Assign values from this
        *mat = *this;
        // Correct the name & handle, they get copied too
        mat->mName = savedName;
        mat->mHandle = savedHandle;

    }
    //-----------------------------------------------------------------------
    void Material::applyDefaults(void)
    {
	    *this = *mDefaultSettings;
    }
    //-----------------------------------------------------------------------
    Technique* Material::createTechnique(void)
    {
        Technique *t = new Technique(this);
        mTechniques.push_back(t);
        return t;
    }
    //-----------------------------------------------------------------------
    Technique* Material::getTechnique(unsigned short index)
    {
        assert (index < mTechniques.size() && "Index out of bounds.");
        return mTechniques[index];
    }
    //-----------------------------------------------------------------------
    void Material::removeTechnique(unsigned short index)
    {
        assert (index < mTechniques.size() && "Index out of bounds.");
        Techniques::iterator i = mTechniques.begin() + index;
        delete(*i);
        mTechniques.erase(i);
    }
    //-----------------------------------------------------------------------
    void Material::removeAllTechniques(void)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            delete(*i);
        }
        mTechniques.clear();
    }
    //-----------------------------------------------------------------------
    Material::TechniqueIterator Material::getTechniqueIterator(void) 
    {
        return TechniqueIterator(mTechniques.begin(), mTechniques.end());
    }
    //-----------------------------------------------------------------------
    Material::TechniqueIterator Material::getSupportedTechniqueIterator(void)
    {
        return TechniqueIterator(mSupportedTechniques.begin(), mSupportedTechniques.end());
    }
    //-----------------------------------------------------------------------
    bool Material::isTransparent(void) const
	{
		// Check each technique
		Techniques::const_iterator i, iend;
		iend = mTechniques.end();
		for (i = mTechniques.begin(); i != iend; ++i)
		{
			if ( (*i)->isTransparent() )
				return true;
		}
		return false;
	}
    //-----------------------------------------------------------------------
    void Material::compile(bool autoManageTextureUnits)
    {
        // Compile each technique, then add it to the list of supported techniques
        mSupportedTechniques.clear();

        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->_compile(autoManageTextureUnits);
            if ( (*i)->isSupported() )
            {
                mSupportedTechniques.push_back(*i);
            }
        }
    }
    //-----------------------------------------------------------------------



    /** TODO: MOVE TO TEXTUREUNITSTATE
    //-----------------------------------------------------------------------
	void Material::setAnisotropy(int maxAniso)
	{
		mMaxAniso = maxAniso;
		for (int n = 0; n < mNumTextureLayers; n++)
			mTextureLayers[n]._setDefTextureAnisotropy(mMaxAniso);
		mIsDefAniso = false;
	}
    //-----------------------------------------------------------------------
	int Material::getAnisotropy() const
	{
		return mMaxAniso;
	}
    //-----------------------------------------------------------------------
    void Material::_setDefTextureFiltering(TextureFilterOptions mode)
    {
		if (mIsDefFiltering)
		{
			mTextureFiltering = mode;
			for (int n = 0; n < mNumTextureLayers; n++)
				mTextureLayers[n]._setDefTextureLayerFiltering(mTextureFiltering);
		}
    }
    //-----------------------------------------------------------------------
    void Material::setTextureFiltering(TextureFilterOptions mode)
    {
	    mTextureFiltering = mode;
		for (int n = 0; n < mNumTextureLayers; n++)
			mTextureLayers[n]._setDefTextureLayerFiltering(mTextureFiltering);
		mIsDefFiltering = false;
    }
    //-----------------------------------------------------------------------
    TextureFilterOptions Material::getTextureFiltering(void) const
    {
	    return mTextureFiltering;
    }
    //-----------------------------------------------------------------------
    */


}

