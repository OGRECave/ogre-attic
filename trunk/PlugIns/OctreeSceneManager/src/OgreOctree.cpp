/***************************************************************************
octree.cpp  -  description
-------------------
begin                : Mon Sep 30 2002
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

#include <OgreOctree.h>
#include <OgreOctreeNode.h>

namespace Ogre
{

/** Returns true is the box will fit in a child.
*/
bool Octree::_isTwiceSize( AxisAlignedBox &box )
{
    const Vector3 * pts1 = mBox.getAllCorners();
    const Vector3 * pts2 = box.getAllCorners();

    return ( ( pts2[ 4 ].x -pts2[ 0 ].x ) <= ( pts1[ 4 ].x - pts1[ 0 ].x ) / 2 ) &&
           ( ( pts2[ 4 ].y - pts2[ 0 ].y ) <= ( pts1[ 4 ].y - pts1[ 0 ].y ) / 2 ) &&
           ( ( pts2[ 4 ].z - pts2[ 0 ].z ) <= ( pts1[ 4 ].z - pts1[ 0 ].z ) / 2 ) ;

}

/** It's assumed the the given box has already been proven to fit into
* a child.  Since it's a loose octree, only the centers need to be
* compared to find the appropriate node.
*/
void Octree::_getChildIndexes( AxisAlignedBox &box, int *x, int *y, int *z )
{
    Vector3 max = mBox.getMaximum();
    Vector3 min = box.getMinimum();

    Vector3 center = mBox.getMaximum().midPoint( mBox.getMinimum() );

    Vector3 ncenter = box.getMaximum().midPoint( box.getMinimum() );

    if ( ncenter.x > center.x )
        * x = 1;
    else
        *x = 0;

    if ( ncenter.y > center.y )
        * y = 1;
    else
        *y = 0;

    if ( ncenter.z > center.z )
        * z = 1;
    else
        *z = 0;

}

Octree::Octree( Octree * parent ) : mHalfSize( 0, 0, 0 )
{
    //initialize all children to null.
    for ( int i = 0; i < 2; i++ )
    {
        for ( int j = 0; j < 2; j++ )
        {
            for ( int k = 0; k < 2; k++ )
            {
                mChildren[ i ][ j ][ k ] = 0;
            }
        }
    }

    mParent = parent;
    mNumNodes = 0;
}

Octree::~Octree()
{
    //initialize all children to null.
    for ( int i = 0; i < 2; i++ )
    {
        for ( int j = 0; j < 2; j++ )
        {
            for ( int k = 0; k < 2; k++ )
            {
                if ( mChildren[ i ][ j ][ k ] != 0 )
                    delete mChildren[ i ][ j ][ k ];
            }
        }
    }

    mParent = 0;
}

void Octree::_addNode( OctreeNode * n )
{
    mNodes.push_back( n );
    n -> setOctant( this );

    //update total counts.
    _ref();

}

void Octree::_removeNode( OctreeNode * n )
{
    mNodes.erase( std::find( mNodes.begin(), mNodes.end(), n ) );
    n -> setOctant( 0 );

    //update total counts.
    _unref();
}

void Octree::_getCullBounds( AxisAlignedBox *b )
{
    const Vector3 * corners = mBox.getAllCorners();
    b -> setExtents( corners[ 0 ] - mHalfSize, corners[ 4 ] + mHalfSize );
}

}
