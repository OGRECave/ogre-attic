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
#include "OgreTechnique.h"

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

    }
    //-----------------------------------------------------------------------
    Material::Material( const String& name )
    {
	    applyDefaults();

	    // Assign name
	    mName = name;
        mCompilationRequired = true;
		
    }
    //-----------------------------------------------------------------------
    Material& Material::operator=(const Material& rhs)
    {
	    mName = rhs.mName;
	    mHandle = rhs.mHandle;
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
            if ((*i)->isSupported())
                mSupportedTechniques.push_back(t);
        }
        mCompilationRequired = rhs.mCompilationRequired;

	    return *this;
    }


    //-----------------------------------------------------------------------
    const String& Material::getName(void) const
    {
	    return mName;
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
        mCompilationRequired = true;

    }
    //-----------------------------------------------------------------------
    Technique* Material::createTechnique(void)
    {
        Technique *t = new Technique(this);
        mTechniques.push_back(t);
        mCompilationRequired = true;
        return t;
    }
    //-----------------------------------------------------------------------
    Technique* Material::getTechnique(unsigned short index)
    {
        assert (index < mTechniques.size() && "Index out of bounds.");
        return mTechniques[index];
    }
    //-----------------------------------------------------------------------
    Technique* Material::getBestTechnique(void)
    {
        if (mSupportedTechniques.empty())
        {
            return NULL;
        }
        else
        {
            return mSupportedTechniques[0];
        }
    }
    //-----------------------------------------------------------------------
    void Material::removeTechnique(unsigned short index)
    {
        assert (index < mTechniques.size() && "Index out of bounds.");
        Techniques::iterator i = mTechniques.begin() + index;
        delete(*i);
        mTechniques.erase(i);
        mSupportedTechniques.clear();
        mCompilationRequired = true;
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
        mSupportedTechniques.clear();
        mCompilationRequired = true;
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
        if (!mCompilationRequired) return;

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
        mCompilationRequired = false;
    }
    //-----------------------------------------------------------------------
    void Material::setAmbient(Real red, Real green, Real blue)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setAmbient(red, green, blue);
        }

    }
    //-----------------------------------------------------------------------
    void Material::setAmbient(const ColourValue& ambient)
    {
        setAmbient(ambient.r, ambient.g, ambient.b);
    }
    //-----------------------------------------------------------------------
    void Material::setDiffuse(Real red, Real green, Real blue)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setDiffuse(red, green, blue);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setDiffuse(const ColourValue& diffuse)
    {
        setDiffuse(diffuse.r, diffuse.g, diffuse.b);
    }
    //-----------------------------------------------------------------------
    void Material::setSpecular(Real red, Real green, Real blue)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setSpecular(red, green, blue);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setSpecular(const ColourValue& specular)
    {
        setSpecular(specular.r, specular.g, specular.b);
    }
    //-----------------------------------------------------------------------
    void Material::setShininess(Real val)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setShininess(val);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setSelfIllumination(Real red, Real green, Real blue)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setSelfIllumination(red, green, blue);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setSelfIllumination(const ColourValue& selfIllum)
    {
        setSelfIllumination(selfIllum.r, selfIllum.g, selfIllum.b);
    }
    //-----------------------------------------------------------------------
    void Material::setDepthCheckEnabled(bool enabled)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setDepthCheckEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setDepthWriteEnabled(bool enabled)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setDepthWriteEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setDepthFunction( CompareFunction func )
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setDepthFunction(func);
        }
    }
    //-----------------------------------------------------------------------
	void Material::setColourWriteEnabled(bool enabled)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setColourWriteEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setCullingMode( CullingMode mode )
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setCullingMode(mode);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setManualCullingMode( ManualCullingMode mode )
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setManualCullingMode(mode);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setLightingEnabled(bool enabled)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setLightingEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setShadingMode( ShadeOptions mode )
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setShadingMode(mode);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setFog(bool overrideScene, FogMode mode, const ColourValue& colour,
        Real expDensity, Real linearStart, Real linearEnd)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setFog(overrideScene, mode, colour, expDensity, linearStart, linearEnd);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setDepthBias(ushort bias)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setDepthBias(bias);
        }
    }
    //-----------------------------------------------------------------------
    void Material::setTextureFiltering(TextureFilterOptions filterType)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setTextureFiltering(filterType);
        }
    }
    // --------------------------------------------------------------------
    void Material::setTextureAnisotropy(int maxAniso)
    {
        Techniques::iterator i, iend;
        iend = mTechniques.end();
        for (i = mTechniques.begin(); i != iend; ++i)
        {
            (*i)->setTextureAnisotropy(maxAniso);
        }
    }
    // --------------------------------------------------------------------




}

