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
#include "OgreSimpleRenderable.h"

#include "OgreMaterialManager.h"

namespace Ogre {

    uint SimpleRenderable::ms_uGenNameCount = 0;

    SimpleRenderable::SimpleRenderable()
    {
        m_matWorldTransform = Matrix4::IDENTITY;

        m_strMatName = "BaseWhite"; 
        m_pMaterial = reinterpret_cast< Material* >( MaterialManager::getSingleton().getByName( "BaseWhite" ) );

        m_pVertexCache = NULL;
        m_pIndexCache = NULL;
        m_pNormalCache = NULL;
        m_pDiffuseCache = NULL;
        m_pSpecularCache = NULL;

        for( int i=0; i<OGRE_MAX_TEXTURE_COORD_SETS; i++ )
            m_pTexCache[i] = NULL;

        m_pParentSceneManager = NULL;

        mParentNode = NULL;
        m_pCamera = NULL;

        // Generate name
        m_strName << _T("SimpleRenderable") << ms_uGenNameCount ++;
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

    Real **SimpleRenderable::getTexCoordCache( unsigned short cn )
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
        mRendOp = rend;
    }

    RenderOperation& SimpleRenderable::getRenderOperation()
    {
        return mRendOp;
    }

    void SimpleRenderable::getRenderOperation( RenderOperation& rend )
    {
        rend = mRendOp;
    }

    void SimpleRenderable::setWorldTransform( const Matrix4& xform )
    {
        m_matWorldTransform = xform;
    }

    void SimpleRenderable::getWorldTransforms( Matrix4* xform )
    {
        *xform = m_matWorldTransform * mParentNode->_getFullTransform();
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
        m_pCamera = cam;
    }

    void SimpleRenderable::setBoundingBox( const AxisAlignedBox& box )
    {
        mBox = box;
    }

    const AxisAlignedBox& SimpleRenderable::getBoundingBox(void) const
    {
        return mBox;
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
    const String& SimpleRenderable::getName(void) const
    {
        return m_strName;
    }
    //-----------------------------------------------------------------------
    const String SimpleRenderable::getMovableType(void) const
    {
        return "SimpleRenderable";
    }

}
