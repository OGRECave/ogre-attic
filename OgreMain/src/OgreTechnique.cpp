/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgreTechnique.h"
#include "OgreMaterial.h"
#include "OgrePass.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreGpuProgramManager.h"


namespace Ogre {
    //-----------------------------------------------------------------------------
    Technique::Technique(Material* parent)
        : mParent(parent), mIsSupported(false), mLodIndex(0)
    {
        // See above, defaults to unsupported until examined
    }
    //-----------------------------------------------------------------------------
    Technique::Technique(Material* parent, const Technique& oth)
        : mParent(parent), mLodIndex(0)
    {
        // Copy using operator=
        *this = oth;
    }
    //-----------------------------------------------------------------------------
    Technique::~Technique()
    {
        removeAllPasses();
        clearIlluminationPasses();
    }
    //-----------------------------------------------------------------------------
    bool Technique::isSupported(void) const
    {
        return mIsSupported;
    }
    //-----------------------------------------------------------------------------
    void Technique::_compile(bool autoManageTextureUnits)
    {
		// assume not supported
		mIsSupported = false;
        // Go through each pass, checking requirements
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            Pass* currPass = *i;
            // Check texture unit requirements
            size_t numTexUnitsRequested = currPass->getNumTextureUnitStates();
            const RenderSystemCapabilities* caps = 
                Root::getSingleton().getRenderSystem()->getCapabilities();
            unsigned short numTexUnits = caps->getNumTextureUnits();

            if (currPass->hasFragmentProgram())
            {
                // Check texture units
                if (numTexUnitsRequested > numTexUnits)
                {
                    // Can't do this one, and can't split a fragment pass
                    return;
                }
                // Check fragment program version
                if (!currPass->getFragmentProgram()->isSupported())
                {
                    // Can't do this one
                    return;
                }
            }
            else
            {
				// Check a few fixed-function options in texture layers
                Pass::TextureUnitStateIterator texi = currPass->getTextureUnitStateIterator();
				while (texi.hasMoreElements())
				{
					TextureUnitState* tex = texi.getNext();
					// Any Cube textures? NB we make the assumption that any 
					// card capable of running fragment programs can support
					// cubic textures, which has to be true, surely?
					if (tex->is3D() && !caps->hasCapability(RSC_CUBEMAPPING))
					{
						// Fail
						return;
					}
					// Any Dot3 blending?
					if (tex->getColourBlendMode().operation == LBX_DOTPRODUCT &&
							!caps->hasCapability(RSC_DOT3))
					{
						// Fail
						return;
					}
				}
				
				// We're ok on operations, now we need to check # texture units
				// Keep splitting this pass so long as units requested > gpu units
                while (numTexUnitsRequested > numTexUnits)
                {
                    // chop this pass into many passes
                    currPass = currPass->_split(numTexUnits);
                    numTexUnitsRequested = currPass->getNumTextureUnitStates();
                }
            }

            if (currPass->hasVertexProgram())
            {
                // Check vertex program version
                if (!currPass->getVertexProgram()->isSupported() )
                {
                    // Can't do this one
                    return;
                }
            }
		
		}
        // If we got this far, we're ok
        mIsSupported = true;

        // Now compile for categorised illumination, incase we need it
        _compileIlluminationPasses();

    }
    //-----------------------------------------------------------------------------
    Pass* Technique::createPass(void)
    {
		Pass* newPass = new Pass(this, static_cast<unsigned short>(mPasses.size()));
		mPasses.push_back(newPass);
		return newPass;
    }
    //-----------------------------------------------------------------------------
    Pass* Technique::getPass(unsigned short index)
    {
		assert(index < mPasses.size() && "Index out of bounds");
		return mPasses[index];
    }
    //-----------------------------------------------------------------------------
    unsigned short Technique::getNumPasses(void) const
    {
		return static_cast<unsigned short>(mPasses.size());
    }
    //-----------------------------------------------------------------------------
    void Technique::removePass(unsigned short index)
    {
		assert(index < mPasses.size() && "Index out of bounds");
		Passes::iterator i = mPasses.begin() + index;
		(*i)->queueForDeletion();
		mPasses.erase(i);
    }
    //-----------------------------------------------------------------------------
    void Technique::removeAllPasses(void)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->queueForDeletion();
        }
        mPasses.clear();
    }
    //-----------------------------------------------------------------------------
    const Technique::PassIterator Technique::getPassIterator(void)
    {
		return PassIterator(mPasses.begin(), mPasses.end());
    }
    //-----------------------------------------------------------------------------
    Technique& Technique::operator=(const Technique& rhs)
    {
		this->mIsSupported = rhs.mIsSupported;
		// copy passes
		removeAllPasses();
		Passes::const_iterator i, iend;
		iend = rhs.mPasses.end();
		for (i = rhs.mPasses.begin(); i != iend; ++i)
		{
			Pass* p = new Pass(this, (*i)->getIndex(), *(*i));
			mPasses.push_back(p);
		}
		return *this;
    }
    //-----------------------------------------------------------------------------
    bool Technique::isTransparent(void) const
    {
        if (mPasses.empty())
        {
            return false;
        }
        else
        {
            // Base decision on the transparency of the first pass
            return mPasses[0]->isTransparent();
        }
    }
    //-----------------------------------------------------------------------------
    void Technique::_load(void)
    {
		assert (mIsSupported && "This technique is not supported");
		// Load each pass
		Passes::iterator i, iend;
		iend = mPasses.end();
		for (i = mPasses.begin(); i != iend; ++i)
		{
			(*i)->_load();
		}
    }
    //-----------------------------------------------------------------------------
    void Technique::_unload(void)
    {
		// Unload each pass
		Passes::iterator i, iend;
		iend = mPasses.end();
		for (i = mPasses.begin(); i != iend; ++i)
		{
			(*i)->_unload();
		}
    }
    //-----------------------------------------------------------------------------
    bool Technique::isLoaded(void) const
    {
        return mParent->isLoaded();
    }
    //-----------------------------------------------------------------------
    void Technique::setAmbient(Real red, Real green, Real blue)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setAmbient(red, green, blue);
        }

    }
    //-----------------------------------------------------------------------
    void Technique::setAmbient(const ColourValue& ambient)
    {
        setAmbient(ambient.r, ambient.g, ambient.b);
    }
    //-----------------------------------------------------------------------
    void Technique::setDiffuse(Real red, Real green, Real blue)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setDiffuse(red, green, blue);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setDiffuse(const ColourValue& diffuse)
    {
        setDiffuse(diffuse.r, diffuse.g, diffuse.b);
    }
    //-----------------------------------------------------------------------
    void Technique::setSpecular(Real red, Real green, Real blue)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setSpecular(red, green, blue);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setSpecular(const ColourValue& specular)
    {
        setSpecular(specular.r, specular.g, specular.b);
    }
    //-----------------------------------------------------------------------
    void Technique::setShininess(Real val)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setShininess(val);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setSelfIllumination(Real red, Real green, Real blue)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setSelfIllumination(red, green, blue);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setSelfIllumination(const ColourValue& selfIllum)
    {
        setSelfIllumination(selfIllum.r, selfIllum.g, selfIllum.b);
    }
    //-----------------------------------------------------------------------
    void Technique::setDepthCheckEnabled(bool enabled)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setDepthCheckEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setDepthWriteEnabled(bool enabled)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setDepthWriteEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setDepthFunction( CompareFunction func )
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setDepthFunction(func);
        }
    }
    //-----------------------------------------------------------------------
	void Technique::setColourWriteEnabled(bool enabled)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setColourWriteEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setCullingMode( CullingMode mode )
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setCullingMode(mode);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setManualCullingMode( ManualCullingMode mode )
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setManualCullingMode(mode);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setLightingEnabled(bool enabled)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setLightingEnabled(enabled);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setShadingMode( ShadeOptions mode )
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setShadingMode(mode);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setFog(bool overrideScene, FogMode mode, const ColourValue& colour,
        Real expDensity, Real linearStart, Real linearEnd)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setFog(overrideScene, mode, colour, expDensity, linearStart, linearEnd);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setDepthBias(ushort bias)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setDepthBias(bias);
        }
    }
    //-----------------------------------------------------------------------
    void Technique::setTextureFiltering(TextureFilterOptions filterType)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setTextureFiltering(filterType);
        }
    }
    // --------------------------------------------------------------------
    void Technique::setTextureAnisotropy(unsigned int maxAniso)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setTextureAnisotropy(maxAniso);
        }
    }
    // --------------------------------------------------------------------
    void Technique::setSceneBlending( const SceneBlendType sbt )
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setSceneBlending(sbt);
        }
    }
    // --------------------------------------------------------------------
    void Technique::setSceneBlending( const SceneBlendFactor sourceFactor, 
        const SceneBlendFactor destFactor)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            (*i)->setSceneBlending(sourceFactor, destFactor);
        }
    }

    //-----------------------------------------------------------------------
    void Technique::_notifyNeedsRecompile(void)
    {
        mParent->_notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Technique::setLodIndex(unsigned short index)
    {
        mLodIndex = index;
        _notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Technique::_compileIlluminationPasses(void)
    {
        clearIlluminationPasses();

        if (isTransparent())
        {
            // Don't need to split transparents since they are rendered separately
            return;
        }

        Passes::iterator i, iend;
        iend = mPasses.end();
        i = mPasses.begin();
        
        IlluminationStage iStage = IS_AMBIENT;

        while (i != iend)
        {
            IlluminationPass* iPass;
            Pass* p = *i;
            switch(iStage)
            {
            case IS_AMBIENT:
                // Keep looking for ambient only
                if (p->isAmbientOnly())
                {
                    // Add this pass wholesale
                    iPass = new IlluminationPass();
                    iPass->destroyOnShutdown = false;
                    iPass->originalPass = iPass->pass = p;
                    iPass->stage = iStage;
                    mIlluminationPasses.push_back(iPass);
                    // progress to next pass
                    ++i;
                }
                else
                {
                    // Split off any ambient part
                    if (p->getAmbient() != ColourValue::Black ||
                        p->getSelfIllumination() != ColourValue::Black)
                    {
                        // Copy existing pass
                        Pass* newPass = new Pass(this, p->getIndex());
                        *newPass = *p;
                        // Remove any texture units
                        newPass->removeAllTextureUnitStates();
                        // Remove any fragment program
                        if (newPass->hasFragmentProgram())
                            newPass->setFragmentProgram("");
                        // We have to leave vertex program alone (if any) and
                        // just trust that the author is using light bindings, which 
                        // we will ensure there are none in the ambient pass
                        newPass->setDiffuse(ColourValue::Black);
                        newPass->setSpecular(ColourValue::Black);

                        // If ambient & emissive are zero, then no colour write
                        if (newPass->getAmbient() == ColourValue::Black && 
                            newPass->getSelfIllumination() == ColourValue::Black)
                        {
                            newPass->setColourWriteEnabled(false);
                        }

                        iPass = new IlluminationPass();
                        iPass->destroyOnShutdown = true;
                        iPass->originalPass = p;
                        iPass->pass = newPass;
                        iPass->stage = iStage;

                        mIlluminationPasses.push_back(iPass);
                        
                    }
                    // This means we're done with ambients, progress to per-light
                    iStage = IS_PER_LIGHT;
                }
                break;
            case IS_PER_LIGHT:
                if (p->getRunOncePerLight())
                {
                    // If this is per-light already, use it directly
                    iPass = new IlluminationPass();
                    iPass->destroyOnShutdown = false;
                    iPass->originalPass = iPass->pass = p;
                    iPass->stage = iStage;
                    // progress to next pass
                    ++i;
                }
                else
                {
                    // Split off per-light details (can only be done for one)
                    if (p->getLightingEnabled() && 
                        (p->getDiffuse() != ColourValue::Black ||
                        p->getSpecular() != ColourValue::Black))
                    {
                        // Copy existing pass
                        Pass* newPass = new Pass(this, p->getIndex());
                        *newPass = *p;
                        // remove texture units
                        newPass->removeAllTextureUnitStates();
                        // remove fragment programs
                        if (newPass->hasFragmentProgram())
                            newPass->setFragmentProgram("");
                        // Cannot remove vertex program, have to assume that
                        // it will process diffuse lights, ambient will be turned off
                        newPass->setAmbient(ColourValue::Black);
                        newPass->setSelfIllumination(ColourValue::Black);
                        // must be additive
                        newPass->setSceneBlending(SBF_ONE, SBF_ONE);

                        iPass = new IlluminationPass();
                        iPass->destroyOnShutdown = true;
                        iPass->originalPass = p;
                        iPass->pass = newPass;
                        iPass->stage = iStage;

                        mIlluminationPasses.push_back(iPass);

                    }
                    // This means the end of per-light passes
                    iStage = IS_DECAL;
                }
                break;
            case IS_DECAL:
                // We just want a 'lighting off' pass to finish off
                // and only if there are texture units
                if (p->getNumTextureUnitStates() > 0)
                {
                    if (!p->getLightingEnabled())
                    {
                        // we assume this pass already combines as required with the scene
                        iPass = new IlluminationPass();
                        iPass->destroyOnShutdown = false;
                        iPass->originalPass = iPass->pass = p;
                        iPass->stage = iStage;
                        mIlluminationPasses.push_back(iPass);
                    }
                    else
                    {
                        // Copy the pass and tweak away the lighting parts
                        Pass* newPass = new Pass(this, p->getIndex());
                        *newPass = *p;
                        newPass->setAmbient(ColourValue::Black);
                        newPass->setDiffuse(ColourValue::Black);
                        newPass->setSpecular(ColourValue::Black);
                        newPass->setSelfIllumination(ColourValue::Black);
                        newPass->setLightingEnabled(false);
                        // modulate
                        newPass->setSceneBlending(SBF_DEST_COLOUR, SBF_ZERO);

                        // NB there is nothing we can do about vertex & fragment
                        // programs here, so people will just have to make their
                        // programs friendly-like if they want to use this technique
                        iPass = new IlluminationPass();
                        iPass->destroyOnShutdown = true;
                        iPass->originalPass = p;
                        iPass->pass = newPass;
                        iPass->stage = iStage;
                        mIlluminationPasses.push_back(iPass);

                    }
                }
                ++i; // always increment on decal, since nothing more to do with this pass

                break;
            }
        }

    }
    //-----------------------------------------------------------------------
    void Technique::clearIlluminationPasses(void)
    {
        IlluminationPassList::iterator i, iend;
        iend = mIlluminationPasses.end();
        for (i = mIlluminationPasses.begin(); i != iend; ++i)
        {
            if ((*i)->destroyOnShutdown)
            {
                (*i)->pass->queueForDeletion();
            }
            delete *i;
        }
        mIlluminationPasses.clear();
    }
    //-----------------------------------------------------------------------
    const Technique::IlluminationPassIterator 
    Technique::getIlluminationPassIterator(void)
    {
        return IlluminationPassIterator(mIlluminationPasses.begin(), 
            mIlluminationPasses.end());
    }


}
