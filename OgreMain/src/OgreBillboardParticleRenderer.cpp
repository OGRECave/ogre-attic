/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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

#include "OgreBillboardParticleRenderer.h"
#include "OgreParticle.h"
#include "OgreStringConverter.h"

namespace Ogre {
    String rendererTypeName = "billboard";

    //-----------------------------------------------------------------------
    BillboardParticleRenderer::CmdBillboardType BillboardParticleRenderer::msBillboardTypeCmd;
    BillboardParticleRenderer::CmdCommonDirection BillboardParticleRenderer::msCommonDirectionCmd;
    //-----------------------------------------------------------------------
    BillboardParticleRenderer::BillboardParticleRenderer()
    {
        if (createParamDictionary("BillboardParticleRenderer"))
        {
            ParamDictionary* dict = getParamDictionary();
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

        // Create billboard set
        mBillboardSet = new BillboardSet("", 0, true);
        // World-relative axes
        mBillboardSet->setBillboardsInWorldSpace(true);
    }
    //-----------------------------------------------------------------------
    BillboardParticleRenderer::~BillboardParticleRenderer()
    {
        delete mBillboardSet;
    }
    //-----------------------------------------------------------------------
    const String& BillboardParticleRenderer::getType(void) const
    {
        return rendererTypeName;
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_updateRenderQueue(RenderQueue* queue, 
        std::list<Particle*>& currentParticles, bool cullIndividually)
    {
        mBillboardSet->setCullIndividually(cullIndividually);

        // Update billboard set geometry
        mBillboardSet->beginBillboards();
        Billboard bb;
        for (std::list<Particle*>::iterator i = currentParticles.begin();
            i != currentParticles.end(); ++i)
        {
            Particle* p = *i;
            bb.mPosition = p->position;
            bb.mDirection = p->direction;
            bb.mColour = p->colour;
            bb.mRotation = p->rotation;
            // Assign and compare at the same time
            if (bb.mOwnDimensions = p->mOwnDimensions)
            {
                bb.mWidth = p->mWidth;
                bb.mHeight = p->mHeight;
            }
            mBillboardSet->injectBillboard(bb);

        }
        
        mBillboardSet->endBillboards();

        // Update the queue
        mBillboardSet->_updateRenderQueue(queue);
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_setMaterial(MaterialPtr& mat)
    {
        mBillboardSet->setMaterialName(mat->getName());
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::setBillboardType(BillboardType bbt)
    {
        mBillboardSet->setBillboardType(bbt);
    }
    //-----------------------------------------------------------------------
    BillboardType BillboardParticleRenderer::getBillboardType(void) const
    {
        return mBillboardSet->getBillboardType();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::setCommonDirection(const Vector3& vec)
    {
        mBillboardSet->setCommonDirection(vec);
    }
    //-----------------------------------------------------------------------
    const Vector3& BillboardParticleRenderer::getCommonDirection(void) const
    {
        return mBillboardSet->getCommonDirection();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyCurrentCamera(Camera* cam)
    {
        mBillboardSet->_notifyCurrentCamera(cam);
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyParticleRotated(void)
    {
        mBillboardSet->_notifyBillboardTextureCoordsModified();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyDefaultDimensions(Real width, Real height)
    {
        mBillboardSet->setDefaultDimensions(width, height);
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyParticleResized(void)
    {
        mBillboardSet->_notifyBillboardResized();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyParticleQuota(size_t quota)
    {
        mBillboardSet->setPoolSize(quota);
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyAttached(Node* parent, bool isTagPoint)
    {
        mBillboardSet->_notifyAttached(parent, isTagPoint);
    }
	//-----------------------------------------------------------------------
	void BillboardParticleRenderer::setRenderQueueGroup(RenderQueueGroupID queueID)
	{
		mBillboardSet->setRenderQueueGroup(queueID);
	}
	//-----------------------------------------------------------------------
	void BillboardParticleRenderer::setKeepParticlesInLocalSpace(bool keepLocal)
	{
		mBillboardSet->setBillboardsInWorldSpace(!keepLocal);
	}
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    const String& BillboardParticleRendererFactory::getType() const
    {
        return rendererTypeName;
    }
    //-----------------------------------------------------------------------
    ParticleSystemRenderer* BillboardParticleRendererFactory::createInstance( 
        const String& name )
    {
        return new BillboardParticleRenderer();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRendererFactory::destroyInstance( 
        ParticleSystemRenderer* inst)
    {
        delete inst;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String BillboardParticleRenderer::CmdBillboardType::doGet(const void* target) const
    {
        BillboardType t = static_cast<const BillboardParticleRenderer*>(target)->getBillboardType();
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
    void BillboardParticleRenderer::CmdBillboardType::doSet(void* target, const String& val)
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
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Invalid billboard_type '" + val + "'", 
                "ParticleSystem::CmdBillboardType::doSet");
        }

        static_cast<BillboardParticleRenderer*>(target)->setBillboardType(t);
    }
    //-----------------------------------------------------------------------
    String BillboardParticleRenderer::CmdCommonDirection::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const BillboardParticleRenderer*>(target)->getCommonDirection() );
    }
    void BillboardParticleRenderer::CmdCommonDirection::doSet(void* target, const String& val)
    {
        static_cast<BillboardParticleRenderer*>(target)->setCommonDirection(
            StringConverter::parseVector3(val));
    }

}

