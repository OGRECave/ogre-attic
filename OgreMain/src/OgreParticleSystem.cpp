/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

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
    ParticleSystem::ParticleSystem()
    {
        initParameters();
    }
    //-----------------------------------------------------------------------
    ParticleSystem::ParticleSystem(const String& name)
    {
        // DO NOT use superclass constructor
        // This will call setPoolSize in the BillboardSet context and create Billboard objects
        //  instead of Particle objects
        // Unavoidable due to C++ funky virtualisation rules & constructors
        mpPositions = 0;
        mpColours = 0;
        mpIndexes = 0;
        mpTexCoords = 0;
        mAutoExtendPool = true;
        mAllDefaultSize = true;
        mOriginType = BBO_CENTER;
        mName = name;
        mCullIndividual = true;
        setDefaultDimensions( 100, 100 );
        setMaterialName( "BaseWhite" );
        // Default to 10 particles, expect app to specify (will only be increased, not decreased)
        setPoolSize( 10 );

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
    String ParticleSystem::getName(void)
    {
        return mName;
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
        return mEmitters.size();
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
        return mAffectors.size();
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
        setPoolSize(rhs.getPoolSize());
        setMaterialName(rhs.mMaterialName);
        mOriginType = rhs.mOriginType;
        mDefaultHeight = rhs.mDefaultHeight;
        mDefaultWidth = rhs.mDefaultWidth;
        mCullIndividual = rhs.mCullIndividual;
        mBillboardType = rhs.mBillboardType;
        mCommonDirection = rhs.mCommonDirection;


        return *this;

    }
    //-----------------------------------------------------------------------
    unsigned int ParticleSystem::getParticleQuota(void) const
    {
        // This is basically a renamed property
        return getPoolSize();
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setParticleQuota(unsigned int quota)
    {
        // This is basically a renamed property
        setPoolSize(quota);
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_update(Real timeElapsed)
    {
        _expire(timeElapsed);
        _triggerEmitters(timeElapsed);
        _applyMotion(timeElapsed);
        _triggerAffectors(timeElapsed);
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_expire(Real timeElapsed)
    {
        ActiveBillboardList::iterator i, itEnd;
        Particle* pParticle;

        itEnd = mActiveBillboards.end();

        for (i = mActiveBillboards.begin(); i != itEnd; ++i)
        {
            pParticle = static_cast<Particle*>(*i);
            if (pParticle->mTimeToLive < timeElapsed)
            {
                // Destroy this one
                mFreeBillboards.push_back( *i );
                i = mActiveBillboards.erase( i );
            }
            else
            {
                // Decrement TTL
                pParticle->mTimeToLive -= timeElapsed;
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

        unsigned int totalRequested, emitterCount, i, emissionAllowed;
        ParticleEmitterList::iterator it, iEmitEnd;
        
        iEmitEnd = mEmitters.end();
        emitterCount = mEmitters.size();
        emissionAllowed = getParticleQuota() - mActiveBillboards.size();
        totalRequested = 0;

        // Count up total requested emissions
        for (it = mEmitters.begin(), i = 0; it != iEmitEnd; ++it, ++i)
        {
            requested[i] = (*it)->_getEmissionCount(timeElapsed);
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
        for (it = mEmitters.begin(), i = 0; it != iEmitEnd; ++it, ++i)
        {
            for (unsigned int j = 0; j < requested[i]; ++j)
            {
                // Create a new particle & init using emitter
                Particle* p = addParticle();
                (*it)->_initParticle(p);
                // Translate position & direction into world space
                // Maybe make emitter do this?
                p->mPosition =  (mParentNode->_getDerivedOrientation() * p->mPosition) + mParentNode->_getDerivedPosition();
                p->mDirection = mParentNode->_getDerivedOrientation() * p->mDirection;

            }
        }


    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_applyMotion(Real timeElapsed)
    {
        ActiveBillboardList::iterator i, itEnd;
        Particle* pParticle;

        itEnd = mActiveBillboards.end();
        for (i = mActiveBillboards.begin(); i != itEnd; ++i)
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
        unsigned int oldSize = mBillboardPool.size();

        // Increase size
        mBillboardPool.reserve(size);
        mBillboardPool.resize(size);

        // Create new particles
        for (unsigned int i = oldSize; i < size; ++i)
            mBillboardPool[i] = new Particle();

    }
    //-----------------------------------------------------------------------
    ParticleIterator ParticleSystem::_getIterator(void)
    {
        return ParticleIterator(mActiveBillboards.begin(), mActiveBillboards.end());
    }
    //-----------------------------------------------------------------------
    Particle* ParticleSystem::addParticle(void)
    {
        // Fast creation (don't use superclass since emitter will init)
        Billboard* newBill = mFreeBillboards.front();
        mFreeBillboards.pop_front();
        mActiveBillboards.push_back(newBill);

        newBill->_notifyOwner(this);

        // Because we're creating objects here we know this is a Particle
        return static_cast<Particle*>(newBill);

    }
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
            camQ = cam.getOrientation();
            *pX = camQ * Vector3::UNIT_X;
            *pY = camQ * Vector3::UNIT_Y;
           
            break;
        case BBT_ORIENTED_COMMON:
             // Y-axis is common direction
            // X-axis is cross with camera direction 
            *pY = mCommonDirection;
            *pX = camQ * cam.getDirection().crossProduct(*pY);
           
            break;
        case BBT_ORIENTED_SELF:
            // Y-axis is direction
            // X-axis is cross with camera direction 

            // Scale direction first
            *pY = (pBill->mDirection * 0.01);
            *pX = camQ * cam.getDirection().crossProduct(*pY);

            break;
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::getWorldTransform(Matrix4& xform)
    {
        // Particles are already in world space
        xform = Matrix4::IDENTITY;

    }
    //-----------------------------------------------------------------------
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
                PT_UNSIGNED_INT),
                &msBillboardTypeCmd);

            dict->addParameter(ParameterDef("common_direction", 
                "Only useful when billboard_type is oriented_common. This parameter sets the common "
                "orientation for all particles in the set (e.g. raindrops may all be oriented downwards).",
                PT_VECTOR3),
                &msCommonDirectionCmd);

        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyCurrentCamera(Camera *cam)
    {
        // Call superclass
        BillboardSet::_notifyCurrentCamera(cam);
        // Have to override because bounds are supposed to be in local node space
        // but we've already put particles in world space to decouple them from the
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
    String ParticleSystem::CmdCull::doGet(void* target)
    {
        return StringConverter::toString(
            static_cast<ParticleSystem*>(target)->getCullIndividually() );
    }
    void ParticleSystem::CmdCull::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setCullIndividually(
            StringConverter::parseBool(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdHeight::doGet(void* target)
    {
        return StringConverter::toString(
            static_cast<ParticleSystem*>(target)->getDefaultHeight() );
    }
    void ParticleSystem::CmdHeight::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setDefaultHeight(
            StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdWidth::doGet(void* target)
    {
        return StringConverter::toString(
            static_cast<ParticleSystem*>(target)->getDefaultWidth() );
    }
    void ParticleSystem::CmdWidth::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setDefaultWidth(
            StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdMaterial::doGet(void* target)
    {
        return static_cast<ParticleSystem*>(target)->getMaterialName();
    }
    void ParticleSystem::CmdMaterial::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdQuota::doGet(void* target)
    {
        return StringConverter::toString(
            static_cast<ParticleSystem*>(target)->getParticleQuota() );
    }
    void ParticleSystem::CmdQuota::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setParticleQuota(
            StringConverter::parseUnsignedInt(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdBillboardType::doGet(void* target)
    {
        BillboardType t = static_cast<ParticleSystem*>(target)->getBillboardType();
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
        static_cast<ParticleSystem*>(target)->setBillboardType(t);
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdCommonDirection::doGet(void* target)
    {
        return StringConverter::toString(
            static_cast<ParticleSystem*>(target)->getCommonDirection() );
    }
    void ParticleSystem::CmdCommonDirection::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setCommonDirection(
            StringConverter::parseVector3(val));
    }

}
