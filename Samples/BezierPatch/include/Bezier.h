/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
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

/**
    \file 
        Bezier.h
    \brief
        Specialisation of OGRE's framework application to show off
        the bezier patch support.
*/

#include "ExampleApplication.h"

class BezierApplication : public ExampleApplication
{
public:
    BezierApplication() {}

protected:

    // Hack struct for test
    PatchSurface ps;
    GeometryData patchCtlPoints;

#if OGRE_COMPILER == COMPILER_MSVC
    #pragma pack(push)
    #pragma pack(1)
#endif
    struct PatchVertex {
        Real x, y, z;
        Real nx, ny, nz;
        Real u, v;
    };
#if OGRE_COMPILER == COMPILER_MSVC
    #pragma pack(pop)
#endif

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(200,80,50);

        // Create patch
        // NB really sloppy with memory here - it's just a test
        patchCtlPoints.hasColours = false;
        patchCtlPoints.hasNormals = true;
        patchCtlPoints.numTexCoords = 1;
        patchCtlPoints.numTexCoordDimensions[0] = 2;
        // Packed data
        patchCtlPoints.vertexStride = sizeof(Real) * 5;
        patchCtlPoints.normalStride = sizeof(Real) * 5;
        patchCtlPoints.texCoordStride[0] = sizeof(Real) * 6;
        // Make a 3x3 patch for test
        patchCtlPoints.pVertices = (Real*)( new PatchVertex[9] );
        patchCtlPoints.numVertices = 9;
        patchCtlPoints.pNormals = patchCtlPoints.pVertices + 3;
        patchCtlPoints.pTexCoords[0] = patchCtlPoints.pVertices + 6;

        // Patch data
        PatchVertex *pVert = (PatchVertex*)patchCtlPoints.pVertices;

        pVert->x = -500.0; pVert->y = 200.0; pVert->z = -500.0;
        pVert->nx = -0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.0; pVert->v = 0.0;
        pVert++;
        pVert->x = 0.0; pVert->y = 500.0; pVert->z = -500.0;
        pVert->nx = 0.0; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.5; pVert->v = 0.0;
        pVert++;
        pVert->x = 500.0; pVert->y = 0.0; pVert->z = -500.0;
        pVert->nx = 0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 1.0; pVert->v = 0.0;
        pVert++;

        pVert->x = -500.0; pVert->y = 0.0; pVert->z = 0.0;
        pVert->nx = -0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.0; pVert->v = 0.5;
        pVert++;
        pVert->x = 0.0; pVert->y = 500.0; pVert->z = 0.0;
        pVert->nx = 0.0; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.5; pVert->v = 0.5;
        pVert++;
        pVert->x = 500.0; pVert->y = -50.0; pVert->z = 0.0;
        pVert->nx = 0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 1.0; pVert->v = 0.5;
        pVert++;

        pVert->x = -500.0; pVert->y = 0.0; pVert->z = 500.0;
        pVert->nx = -0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.0; pVert->v = 1.0;
        pVert++;
        pVert->x = 0.0; pVert->y = 500.0; pVert->z = 500.0;
        pVert->nx = 0.0; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.5; pVert->v = 1.0;
        pVert++;
        pVert->x = 500.0; pVert->y = 200.0; pVert->z = 500.0;
        pVert->nx = 0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 1.0; pVert->v = 1.0;
        pVert++;


        ps.defineSurface("Bezier1", patchCtlPoints, 3, PatchSurface::PST_BEZIER, 4, PatchSurface::VS_BOTH);
        ps.build();

        TextureFont *pFont;

        /*
        // This is hard-coded and should really get changed
        try {        
		    pFont = new TextureFont( "../../../Media/starjedi.ttf" );
        } catch( Exception e ) {
            try {
                pFont = new TextureFont( "c:\\WINDOWS\\fonts\\trebucbd.ttf" );
            } catch( Exception e ) {
                pFont = new TextureFont( "c:\\WINNT\\fonts\\trebucbd.ttf" );
            }
        }

		pFont->createAlphaMask( 
            "TextTexture", 
            "Yay! I finally got a screenshot showing text rendering with Ogre & FreeType 2 :)\n"
            "This should be the second line :)\n"
            "\n"
            "\n"
            "This is a Bezier patch that uses a material which holds a texture created with the\n"
            "function TextureFont::createAlphaMask\n"
            "The font used is Trebuchet MS\n"
            "The material has a SBT_TRANSPARENT_ALPHA scene blending and has depth\n"
            "writing set to off!"
            "\n"
            "\n"
            "Look up ;)", 
            512, 512, 
            0xff, 0xcc, 0x00, 710 );
		
        Material *pMat = (Material*)MaterialManager::getSingleton().create( "TextMat" );
        pMat->addTextureLayer( "TextTexture" );
        pMat->setSceneBlending( SBT_TRANSPARENT_ALPHA );
        pMat->setDepthWriteEnabled( false );
        */

        // Create entity based on patch
        Entity* ent = mSceneMgr->createEntity("Entity1", "Bezier1");

        Material *pMat = (Material*)MaterialManager::getSingleton().create( "TextMat" );
        pMat->addTextureLayer( "BumpyMetal.jpg" );
        ent->setMaterialName("TextMat");

        // Attach the entity to the root of the scene
        mSceneMgr->getRootSceneNode()->attachObject(ent);

    }

};
