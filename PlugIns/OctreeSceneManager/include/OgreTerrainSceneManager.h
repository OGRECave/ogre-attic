/***************************************************************************
                        terrainscenemanager.h  -  description
                           -------------------
  begin                : Mon Sep 23 2002
  copyright            : (C) 2002 by Jon Anderson
  email                : janders@users.sf.net
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TERRAINSCENEMANAGER_H
#define TERRAINSCENEMANAGER_H

#include <OgreOctreeSceneManager.h>
#include <OgreTerrainRenderable.h>


namespace Ogre
{

class Image;

typedef std::vector < TerrainRenderable * > TerrainRow;
typedef std::vector < TerrainRow > Terrain2D;


/** This is a basic SceneManager for organizing TerrainRenderables into a total landscape.
  * It loads a terrain from a .cfg file that specifices what textures/scale/mipmaps/etc to use.
  *@author Jon Anderson
  */

class TerrainSceneManager : public OctreeSceneManager
{
public:
    TerrainSceneManager( );
    virtual ~TerrainSceneManager( );

    /** Loads the terrain using parameters int he given config file. */

    void setWorldGeometry( const String& filename );

    /** Updates all the TerrainRenderables LOD. */
    virtual void _updateSceneGraph( Camera * cam );

    /** Aligns TerrainRenderable neighbors, and renders them. */
    virtual void _renderVisibleObjects( void );

    /** Returns the height at the given terrain coordinates. */
    float getHeightAt( float x, float y );


    bool intersectSegment( const Vector3 & start, const Vector3 & end, Vector3 * result );


    /** Returns the normal at the given terrain coordinates. */
    Vector3 & getNormalAt( float x, float y )
    {
        return mTempNormal;
    };

    /** Returns the TerrainRenderable that contains the given pt.
        If no tile exists at the point, it returns 0;
    */
    TerrainRenderable * getTerrainTile( const Vector3 & pt );


protected:

    bool _checkSize( int s )
    {
        for ( int i = 0; i < 16; i++ )
        {
            printf( "Checking...%d\n", ( 1 << i ) + 1 );

            if ( s == ( 1 << i ) + 1 )
                return true;
        }

        return false;

    }


    int mNumTiles;

    int mTileSize;

    Vector3 mTempNormal;

    Vector3 mScale;

    Material *mTerrainMaterial;

    SceneNode * mTerrainRoot;

    Terrain2D mTiles;

    TerrainOptions mOptions;

};

}

#endif
