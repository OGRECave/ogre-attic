/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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
#ifndef __BillboardParticleRenderer_H__
#define __BillboardParticleRenderer_H__

#include "OgrePrerequisites.h"
#include "OgreParticleSystemRenderer.h"

namespace Ogre {

    /** Specialisation of ParticleSystemRenderer to render particles using 
        a BillboardSet. 
    @remarks
        This renderer has a few more options than the standard particle system,
        which will be passed to it automatically when the particle system itself
        does not understand them.
    */
    class _OgreExport BillboardParticleRenderer : public ParticleSystemRenderer
    {
    protected:
        /// The billboard set that's doing the rendering
        BillboardSet* mBillboardSet;
    public:
        BillboardParticleRenderer();
        ~BillboardParticleRenderer();

        /** Command object for billboard type (see ParamCommand).*/
        class CmdBillboardType : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for common direction (see ParamCommand).*/
        class CmdCommonDirection : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /// @copydoc ParticleSystemRenderer::getType
        const String& getType(void) const;
        /// @copydoc ParticleSystemRenderer::_updateRenderQueue
        void _updateRenderQueue(RenderQueue* queue, 
            std::list<Particle*>& currentParticles, bool cullIndividually);
        /// @copydoc ParticleSystemRenderer::_setMaterial
        void _setMaterial(MaterialPtr& mat);
        /// @copydoc ParticleSystemRenderer::_notifyCurrentCamera
        void _notifyCurrentCamera(Camera* cam);
        /// @copydoc ParticleSystemRenderer::_notifyParticleRotated
        void _notifyParticleRotated(void);
        /// @copydoc ParticleSystemRenderer::_notifyParticleResized
        void _notifyParticleResized(void);
        /// @copydoc ParticleSystemRenderer::_notifyParticleQuota
        void _notifyParticleQuota(size_t quota);

    protected:
        static CmdBillboardType msBillboardTypeCmd;
        static CmdCommonDirection msCommonDirectionCmd;


    };

    /** Factory class for BillboardParticleRenderer */
    class _OgreExport BillboardParticleRendererFactory : public ParticleSystemRendererFactory
    {
    public:
        /// @copydoc FactoryObj::getType
        const String& getType() const;
        /// @copydoc FactoryObj::createInstance
        ParticleSystemRenderer* createInstance( const String& name );    
        /// @copydoc FactoryObj::destroyInstance
        void destroyInstance( ParticleSystemRenderer* inst);    
    };

}

#endif

