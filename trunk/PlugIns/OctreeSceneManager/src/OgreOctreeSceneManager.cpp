/***************************************************************************
octreescenemanager.cpp  -  description
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

#include <OgreOctreeSceneManager.h>
#include <OgreOctreeNode.h>
#include <OgreOctreeCamera.h>

#include <OgreRenderSystem.h>

namespace Ogre
{

    int OctreeSceneManager::intersect_call = 0;

    enum Intersection
    {
        OUTSIDE,
        INSIDE,
        INTERSECT
    };

    /** Checks how the second box intersects with the first.
    */
    Intersection intersect( const AxisAlignedBox &one, const AxisAlignedBox &two )
    {
        OctreeSceneManager::intersect_call++;
        const Vector3 * outside = one.getAllCorners();
        const Vector3 *inside = two.getAllCorners();

        if ( inside[ 4 ].x < outside[ 0 ].x ||
            inside[ 4 ].y < outside[ 0 ].y ||
            inside[ 4 ].z < outside[ 0 ].z ||
            inside[ 0 ].x > outside[ 4 ].x ||
            inside[ 0 ].y > outside[ 4 ].y ||
            inside[ 0 ].z > outside[ 4 ].z )
        {
            return OUTSIDE;
        }

        bool full = ( inside[ 0 ].x > outside[ 0 ].x &&
            inside[ 0 ].y > outside[ 0 ].y &&
            inside[ 0 ].z > outside[ 0 ].z &&
            inside[ 4 ].x < outside[ 4 ].x &&
            inside[ 4 ].x < outside[ 4 ].x &&
            inside[ 4 ].x < outside[ 4 ].x );

        if ( full )
            return INSIDE;
        else
            return INTERSECT;

    }

    /** Checks how the box intersects with the sphere.
    */
    Intersection intersect( const Sphere &one, const AxisAlignedBox &two )
    {
        OctreeSceneManager::intersect_call++;
        float sradius = one.getRadius();

        sradius *= sradius;

        Vector3 scenter = one.getCenter();

        const Vector3 *corners = two.getAllCorners();

        float s, d = 0;

        Vector3 mndistance = ( corners[ 0 ] - scenter );
        Vector3 mxdistance = ( corners[ 4 ] - scenter );

        if ( mndistance.squaredLength() < sradius &&
            mxdistance.squaredLength() < sradius )
        {
            return INSIDE;
        }

        //find the square of the distance
        //from the sphere to the box
        for ( int i = 0 ; i < 3 ; i++ )
        {
            if ( scenter[ i ] < corners[ 0 ][ i ] )
            {
                s = scenter[ i ] - corners[ 0 ][ i ];
                d += s * s;
            }

            else if ( scenter[ i ] > corners[ 4 ][ i ] )
            {
                s = scenter[ i ] - corners[ 4 ][ i ];
                d += s * s;
            }

        }

        bool partial = ( d <= sradius );

        if ( !partial )
        {
            return OUTSIDE;
        }

        else
        {
            return INTERSECT;
        }


    }


    unsigned long white = 0xFFFFFFFF;

    unsigned short OctreeSceneManager::mIndexes[ 24 ] = {0, 1, 1, 2, 2, 3, 3, 0,    //back
        0, 6, 6, 5, 5, 1,          //left
        3, 7, 7, 4, 4, 2,          //right
        6, 7, 5, 4 };          //front
    unsigned long OctreeSceneManager::mColors[ 8 ] = {white, white, white, white, white, white, white, white };


    OctreeSceneManager::OctreeSceneManager( ) : SceneManager()
    {
        AxisAlignedBox b( -500, -500, -500, 500, 500, 500 );
        int depth = 5; //infinite depth
        init( b, depth );
    }

    OctreeSceneManager::OctreeSceneManager( AxisAlignedBox &box, int max_depth ) : SceneManager()
    {

        init( box, max_depth );
    }

    void OctreeSceneManager::init( AxisAlignedBox &box, int depth )
    {
        delete mSceneRoot; //get rid of old root.

        // -- Changes by Steve
        // Don't do it this way, it will add it to the mSceneNodes which we don't want
        //mSceneRoot = createSceneNode( "SceneRoot" );
        mSceneRoot = new OctreeNode( this, "SceneRoot" );
        // -- End changes by Steve

        if ( mOctree != 0 )
            delete mOctree;

        mOctree = new Octree( 0 );

        mMaxDepth = depth;

        mOctree -> mBox = box;

        Vector3 min = box.getMinimum();

        Vector3 max = box.getMaximum();

        mOctree -> mHalfSize = ( max - min ) / 2;


        mRenderOp.useIndexes = true;

        mRenderOp.numTextureCoordSets = 0; // no textures

        mRenderOp.vertexOptions = RenderOperation::VO_DIFFUSE_COLOURS;

        mRenderOp.operationType = RenderOperation::OT_LINE_LIST;

        mRenderOp.numVertices = 8;

        mRenderOp.numIndexes = 24;

        mRenderOp.pVertices = mCorners;

        mRenderOp.pIndexes = mIndexes;

        mRenderOp.pDiffuseColour = mColors;

        mShowBoxes = false;

        mCullCamera = false;

        mNumObjects = 0;

        Vector3 v( 1.5, 1.5, 1.5 );

        mScaleFactor.setScale( v );

        // setDisplaySceneNodes( true );
        // setShowBoxes( true );

        //
        //setUseCullCamera( true );
        //mSceneRoot isn't put into the octree since it has no volume.

    }

    OctreeSceneManager::~OctreeSceneManager()
    {
        // -- Changed by Steve
        // Don't do this here, SceneManager will do it
        /*
        if( mSceneRoot )
            delete mSceneRoot;
        */
        // --End Changes by Steve

        if( mOctree )
            delete mOctree;
    }

    Camera * OctreeSceneManager::createCamera( const String &name )
    {
        Camera * c = new OctreeCamera( name, this );
        mCameras.insert( CameraList::value_type( name, c ) );
        return c;
    }


    void OctreeSceneManager::_updateOctreeNode( OctreeNode * onode )
    {
        AxisAlignedBox box = onode -> _getWorldAABB();

        if ( box.isNull() )
            return ;


        if ( onode -> getOctant() == 0 )
        {
            _addOctreeNode( onode, mOctree );
            return ;
        }

        if ( ! onode -> _isIn( onode -> getOctant() -> mBox ) )
        {
            _removeOctreeNode( onode );
            _addOctreeNode( onode, mOctree );
        }
    }

    /** Only removes the node from the octree.  It leaves the octree, even if it's empty.
    */
    void OctreeSceneManager::_removeOctreeNode( OctreeNode * n )
    {
        n -> getOctant() -> _removeNode( n );
    }


    void OctreeSceneManager::_addOctreeNode( OctreeNode * n, Octree *octant, int depth )
    {

        AxisAlignedBox bx = n -> _getWorldAABB();


        //if the octree is twice as big as the scene node,
        //we will add it to a child.
        if ( ( depth < mMaxDepth ) && octant -> _isTwiceSize( bx ) )
        {
            int x, y, z;
            octant -> _getChildIndexes( bx, &x, &y, &z );

            if ( octant -> mChildren[ x ][ y ][ z ] == 0 )
            {
                octant -> mChildren[ x ][ y ][ z ] = new Octree( octant );

                const Vector3 *corners = octant -> mBox.getAllCorners();
                Vector3 min, max;

                if ( x == 0 )
                {
                    min.x = corners[ 0 ].x;
                    max.x = ( corners[ 0 ].x + corners[ 4 ].x ) / 2;
                }

                else
                {
                    min.x = ( corners[ 0 ].x + corners[ 4 ].x ) / 2;
                    max.x = corners[ 4 ].x;
                }

                if ( y == 0 )
                {
                    min.y = corners[ 0 ].y;
                    max.y = ( corners[ 0 ].y + corners[ 4 ].y ) / 2;
                }

                else
                {
                    min.y = ( corners[ 0 ].y + corners[ 4 ].y ) / 2;
                    max.y = corners[ 4 ].y;
                }

                if ( z == 0 )
                {
                    min.z = corners[ 0 ].z;
                    max.z = ( corners[ 0 ].z + corners[ 4 ].z ) / 2;
                }

                else
                {
                    min.z = ( corners[ 0 ].z + corners[ 4 ].z ) / 2;
                    max.z = corners[ 4 ].z;
                }

                octant -> mChildren[ x ][ y ][ z ] -> mBox.setExtents( min, max );
                octant -> mChildren[ x ][ y ][ z ] -> mHalfSize = ( max - min ) / 2;
            }

            _addOctreeNode( n, octant -> mChildren[ x ][ y ][ z ], depth++ );

        }

        else
        {
            octant -> _addNode( n );
        }
    }


    SceneNode * OctreeSceneManager::createSceneNode( void )
    {
        OctreeNode * on = new OctreeNode( this );
        mSceneNodes[ on->getName() ] = on;
        return on;
    }

    SceneNode * OctreeSceneManager::createSceneNode( const String &name )
    {
        OctreeNode * on = new OctreeNode( this, name );
        mSceneNodes[ on->getName() ] = on;
        return on;
    }

    void OctreeSceneManager::_updateSceneGraph( Camera * cam )
    {
        SceneManager::_updateSceneGraph( cam );
    }

    void OctreeSceneManager::_renderVisibleObjects( void )
    {

        if ( mShowBoxes )
        {
            mDestRenderSystem->_setWorldMatrix( Matrix4::IDENTITY );

            BoxList::iterator it = mBoxes.begin();

            while ( it != mBoxes.end() )
            {
                getBoxVerts( *( *it ), mRenderOp.pVertices );
                mDestRenderSystem->_render( mRenderOp );
                ++it;
            }
        }

        if ( mCullCamera )
        {
            Camera * c = getCamera( "CullCamera" );

            if ( c != 0 )
            {
                RenderOperation r;
                static_cast < OctreeCamera* > ( c ) -> getRenderOperation( r );
                mDestRenderSystem -> _render( r );
            }
        }

        SceneManager::_renderVisibleObjects( );



    }

    void OctreeSceneManager::getBoxVerts( AxisAlignedBox &box, Real *r )
    {
        const Vector3 * corners = box.getAllCorners();

        for ( int i = 0; i < 8; i++ )
        {
            *r = corners[ i ].x; r++;
            *r = corners[ i ].y; r++;
            *r = corners[ i ].z; r++;
        }
    }

    void OctreeSceneManager::_findVisibleObjects( Camera * cam )
    {

        mRenderQueue.clear();
        mBoxes.clear();

        if ( mCullCamera )
        {
            Camera * c = getCamera( "CullCamera" );

            if ( c != 0 )
                cam = getCamera( "CullCamera" );
        }

        mNumObjects = 0;

        //walk the octree, adding all visible Octreenodes nodes to the render queue.
        walkOctree( static_cast < OctreeCamera * > ( cam ), &mRenderQueue, mOctree, false );

    }

    void OctreeSceneManager::walkOctree( OctreeCamera *camera, RenderQueue *queue, Octree *octant, bool foundvisible )
    {

        //return immediately if nothing is in the node.
        if ( octant -> numNodes() == 0 )
            return ;

        OctreeCamera::Visibility v = OctreeCamera::NONE;

        if ( foundvisible )
        {
            v = OctreeCamera::FULL;
        }

        else
        {
            AxisAlignedBox box;
            octant -> _getCullBounds( &box );
            v = camera -> getVisibility( box );
        }


        if ( v != OctreeCamera::NONE )
        {

            //Add stuff to be rendered;
            NodeList::iterator it = octant -> mNodes.begin();

            if ( mShowBoxes )
                mBoxes.push_back( &( octant -> mBox ) );

            bool vis = true;

            while ( it != octant -> mNodes.end() )
            {
                OctreeNode * sn = *it;

                // if this octree is partially visible, manually cull all
                // scene nodes attached directly to this level.

                if ( v == OctreeCamera::PARTIAL )
                    vis = camera -> isVisible( sn -> _getWorldAABB() );

                if ( vis )
                {

                    mNumObjects++;
                    sn -> _addToRenderQueue( queue );

                    if ( mDisplayNodes )
                        queue -> addRenderable( sn );

                }

                ++it;
            }

            if ( octant -> mChildren[ 0 ][ 0 ][ 0 ] != 0 ) walkOctree( camera, queue, octant -> mChildren[ 0 ][ 0 ][ 0 ], ( v == OctreeCamera::FULL ) );

            if ( octant -> mChildren[ 1 ][ 0 ][ 0 ] != 0 ) walkOctree( camera, queue, octant -> mChildren[ 1 ][ 0 ][ 0 ], ( v == OctreeCamera::FULL ) );

            if ( octant -> mChildren[ 0 ][ 1 ][ 0 ] != 0 ) walkOctree( camera, queue, octant -> mChildren[ 0 ][ 1 ][ 0 ], ( v == OctreeCamera::FULL ) );

            if ( octant -> mChildren[ 1 ][ 1 ][ 0 ] != 0 ) walkOctree( camera, queue, octant -> mChildren[ 1 ][ 1 ][ 0 ], ( v == OctreeCamera::FULL ) );

            if ( octant -> mChildren[ 0 ][ 0 ][ 1 ] != 0 ) walkOctree( camera, queue, octant -> mChildren[ 0 ][ 0 ][ 1 ], ( v == OctreeCamera::FULL ) );

            if ( octant -> mChildren[ 1 ][ 0 ][ 1 ] != 0 ) walkOctree( camera, queue, octant -> mChildren[ 1 ][ 0 ][ 1 ], ( v == OctreeCamera::FULL ) );

            if ( octant -> mChildren[ 0 ][ 1 ][ 1 ] != 0 ) walkOctree( camera, queue, octant -> mChildren[ 0 ][ 1 ][ 1 ], ( v == OctreeCamera::FULL ) );

            if ( octant -> mChildren[ 1 ][ 1 ][ 1 ] != 0 ) walkOctree( camera, queue, octant -> mChildren[ 1 ][ 1 ][ 1 ], ( v == OctreeCamera::FULL ) );

        }

    }

    void OctreeSceneManager::findNodesIn( const AxisAlignedBox &box, std::list < SceneNode * > &list, SceneNode *exclude, bool full, Octree *octant )
    {
        if ( octant == 0 )
        {
            octant = mOctree;
        }

        if ( !full )
        {
            AxisAlignedBox obox;
            octant -> _getCullBounds( &obox );

            Intersection isect = intersect( box, obox );

            if ( isect == OUTSIDE )
                return ;

            full = ( isect == INSIDE );
        }


        NodeList::iterator it = octant -> mNodes.begin();

        while ( it != octant -> mNodes.end() )
        {
            OctreeNode * on = ( *it );

            if ( on != exclude )
            {
                if ( full )
                {
                    list.push_back( on );
                }

                else
                {
                    Intersection nsect = intersect( box, on -> _getWorldAABB() );

                    if ( nsect != OUTSIDE )
                    {
                        list.push_back( on );
                    }
                }

            }

            ++it;
        }



        if ( octant -> mChildren[ 0 ][ 0 ][ 0 ] != 0 ) findNodesIn( box, list, exclude, full, octant -> mChildren[ 0 ][ 0 ][ 0 ] );

        if ( octant -> mChildren[ 1 ][ 0 ][ 0 ] != 0 ) findNodesIn( box, list, exclude, full, octant -> mChildren[ 1 ][ 0 ][ 0 ] );

        if ( octant -> mChildren[ 0 ][ 1 ][ 0 ] != 0 ) findNodesIn( box, list, exclude, full, octant -> mChildren[ 0 ][ 1 ][ 0 ] );

        if ( octant -> mChildren[ 1 ][ 1 ][ 0 ] != 0 ) findNodesIn( box, list, exclude, full, octant -> mChildren[ 1 ][ 1 ][ 0 ] );

        if ( octant -> mChildren[ 0 ][ 0 ][ 1 ] != 0 ) findNodesIn( box, list, exclude, full, octant -> mChildren[ 0 ][ 0 ][ 1 ] );

        if ( octant -> mChildren[ 1 ][ 0 ][ 1 ] != 0 ) findNodesIn( box, list, exclude, full, octant -> mChildren[ 1 ][ 0 ][ 1 ] );

        if ( octant -> mChildren[ 0 ][ 1 ][ 1 ] != 0 ) findNodesIn( box, list, exclude, full, octant -> mChildren[ 0 ][ 1 ][ 1 ] );

        if ( octant -> mChildren[ 1 ][ 1 ][ 1 ] != 0 ) findNodesIn( box, list, exclude, full, octant -> mChildren[ 1 ][ 1 ][ 1 ] );

    }

    void OctreeSceneManager::findNodesIn( const Sphere &sphere, std::list < SceneNode * > &list, SceneNode *exclude, bool full, Octree *octant )
    {
        if ( octant == 0 )
        {
            octant = mOctree;
        }

        if ( !full )
        {
            AxisAlignedBox obox;
            octant -> _getCullBounds( &obox );

            Intersection isect = intersect( sphere, obox );

            if ( isect == OUTSIDE )
                return ;

            full = ( isect == INSIDE );
        }

        NodeList::iterator it = octant -> mNodes.begin();

        while ( it != octant -> mNodes.end() )
        {
            OctreeNode * on = ( *it );

            if ( on != exclude )
            {
                if ( full )
                {
                    list.push_back( on );
                }

                else
                {
                    Intersection nsect = intersect( sphere, on -> _getWorldAABB() );

                    if ( nsect != OUTSIDE )
                    {
                        list.push_back( on );
                    }
                }
            }

            ++it;
        }



        if ( octant -> mChildren[ 0 ][ 0 ][ 0 ] != 0 ) findNodesIn( sphere, list, exclude, full, octant -> mChildren[ 0 ][ 0 ][ 0 ] );

        if ( octant -> mChildren[ 1 ][ 0 ][ 0 ] != 0 ) findNodesIn( sphere, list, exclude, full, octant -> mChildren[ 1 ][ 0 ][ 0 ] );

        if ( octant -> mChildren[ 0 ][ 1 ][ 0 ] != 0 ) findNodesIn( sphere, list, exclude, full, octant -> mChildren[ 0 ][ 1 ][ 0 ] );

        if ( octant -> mChildren[ 1 ][ 1 ][ 0 ] != 0 ) findNodesIn( sphere, list, exclude, full, octant -> mChildren[ 1 ][ 1 ][ 0 ] );

        if ( octant -> mChildren[ 0 ][ 0 ][ 1 ] != 0 ) findNodesIn( sphere, list, exclude, full, octant -> mChildren[ 0 ][ 0 ][ 1 ] );

        if ( octant -> mChildren[ 1 ][ 0 ][ 1 ] != 0 ) findNodesIn( sphere, list, exclude, full, octant -> mChildren[ 1 ][ 0 ][ 1 ] );

        if ( octant -> mChildren[ 0 ][ 1 ][ 1 ] != 0 ) findNodesIn( sphere, list, exclude, full, octant -> mChildren[ 0 ][ 1 ][ 1 ] );

        if ( octant -> mChildren[ 1 ][ 1 ][ 1 ] != 0 ) findNodesIn( sphere, list, exclude, full, octant -> mChildren[ 1 ][ 1 ][ 1 ] );

    }

}

