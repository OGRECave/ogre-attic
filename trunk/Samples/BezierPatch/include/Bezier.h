/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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

/**
    \file 
        Bezier.h
    \brief
        Specialisation of OGRE's framework application to show off
        the bezier patch support.
*/

#include "ExampleApplication.h"
#include "OgreStringConverter.h"

// Hack struct for test
PatchSurface ps;

// Event handler to add ability to alter subdivision
class BezierListener : public ExampleFrameListener
{
protected:
public:
    BezierListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
        
    }

    bool frameStarted(const FrameEvent& evt)
    {
        static Real timeLapse = 99.0f;
        static int level = 0;
        static bool wireframe = 0;


        timeLapse += evt.timeSinceLastFrame;

        if (timeLapse > 3.0f)
        {
            level = (level + 1) % 5;
            ps.setSubdivisionLevel(level);
            ps.build();
            mWindow->setDebugText("Bezier subdivisions: " + StringConverter::toString(level));
            timeLapse = 0.0f;
            if (level == 0)
            {
                wireframe = !wireframe;
                mCamera->setDetailLevel(wireframe ? SDL_WIREFRAME : SDL_SOLID);
            }

        }

        // Call default
        return ExampleFrameListener::frameStarted(evt);
    }
};

       
class BezierApplication : public ExampleApplication
{
protected:

    GeometryData patchCtlPoints;
public:
    BezierApplication() { patchCtlPoints.pVertices = 0; }
    ~BezierApplication()
    {
        if (patchCtlPoints.pVertices)
            delete [] patchCtlPoints.pVertices;
    }

protected:

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
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-0.5, -0.5, 0);

        // Create patch
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
        pVert->x = 0.0; pVert->y = 500.0; pVert->z = -750.0;
        pVert->nx = 0.0; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.5; pVert->v = 0.0;
        pVert++;
        pVert->x = 500.0; pVert->y = 1000.0; pVert->z = -500.0;
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
        pVert->x = 500.0; pVert->y = 200.0; pVert->z = 800.0;
        pVert->nx = 0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 1.0; pVert->v = 1.0;
        pVert++;


        ps.defineSurface("Bezier1", patchCtlPoints, 3, PatchSurface::PST_BEZIER, 0, PatchSurface::VS_BOTH);
        ps.build();

        // Create entity based on patch
        Entity* ent = mSceneMgr->createEntity("Entity1", "Bezier1");

        Material *pMat = (Material*)MaterialManager::getSingleton().create( "TextMat" );
        pMat->addTextureLayer( "BumpyMetal.jpg" );
        ent->setMaterialName("TextMat");

        // Attach the entity to the root of the scene
        mSceneMgr->getRootSceneNode()->attachObject(ent);

        mCamera->setPosition(500,500, 1500);
        mCamera->lookAt(0,200,-300);

    }
	void createFrameListener(void)
    {
		// This is where we instantiate our own frame listener
        mFrameListener= new BezierListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);

    }

};
