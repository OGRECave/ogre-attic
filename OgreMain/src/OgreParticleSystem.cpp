/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreParticleSystem.h"
#include "OgreParticleSystemManager.h"
#include "OgreRenderQueue.h"
#include "OgreBillboardSet.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"
#include "OgreParticle.h"
#include "OgreSceneNode.h"
#include "OgreCamera.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreMaterialManager.h"



namespace Ogre {
    // Init statics
    ParticleSystem::CmdCull ParticleSystem::msCullCmd;
    ParticleSystem::CmdHeight ParticleSystem::msHeightCmd;
    ParticleSystem::CmdMaterial ParticleSystem::msMaterialCmd;
    ParticleSystem::CmdQuota ParticleSystem::msQuotaCmd;
    ParticleSystem::CmdWidth ParticleSystem::msWidthCmd;
    ParticleSystem::CmdBillboardType ParticleSystem::msBillboardTypeCmd;
    ParticleSystem::CmdCommonDirection ParticleSystem::msCommonDirectionCmd;

    //-----------------------------------------------------------------------
    ParticleSystem::ParticleSystem() :
        mAllDefaultSize( true ),
        mPoolSize(0),
        mRenderer(0),
        mCullIndividual(false)
    {
        initParameters();
    }
    //-----------------------------------------------------------------------
    ParticleSystem::ParticleSystem(const String& name):
        mAllDefaultSize( true ),
        mPoolSize(0),
        mRenderer(0),
        mCullIndividual(false)
    {
        mName = name;
        setDefaultDimensions( 100, 100 );
        setMaterialName( "BaseWhite" );
        // Default to 10 particles, expect app to specify (will only be increased, not decreased)
        setParticleQuota( 10 );
        initParameters();

    }
    //-----------------------------------------------------------------------
    ParticleSystem::~ParticleSystem()
    {
        // Arrange for the deletion of emitters & affectors
        removeAllEmitters();
        removeAllAffectors();
    }
    //-----------------------------------------------------------------------
    ParticleEmitter* ParticleSystem::addEmitter(const String& emitterType)
    {
        ParticleEmitter* em = ParticleSystemManager::getSingleton()._createEmitter(emitterType);
        mEmitters.push_back(em);
        return em;
    }
    //-----------------------------------------------------------------------
    ParticleEmitter* ParticleSystem::getEmitter(unsigned short index) const
    {
        assert(index < mEmitters.size() && "Emitter index out of bounds!");
        return mEmitters[index];
    }
    //-----------------------------------------------------------------------
    unsigned short ParticleSystem::getNumEmitters(void) const
    {
        return static_cast< unsigned short >( mEmitters.size() );
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeEmitter(unsigned short index)
    {
        assert(index < mEmitters.size() && "Emitter index out of bounds!");
        ParticleEmitterList::iterator ei = mEmitters.begin() + index;
        ParticleSystemManager::getSingleton()._destroyEmitter(*ei);
        mEmitters.erase(ei);
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeAllEmitters(void)
    {
        // DON'T delete directly, we don't know what heap these have been created on
        ParticleEmitterList::iterator ei;
        for (ei = mEmitters.begin(); ei != mEmitters.end(); ++ei)
        {
            ParticleSystemManager::getSingleton()._destroyEmitter(*ei);
        }
        mEmitters.clear();
    }
    //-----------------------------------------------------------------------
    ParticleAffector* ParticleSystem::addAffector(const String& affectorType)
    {
        ParticleAffector* af = ParticleSystemManager::getSingleton()._createAffector(affectorType);
        mAffectors.push_back(af);
        return af;
    }
    //-----------------------------------------------------------------------
    ParticleAffector* ParticleSystem::getAffector(unsigned short index) const
    {
        assert(index < mAffectors.size() && "Affector index out of bounds!");
        return mAffectors[index];
    }
    //-----------------------------------------------------------------------
    unsigned short ParticleSystem::getNumAffectors(void) const
    {
        return static_cast< unsigned short >( mAffectors.size() );
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeAffector(unsigned short index)
    {
        assert(index < mAffectors.size() && "Affector index out of bounds!");
        ParticleAffectorList::iterator ai = mAffectors.begin() + index;
        ParticleSystemManager::getSingleton()._destroyAffector(*ai);
        mAffectors.erase(ai);
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeAllAffectors(void)
    {
        // DON'T delete directly, we don't know what heap these have been created on
        ParticleAffectorList::iterator ai;
        for (ai = mAffectors.begin(); ai != mAffectors.end(); ++ai)
        {
            ParticleSystemManager::getSingleton()._destroyAffector(*ai);
        }
        mAffectors.clear();
    }
    //-----------------------------------------------------------------------
    ParticleSystem& ParticleSystem::operator=(const ParticleSystem& rhs)
    {
        // Blank this system's emitters & affectors
        removeAllEmitters();
        removeAllAffectors();

        // Copy emitters
        unsigned int i;
        for(i = 0; i < rhs.getNumEmitters(); ++i)
        {
            ParticleEmitter* rhsEm = rhs.getEmitter(i);
            ParticleEmitter* newEm = addEmitter(rhsEm->getType());
            rhsEm->copyParametersTo(newEm);
        }
        // Copy affectors
        for(i = 0; i < rhs.getNumAffectors(); ++i)
        {
            ParticleAffector* rhsAf = rhs.getAffector(i);
            ParticleAffector* newAf = addAffector(rhsAf->getType());
            rhsAf->copyParametersTo(newAf);
        }
        setParticleQuota(rhs.getParticleQuota());
        setMaterialName(rhs.mMaterialName);
        mDefaultHeight = rhs.mDefaultHeight;
        mDefaultWidth = rhs.mDefaultWidth;
        mCullIndividual = rhs.mCullIndividual;

        return *this;

    }
    //-----------------------------------------------------------------------
    size_t ParticleSystem::getNumParticles(void) const
    {
        return mActiveParticles.size();
    }
    //-----------------------------------------------------------------------
    size_t ParticleSystem::getParticleQuota(void) const
    {
        return mParticlePool.size();
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setParticleQuota(size_t size)
    {
        // Never shrink below size()
        size_t currSize = mParticlePool.size();

        if( currSize < size )
        {
            this->increasePool(size);

            for( size_t i = currSize; i < size; ++i )
            {
                // Add new items to the queue
                mFreeParticles.push_back( mParticlePool[i] );
            }

            mPoolSize = size;
            // Tell the renderer
            if (mRenderer)
            {
                mRenderer->_notifyParticleQuota(size);
            }
            
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_update(Real timeElapsed)
    {
		// Only update if attached to a node
		if (mParentNode)
		{
			// Update existing particles
        	_expire(timeElapsed);
        	_triggerAffectors(timeElapsed);
        	_applyMotion(timeElapsed);
			// Emit new particles
        	_triggerEmitters(timeElapsed);
			// Update bounds
            if (mRenderer)
        	    mRenderer->_updateBounds(mActiveParticles);
		}
		

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_expire(Real timeElapsed)
    {
        ActiveParticleList::iterator i, itEnd;
        Particle* pParticle;

        itEnd = mActiveParticles.end();

        for (i = mActiveParticles.begin(); i != itEnd; )
        {
            pParticle = static_cast<Particle*>(*i);
            if (pParticle->timeToLive < timeElapsed)
            {
                // Destroy this one
                mFreeParticles.push_back( *i );
                i = mActiveParticles.erase( i );
            }
            else
            {
                // Decrement TTL
                pParticle->timeToLive -= timeElapsed;
				++i;
            }

        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_triggerEmitters(Real timeElapsed)
    {
        // Add up requests for emission
        static std::vector<unsigned> requested;
        if( requested.size() != mEmitters.size() )
            requested.resize( mEmitters.size() );

        size_t totalRequested, emitterCount, i, emissionAllowed;
        ParticleEmitterList::iterator	itEmit, iEmitEnd;
        ParticleAffectorList::iterator	itAff, itAffEnd;
			    
        iEmitEnd = mEmitters.end();
        emitterCount = mEmitters.size();
        emissionAllowed = getParticleQuota() - mActiveParticles.size();
        totalRequested = 0;

        // Count up total requested emissions
        for (itEmit = mEmitters.begin(), i = 0; itEmit != iEmitEnd; ++itEmit, ++i)
        {
            requested[i] = (*itEmit)->_getEmissionCount(timeElapsed);
            totalRequested += requested[i];
        }


        // Check if the quota will be exceeded, if so reduce demand
        if (totalRequested > emissionAllowed)
        {
            // Apportion down requested values to allotted values
            Real ratio =  (Real)emissionAllowed / (Real)totalRequested;
            for (i = 0; i < emitterCount; ++i)
            {
                requested[i] *= (unsigned int)ratio;
            }
        }

        // Emit
		// For each emission, apply a subset of the motion for the frame
		// this ensures an even distribution of particles when many are
		// emitted in a single frame
        for (itEmit = mEmitters.begin(), i = 0; itEmit != iEmitEnd; ++itEmit, ++i)
        {
			Real timePoint = 0.0f;
			Real timeInc = timeElapsed / requested[i];
	        for (unsigned int j = 0; j < requested[i]; ++j)
            {
                // Create a new particle & init using emitter
                Particle* p = addParticle();
                (*itEmit)->_initParticle(p);

				// Translate position & direction into world space
                // Maybe make emitter do this?
                p->mPosition  = (mParentNode->_getDerivedOrientation() * p->mPosition) + mParentNode->_getDerivedPosition();
                p->mDirection = (mParentNode->_getDerivedOrientation() * p->mDirection);

				// apply partial frame motion to this particle
            	p->mPosition += (p->mDirection * timePoint);

				// apply particle initialization by the affectors
				itAffEnd = mAffectors.end();
				for (itAff = mAffectors.begin(); itAff != itAffEnd; ++itAff)
					(*itAff)->_initParticle(p);

				// Increment time fragment
				timePoint += timeInc;
            }
        }


    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_applyMotion(Real timeElapsed)
    {
        ActiveParticleList::iterator i, itEnd;
        Particle* pParticle;

        itEnd = mActiveParticles.end();
        for (i = mActiveParticles.begin(); i != itEnd; ++i)
        {
            pParticle = static_cast<Particle*>(*i);
            pParticle->mPosition += (pParticle->mDirection * timeElapsed);
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_triggerAffectors(Real timeElapsed)
    {
        ParticleAffectorList::iterator i, itEnd;
        
        itEnd = mAffectors.end();
        for (i = mAffectors.begin(); i != itEnd; ++i)
        {
            (*i)->_affectParticles(this, timeElapsed);
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::increasePool(unsigned int size)
    {
        size_t oldSize = mParticlePool.size();

        // Increase size
        mParticlePool.reserve(size);
        mParticlePool.resize(size);

        // Create new particles
        for( size_t i = oldSize; i < size; i++ )
            mParticlePool[i] = new Particle();

    }
    //-----------------------------------------------------------------------
    ParticleIterator ParticleSystem::_getIterator(void)
    {
        return ParticleIterator(mActiveParticles.begin(), mActiveParticles.end());
    }
    //-----------------------------------------------------------------------
    Particle* ParticleSystem::addParticle(void)
    {
        // Fast creation (don't use superclass since emitter will init)
        Particle* newBill = mFreeParticles.front();
        mFreeParticles.pop_front();
        mActiveParticles.push_back(newBill);

        newBill->_notifyOwner(this);

        // Because we're creating objects here we know this is a Particle
        return static_cast<Particle*>(newBill);

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_updateRenderQueue(RenderQueue* queue)
    {
        if (mRenderer)
        {
            mRenderer->_updateRenderQueue(queue, mActiveParticles, mCullIndividual);
        }
    }
    /*
    //-----------------------------------------------------------------------
    void ParticleSystem::genBillboardAxes(const Camera& cam, Vector3* pX, Vector3 *pY, const Billboard* pBill)    
    {
        // Orientation different from BillboardSet
        // Billboards are in world space (to decouple them from emitters in node space)
        Quaternion camQ;

        switch (mBillboardType)
        {
        case BBT_POINT:
            // Get camera world axes for X and Y (depth is irrelevant)
            // No inverse transform
            camQ = cam.getDerivedOrientation();
            *pX = camQ * Vector3::UNIT_X;
            *pY = camQ * Vector3::UNIT_Y;
           
            break;
        case BBT_ORIENTED_COMMON:
             // Y-axis is common direction
            // X-axis is cross with camera direction 
            *pY = mCommonDirection;
            *pX = cam.getDerivedDirection().crossProduct(*pY);
           
            break;
        case BBT_ORIENTED_SELF:
            // Y-axis is direction
            // X-axis is cross with camera direction 

            // Scale direction first
            *pY = (pBill->mDirection * 0.01);
            *pX = cam.getDerivedDirection().crossProduct(*pY);

            break;
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::getWorldTransforms(Matrix4* xform) const
    {
        // Particles are already in world space
        *xform = Matrix4::IDENTITY;

    }
    //-----------------------------------------------------------------------
    const Quaternion& ParticleSystem::getWorldOrientation(void) const
    {
        return mParentNode->_getDerivedOrientation();
    }
    //-----------------------------------------------------------------------
    const Vector3& ParticleSystem::getWorldPosition(void) const
    {
        return mParentNode->_getDerivedPosition();
    }
    //-----------------------------------------------------------------------
    */
    void ParticleSystem::initParameters(void)
    {
        if (createParamDictionary("ParticleSystem"))
        {
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("quota", 
                "The maximum number of particle allowed at once in this system.",
                PT_UNSIGNED_INT),
                &msQuotaCmd);

            dict->addParameter(ParameterDef("material", 
                "The name of the material to be used to render all particles in this system.",
                PT_STRING),
                &msMaterialCmd);

            dict->addParameter(ParameterDef("particle_width", 
                "The width of particles in world units.",
                PT_REAL),
                &msWidthCmd);

            dict->addParameter(ParameterDef("particle_height", 
                "The height of particles in world units.",
                PT_REAL),
                &msHeightCmd);

            dict->addParameter(ParameterDef("cull_each", 
                "If true, each particle is culled in it's own right. If false, the entire system is culled as a whole.",
                PT_BOOL),
                &msCullCmd);

            dict->addParameter(ParameterDef("billboard_type", 
                "The type of billboard to use. 'point' means a simulated spherical particle, " 
                "'oriented_common' means all particles in the set are oriented around common_direction, "
                "and 'oriented_self' means particles are oriented around their own direction.",
                PT_STRING),
                &msBillboardTypeCmd);

            dict->addParameter(ParameterDef("common_direction", 
                "Only useful when billboard_type is oriented_common. This parameter sets the common "
                "orientation for all particles in the set (e.g. raindrops may all be oriented downwards).",
                PT_VECTOR3),
                &msCommonDirectionCmd);

        }
    }
    /*
    //-----------------------------------------------------------------------
    void ParticleSystem::_updateBounds()
    {

        if (mParentNode)
        {
            // We've already put particles in world space to decouple them from the
            // node transform, so reverse transform back

            Vector3 min( Math::POS_INFINITY, Math::POS_INFINITY, Math::POS_INFINITY );
            Vector3 max( Math::NEG_INFINITY, Math::NEG_INFINITY, Math::NEG_INFINITY );
            Vector3 temp;
            const Vector3 *corner = mAABB.getAllCorners();
            Quaternion invQ = mParentNode->_getDerivedOrientation().Inverse();
            Vector3 t = mParentNode->_getDerivedPosition();

            for (int i = 0; i < 8; ++i)
            {
                // Reverse transform corner
                temp = invQ * (corner[i] - t);
                min.makeFloor(temp);
                max.makeCeil(temp);
            }
            mAABB.setExtents(min, max);
        }
    }
    */
    //-----------------------------------------------------------------------

    void ParticleSystem::fastForward(Real time, Real interval)
    {
        // First make sure all transforms are up to date

        for (Real ftime = 0; ftime < time; ftime += interval)
        {
            _update(interval);
        }
    }
    //-----------------------------------------------------------------------
    const String& ParticleSystem::getMovableType(void) const
    {
        static String mType = "ParticleSystem";
        return mType;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyParticleResized(void)
    {
        mAllDefaultSize = false;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyParticleRotated(void)
    {
        if (mRenderer)
        {
            mRenderer->_notifyParticleRotated();
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setDefaultDimensions( Real width, Real height )
    {
        mDefaultWidth = width;
        mDefaultHeight = height;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setDefaultWidth(Real width)
    {
        mDefaultWidth = width;
    }
    //-----------------------------------------------------------------------
    Real ParticleSystem::getDefaultWidth(void) const
    {
        return mDefaultWidth;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setDefaultHeight(Real height)
    {
        mDefaultHeight = height;
    }
    //-----------------------------------------------------------------------
    Real ParticleSystem::getDefaultHeight(void) const
    {
        return mDefaultHeight;
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& ParticleSystem::getBoundingBox(void) const
    {
        if (mRenderer)
        {
            return mRenderer->getBoundingBox();
        }
        else
        {
            static AxisAlignedBox aabb;
            return aabb;
        }
    }
    //-----------------------------------------------------------------------
    Real ParticleSystem::getBoundingRadius(void) const
    {
        if (mRenderer)
        {
            return mRenderer->getBoundingRadius();
        }
        else
        {
            return 0.0f;
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyCurrentCamera(Camera* cam)
    {
        if (mRenderer)
        {
            mRenderer->_notifyCurrentCamera(cam);
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setMaterialName(const String& name)
    {
        mMaterialName = name;
        if (mRenderer)
        {
            MaterialPtr mat = MaterialManager::getSingleton().getByName(name);
            mRenderer->_setMaterial(mat);
        }
    }
    //-----------------------------------------------------------------------
    const String& ParticleSystem::getMaterialName(void) const
    {
        return mMaterialName;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::clear()
    {
        // Insert actives into free list
        mFreeParticles.insert(mFreeParticles.end(), mActiveParticles.begin(), mActiveParticles.end());

        // Remove all active instances
        mActiveParticles.clear(); 

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setRenderer(ParticleSystemRenderer* renderer)
    {
        mRenderer = renderer;
        if (!mMaterialName.empty() && mRenderer)
        {
            MaterialPtr mat = MaterialManager::getSingleton().getByName(mMaterialName);
            mRenderer->_setMaterial(mat);
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setRenderer(const String& rendererName)
    {
        if (rendererName.empty())
        {
            mRenderer = 0;
        }
        else
        {
            setRenderer(
                ParticleSystemManager::getSingleton().getRenderer(rendererName));

        }
    }
    //-----------------------------------------------------------------------
    ParticleSystemRenderer* ParticleSystem::getRenderer(void) const
    {
        return mRenderer;
    }
    //-----------------------------------------------------------------------
    bool ParticleSystem::getCullIndividually(void) const
    {
        return mCullIndividual;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setCullIndividually(bool cullIndividual)
    {
        mCullIndividual = cullIndividual;
    }
    //-----------------------------------------------------------------------
    /*
    bool ParticleSystem::particleVisible(Camera* cam, ActiveParticleList::iterator p)
    {
        if (mCullIndividual) 
        {
            return mRenderer->particleVisible(p);
        }
        else
        {
            // Return always visible if not culling individually
            return true;
        }


        // Cull based on sphere (have to transform less)
        Sphere sph;
        Matrix4 xworld;

        getWorldTransforms(&xworld);

        sph.setCenter(xworld * (*bill)->mPosition);

        if ((*bill)->mOwnDimensions)
        {
            sph.setRadius(std::max((*bill)->mWidth, (*bill)->mHeight));
        }
        else
        {
            sph.setRadius(std::max(mDefaultWidth, mDefaultHeight));
        }

        return cam->isVisible(sph);

    }
    */
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdCull::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getCullIndividually() );
    }
    void ParticleSystem::CmdCull::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setCullIndividually(
            StringConverter::parseBool(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdHeight::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getDefaultHeight() );
    }
    void ParticleSystem::CmdHeight::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setDefaultHeight(
            StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdWidth::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getDefaultWidth() );
    }
    void ParticleSystem::CmdWidth::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setDefaultWidth(
            StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdMaterial::doGet(const void* target) const
    {
        return static_cast<const ParticleSystem*>(target)->getMaterialName();
    }
    void ParticleSystem::CmdMaterial::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdQuota::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getParticleQuota() );
    }
    void ParticleSystem::CmdQuota::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setParticleQuota(
            StringConverter::parseUnsignedInt(val));
    }
    /*
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdBillboardType::doGet(const void* target) const
    {
        BillboardType t = static_cast<const ParticleSystem*>(target)->getBillboardType();
        switch(t)
        {
        case BBT_POINT:
            return "point";
            break;
        case BBT_ORIENTED_COMMON:
            return "oriented_common";
            break;
        case BBT_ORIENTED_SELF:
            return "oriented_self";
            break;
        }
        // Compiler nicety
        return "";
    }
    void ParticleSystem::CmdBillboardType::doSet(void* target, const String& val)
    {
        BillboardType t;
        if (val == "point")
        {
            t = BBT_POINT;
        }
        else if (val == "oriented_common")
        {
            t = BBT_ORIENTED_COMMON;
        }
        else if (val == "oriented_self")
        {
            t = BBT_ORIENTED_SELF;
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, 
                "Invalid billboard_type '" + val + "'", 
                "ParticleSystem::CmdBillboardType::doSet");
        }

        static_cast<ParticleSystem*>(target)->setBillboardType(t);
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdCommonDirection::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getCommonDirection() );
    }
    void ParticleSystem::CmdCommonDirection::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setCommonDirection(
            StringConverter::parseVector3(val));
    }
    */

}
