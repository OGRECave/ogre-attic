/***************************************************************************
octreecamera.cpp  -  description
-------------------
begin                : Fri Sep 27 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Lesser General Public License as        *
*   published by the Free Software Foundation; either version 2 of the    * 
*   License, or (at your option) any later version.                       *
*                                                                         *
***************************************************************************/

#include <OgreMath.h>
#include <OgreAxisAlignedBox.h>
#include <OgreRoot.h>

#include <OgreOctreeCamera.h>

namespace Ogre
{
#define POSITION_BINDING 0
#define COLOUR_BINDING 1

unsigned long red = 0xFF0000FF;

unsigned short OctreeCamera::mIndexes[ 24 ] = {0, 1, 1, 2, 2, 3, 3, 0,       //back
        0, 6, 6, 5, 5, 1,             //left
        3, 7, 7, 4, 4, 2,             //right
        6, 7, 5, 4 };          //front

unsigned long OctreeCamera::mColors[ 8 ] = {red, red, red, red, red, red, red, red};

OctreeCamera::OctreeCamera( const String& name, SceneManager* sm ) : Camera( name, sm )
{
    mMaterial = sm->getMaterial("BaseWhite");

    mVertexData = new VertexData;
    mVertexData->vertexStart = 0;
    mVertexData->vertexCount = 8;

    mIndexData = new IndexData;
    mIndexData->indexStart = 0;
    mIndexData->indexCount = 24;

    VertexDeclaration* decl = mVertexData->vertexDeclaration;
    VertexBufferBinding* bind = mVertexData->vertexBufferBinding;

    decl->addElement(POSITION_BINDING, 0, VET_FLOAT3, VES_POSITION);
    decl->addElement(COLOUR_BINDING, 0, VET_COLOUR, VES_DIFFUSE);

    HardwareVertexBufferSharedPtr vbuf =
        HardwareBufferManager::getSingleton().createVertexBuffer(
            decl->getVertexSize(POSITION_BINDING),
            mVertexData->vertexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    bind->setBinding(POSITION_BINDING, vbuf);

    mIndexData->indexBuffer = 
        HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT,
            24, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
                                                                      
}

OctreeCamera::~OctreeCamera()
{
    if(mVertexData)
        delete mVertexData;
    if(mIndexData)
        delete mIndexData;
}

OctreeCamera::Visibility OctreeCamera::getVisibility( const AxisAlignedBox &bound )
{

    // Null boxes always invisible
    if ( bound.isNull() )
        return NONE;

    // Make any pending updates to the calculated frustum
    updateView();

    // Get corners of the box
    const Vector3* pCorners = bound.getAllCorners();

    // For each plane, see if all points are on the negative side
    // If so, object is not visible.
    // If one or more are, it's partial.
    // If all aren't, full

    int corners[ 8 ] = {0, 4, 3, 5, 2, 6, 1, 7};

    int planes[ 6 ] = {FRUSTUM_PLANE_TOP, FRUSTUM_PLANE_BOTTOM,
                       FRUSTUM_PLANE_LEFT, FRUSTUM_PLANE_RIGHT,
                       FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_NEAR };

    bool all_inside = true;

    for ( int plane = 0; plane < 6; ++plane )
    {

        bool all_outside = true;

        float distance = 0;

        for ( int corner = 0; corner < 8; ++corner )
        {
            distance = mFrustumPlanes[ planes[ plane ] ].getDistance( pCorners[ corners[ corner ] ] );
            all_outside = all_outside && ( distance < 0 );
            all_inside = all_inside && ( distance >= 0 );

            if ( !all_outside && !all_inside )
                break;
        }

        if ( all_outside )
            return NONE;
    }

    if ( all_inside )
        return FULL;
    else
        return PARTIAL;

}

void OctreeCamera::getRenderOperation( RenderOperation& op )
{
    std::cerr << "OctreeCamera::getRenderOperation\n";

    Real * r = mCorners;
    //could also project pts using inverse of 4x4 Projection matrix, but no inverse function on that.
    /*
    Matrix4 invP =getProjectionMatrix().Inverse();

    Vector3 f1(-1,-1,-1); f1 = f1*invP;
    Vector3 f2(-1, 1,-1); f2 = f2*invP;
    Vector3 f3( 1, 1,-1); f3 = f3*invP;
    Vector3 f4( 1,-1,-1); f4 = f4*invP;

    Vector3 b1(-1,-1,1); b1 = b1*invP;
    Vector3 b2(-1, 1,1); b2 = b2*invP;
    Vector3 b3( 1, 1,1); b3 = b3*invP;
    Vector3 b4( 1,-1,1); b4 = b4*invP;
    */
    _getCorner( r, FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_LEFT, FRUSTUM_PLANE_BOTTOM ); r += 3;
    _getCorner( r, FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_LEFT, FRUSTUM_PLANE_TOP ); r += 3;
    _getCorner( r, FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_RIGHT, FRUSTUM_PLANE_TOP ); r += 3;
    _getCorner( r, FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_RIGHT, FRUSTUM_PLANE_BOTTOM ); r += 3;

    _getCorner( r, FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_RIGHT, FRUSTUM_PLANE_TOP ); r += 3;
    _getCorner( r, FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_LEFT, FRUSTUM_PLANE_TOP ); r += 3;
    _getCorner( r, FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_LEFT, FRUSTUM_PLANE_BOTTOM ); r += 3;
    _getCorner( r, FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_RIGHT, FRUSTUM_PLANE_BOTTOM );

    updateView();

    HardwareVertexBufferSharedPtr vbuf = 
        mVertexData->vertexBufferBinding->getBuffer(POSITION_BINDING);

    vbuf->writeData(0, 8 * sizeof(Real), mCorners);

    vbuf = mVertexData->vertexBufferBinding->getBuffer(COLOUR_BINDING);
    vbuf->writeData(0, 8 * sizeof(RGBA), mColors);

    mIndexData->indexBuffer->writeData(0, 24 * sizeof(unsigned short), mIndexes);

    op.useIndexes = true;
    op.operationType = RenderOperation::OT_LINE_LIST;
    op.vertexData = mVertexData;
    op.indexData = mIndexData;

    /* 
    rend.useIndexes = true;
    rend.numTextureCoordSets = 0; // no textures
    rend.vertexOptions = LegacyRenderOperation::VO_DIFFUSE_COLOURS;
    rend.numVertices = 8;
    rend.numIndexes = 24;

    rend.pVertices = mCorners;
    rend.pIndexes = mIndexes;
    rend.pDiffuseColour = mColors;
    */

}

void OctreeCamera::_getCorner( Real *r, FrustumPlane pp1, FrustumPlane pp2, FrustumPlane pp3 )
{
    //intersect the three planes to get a point.
    //this could be faster since we know what Z is, but showing the camera is only a debug tool...


    Plane p1 = mFrustumPlanes[ pp1 ];
    Plane p2 = mFrustumPlanes[ pp2 ];
    Plane p3 = mFrustumPlanes[ pp3 ];

    Matrix3 mdet ( p1.normal.x , p1.normal.y , p1.normal.z ,
                   p2.normal.x , p2.normal.y , p2.normal.z ,
                   p3.normal.x , p3.normal.y , p3.normal.z );

    float det = mdet.Determinant ();

    if ( det == 0 ) return ; //some planes are parallel.

    Matrix3 mx ( -p1.d , p1.normal.y , p1.normal.z,
                 -p2.d, p2.normal.y , p2.normal.z,
                 -p3.d, p3.normal.y , p3.normal.z );

    float xdet = mx.Determinant();

    Matrix3 my ( p1.normal.x, -p1.d, p1.normal.z,
                 p2.normal.x, -p2.d, p2.normal.z,
                 p3.normal.x, -p3.d, p3.normal.z );

    float ydet = my.Determinant ();

    Matrix3 mz ( p1.normal.x, p1.normal.y, -p1.d,
                 p2.normal.x, p2.normal.y, -p2.d,
                 p3.normal.x, p3.normal.y, -p3.d );

    float zdet = mz.Determinant ();

    r[ 0 ] = xdet / det;

    r[ 1 ] = ydet / det;

    r[ 2 ] = zdet / det;

}

Real OctreeCamera::getSquaredViewDepth(const Camera* cam) const
{
	Vector3 dist = cam->getDerivedPosition() - this->getDerivedPosition();
	return dist.squaredLength();
}
Material* OctreeCamera::getMaterial(void) const
{
    return mMaterial;
}

void OctreeCamera::getWorldTransforms(Matrix4* xform)
{
    xform[0] = Matrix4::IDENTITY;
}

}




