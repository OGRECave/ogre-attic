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
/***************************************************************************
terrainscenemanager.h  -  description
---------------------
  begin                : Mon Sep 23 2002
  copyright            : (C) 2002 by Jon Anderson
  email                : janders@users.sf.net

  Enhancements 2003 - 2004 (C) The OGRE Team

***************************************************************************/

#ifndef TERRAINSCENEMANAGER_H
#define TERRAINSCENEMANAGER_H

#include "OgreOctreeSceneManager.h"
#include "OgreTerrainRenderable.h"


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

    /** Aligns TerrainRenderable neighbors, and renders them. */
    virtual void _renderVisibleObjects( void );

    /** Returns the height at the given terrain coordinates. */
    float getHeightAt( float x, float y );


    bool intersectSegment( const Vector3 & start, const Vector3 & end, Vector3 * result );

    /** Sets the name of the image to use as the heightmap input. 
    @remarks
        Heightmaps must be of size power of 2 + 1
    */
    void setHeightmap(const String& heightmapName);
    /** Sets the texture to use for the main world texture. */
    void setWorldTexture(const String& textureName);
    /** Sets the texture to use for the detail texture. */
    void setDetailTexture(const String& textureName);
    /** Sets the number of times per tile the detail texture should be repeated. */
    void setDetailTextureRepeat(int repeat);
    /** Sets the dimensions of each tile (must be power of 2 + 1) */
    void setTileSize(int size); 
    /** Sets the maximum screen space pixel error.  */
    void setMaxPixelError(int pixelError); 
    /** Sets how to scale the terrain data. */
    void setScale(const Vector3& scale);
    /** Sets the maximum geomipmap level to allow. */
    void setMaxGeoMipMapLevel(int maxMip);
    


    /** Sets whether the terrain should use triangle strips or not.
    @remarks
        The default is not, in which case it uses triangle lists. 
    */
    void setUseTriStrips(bool useStrips);
    /** Sets whether or not terrain tiles should be morphed between LODs
    (NB requires vertex program support). */
    void setUseLODMorph(bool useMorph);
    /** Sets the name of a custom material to use to shade the landcape.
    @remarks
        This method allows you to provide a custom material which will be
        used to render the landscape instead of the standard internal 
        material. This gives you a great deal of flexibility and allows you 
        to perform special effects if you wish. Note that because you determine
        every aspect of the material, this setting makes the use of setWorldTexture
        and setDetailTexture redundant.
    @par
        In your custom material, you can use all the usual features of Ogre's
        materials, including multiple passes if you wish. You can also use 
        the programmable pipeline to hook vertex and fragment programs into the
        terrain rendering. The plugin provides the following vertex components:
        <ul>
            <li>positions</li>
            <li>2 sets of texture coordinates (index 0 is world texture, 
            index 1 is detail texture)</li>
            <li>Normals, if enabled</li>
            <li>Per-vertex delta values, for morphing a higher LOD tile into 
                a lower LOD tile. This is one float per vertex bound as 'blend
                weight'. If you want to use this you also have to provide the
                name or index of the parameter you wish to receive the morph factor
                (@see setCustomMaterialMorphFactorParam)</li>
        </ul>
    */
    void setCustomMaterial(const String& materialName);
    /** Sets the name of the vertex program parameter to which to pass the
        LOD morph factor.
    @remarks
        When LOD morphing is enabled, and you are using a custom material to 
        shade the terrain, you need to inform this class of the parameter you
        wish the current LOD morph factor to be passed to. This is a simple
        float parameter value that the plugin will set from 0 to 1, depending on
        the morph stage of a tile. 0 represents no morphing, ie the vertices are
        all in the original position. 1 represents a complete morph such that
        the height of the vertices is the same as they are at the next lower LOD
        level. The vertex program must use this factor, in conjunction with the
        per-vertex height delta values (bound as 'blend weight'), to displace
        vertices.
    @note This version of the method lets you specify a parameter name, compatible
        with high-level vertex programs. There is an alternative signature method
        which allows you to set the parameter index for low-level assembler programs.
    @param paramName The name of the parameter which will receive the morph factor
    */
    void setCustomMaterialMorphFactorParam(const String& paramName);
    /** Sets the index of the vertex program parameter to which to pass the
        LOD morph factor.
    @remarks
        When LOD morphing is enabled, and you are using a custom material to 
        shade the terrain, you need to inform this class of the parameter you
        wish the current LOD morph factor to be passed to. This is a simple
        float parameter value that the plugin will set from 0 to 1, depending on
        the morph stage of a tile. 0 represents no morphing, ie the vertices are
        all in the original position. 1 represents a complete morph such that
        the height of the vertices is the same as they are at the next lower LOD
        level. The vertex program must use this factor, in conjunction with the
        per-vertex height delta values (bound as 'blend weight'), to displace
        vertices.
    @note This version of the method lets you specify a parameter index, compatible
        with low-level assembler vertex programs. There is an alternative signature method
        which allows you to set the parameter name for high-level programs.
    @param paramName The name of the parameter which will receive the morph factor
    */
    void setCustomMaterialMorphFactorParam(size_t paramIndex);

    /** Returns the TerrainRenderable that contains the given pt.
        If no tile exists at the point, it returns 0;
    */
    TerrainRenderable * getTerrainTile( const Vector3 & pt );


protected:

    /// Validates that the size picked for the terrain is acceptable
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

    /// The number of terrain tiles making up the terrain
    int mNumTiles;
    /// The size (in vertices) of one edge of a terrain tile
    int mTileSize;
    /// The scale factor of the terrain
    Vector3 mScale;
    /// Pointer to the material to use to render the terrain
    Material *mTerrainMaterial;
    /// The node to which all terrain tiles are attached
    SceneNode * mTerrainRoot;
    /// Array of terrain tiles
    Terrain2D mTiles;
    /// Terrain size, detail etc
    TerrainOptions mOptions;
    /// Should we use an externally-defined custom material?
    bool mUseCustomMaterial;
    /// The name of the custom material to use
    String mCustomMaterialName;
    /// The name of the source heightmap
    String mHeightmapName;
    /// The name of the world texture
    String mWorldTextureName;
    /// The name of the detail texture
    String mDetailTextureName;
    /// Are we using a named parameter to hook up LOD morph?
    bool mUseNamedParameterLodMorph;
    /// The name of the parameter to send the LOD morph to
    String mLodMorphParamName;
    /// The index of the parameter to send the LOD morph to
    size_t mLodMorphParamIndex;
    
    /// Internal method for loading configurations settings
    void loadConfig(const String& filename);

    /// Loads configured heightmap data into the position buffer
    void loadHeightmap(void);
    /// Sets up the terrain material
    void setupTerrainMaterial(void);
    /// Creates a bunch of terrain tiles and populates them
    void setupTerrainTiles(void);

};

}

#endif
