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
#ifndef __SimpleRenderable_H__
#define __SimpleRenderable_H__

#include "OgrePrerequisites.h"

#include "OgreMovableObject.h"
#include "OgreRenderable.h"
#include "OgreAxisAlignedBox.h"
#include "OgreMaterial.h"

namespace Ogre {

    class _OgreExport SimpleRenderable : public MovableObject, public Renderable
    {
    protected:
        RenderOperation mRO;
        Matrix4 mWorldTransform;
        AxisAlignedBox mAAB;

        String m_strMatName;
        Material *m_pMaterial;

        Real *m_pVertexCache;
        unsigned short *m_pIndexCache;
        Real *m_pNormalCache;
        RGBA *m_pDiffuseCache;
        RGBA *m_pSpecularCache;
        Real *m_pTexCache[OGRE_MAX_TEXTURE_COORD_SETS];

        SceneManager *m_pParentSceneManager;

        Camera *m_pCamera;

        /// Name, for conformance with MovableObject
        String mName;

        /// Counter for auto-generating names for backwards compatibility
        static int msGenNameCount;

        /// Shared class-level name for Movable type
        static String msMovableType;

    public:
        SimpleRenderable()
        {
            mWorldTransform = Matrix4::IDENTITY;

            m_strMatName = ""; m_pMaterial = NULL;

            m_pVertexCache = 0;
            m_pIndexCache = 0;
            m_pNormalCache = 0;
            m_pDiffuseCache = m_pSpecularCache = NULL;        

            for( int i=0; i<OGRE_MAX_TEXTURE_COORD_SETS; i++ )
                m_pTexCache[i] = NULL;

            m_pParentSceneManager = NULL;

            mParentNode = NULL;
            m_pCamera = NULL;

            // Generate name
            char tempName[32];
            sprintf(tempName, "SimpleRenderable%d", msGenNameCount++);
            mName = tempName;



        }

        // These are to quickly set the RO's properties
        RenderOperation& getRO();

        const RenderOperation& getRO() const;

        Real **getVertexCache();
        unsigned short **getIndexCache();
        Real **getNormalCache();
        RGBA **getDiffuseCache();
        RGBA **getSpecularCache();
        Real **getTexCoordCcache( unsigned short cn );

        void setMaterial( const String& matName );
        virtual Material* getMaterial(void) const;

        virtual void setRenderOperation( const RenderOperation& rend );
        virtual void getRenderOperation( RenderOperation& rend );

        void setWorldTransform( const Matrix4& xform );
        virtual void getWorldTransforms( Matrix4* xform );

        virtual SceneNode* getParentNode(void);

        virtual void _notifyAttached( SceneNode* parent );
        virtual void _notifyCurrentCamera(Camera* cam);

        AxisAlignedBox& getABB();
        virtual const AxisAlignedBox& getBoundingBox(void) const;

        virtual void _updateRenderQueue(RenderQueue* queue);

        virtual ~SimpleRenderable();

        /** Overridden from MovableObject */
        String getName(void);

        /** Overridden from MovableObject */
        String getMovableType(void);


    };
}

#endif

