#include "OgreSimpleRenderable.h"

#include "OgreMaterialManager.h"

namespace Ogre {

    int SimpleRenderable::msGenNameCount = 0;
    String SimpleRenderable::msMovableType = "SimpleRenderable";

    // These are to quickly set the RO's properties
    RenderOperation& SimpleRenderable::getRO()
    {
        return mRO;
    }

    const RenderOperation& SimpleRenderable::getRO() const
    {
        return mRO;
    }

    Real **SimpleRenderable::getVertexCache()
    {
        return &m_pVertexCache;
    }

    unsigned short **SimpleRenderable::getIndexCache()
    {
        return &m_pIndexCache;
    }

    Real **SimpleRenderable::getNormalCache()
    {
        return &m_pNormalCache;
    }

    RGBA **SimpleRenderable::getDiffuseCache()
    {
        return &m_pDiffuseCache;
    }

    RGBA **SimpleRenderable::getSpecularCache()
    {
        return &m_pSpecularCache;
    }

    Real **SimpleRenderable::getTexCoordCcache( unsigned short cn )
    {
        assert( cn < OGRE_MAX_TEXTURE_COORD_SETS );

        return &m_pTexCache[cn];
    }

    void SimpleRenderable::setMaterial( const String& matName )
    {
        m_strMatName = matName;
        m_pMaterial = reinterpret_cast<Material*>(
            MaterialManager::getSingleton().getByName( m_strMatName ) );
    
        // Won't load twice anyway
        m_pMaterial->load();
    }

    Material* SimpleRenderable::getMaterial(void) const
    {
        return m_pMaterial;
    }

    void SimpleRenderable::setRenderOperation( const RenderOperation& rend )
    {
        mRO = rend;
    }

    void SimpleRenderable::getRenderOperation( RenderOperation& rend )
    {
        rend = mRO;
    }

    void SimpleRenderable::setWorldTransform( const Matrix4& xform )
    {
        mWorldTransform = xform;
    }

    void SimpleRenderable::getWorldTransforms( Matrix4* xform )
    {
        *xform = mParentNode->_getFullTransform();
    }

    SceneNode* SimpleRenderable::getParentNode(void)
    {
        return mParentNode;
    }

    void SimpleRenderable::_notifyAttached( SceneNode* parent )
    {
        mParentNode = parent;
    }

    void SimpleRenderable::_notifyCurrentCamera(Camera* cam)
    {
        m_pCamera = NULL;
    }

    AxisAlignedBox& SimpleRenderable::getABB()
    {
        return mAAB;
    }

    const AxisAlignedBox& SimpleRenderable::getBoundingBox(void) const
    {
        return mAAB;
    }

    void SimpleRenderable::_updateRenderQueue(RenderQueue* queue)
    {
        queue->addRenderable( this );
    }

    SimpleRenderable::~SimpleRenderable()
    {
        if( m_pVertexCache )
            delete[] m_pVertexCache;

        if( m_pIndexCache )
            delete[] m_pIndexCache;

        if( m_pNormalCache )
            delete[] m_pNormalCache;

        for( int i=0; i<OGRE_MAX_TEXTURE_COORD_SETS; i++ )
            if( m_pTexCache[i] )
                delete[] m_pTexCache[i];

        if( m_pDiffuseCache )
            delete[] m_pDiffuseCache;

        if( m_pSpecularCache )
            delete[] m_pSpecularCache;
    }
    //-----------------------------------------------------------------------
    String SimpleRenderable::getName(void)
    {
        return mName;
    }
    //-----------------------------------------------------------------------
    String SimpleRenderable::getMovableType(void)
    {
        return msMovableType;
    }

}
