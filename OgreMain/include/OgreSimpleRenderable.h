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
            char tempName[128];
            sprintf(tempName, "SimpleRenderable%1", msGenNameCount++);
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
