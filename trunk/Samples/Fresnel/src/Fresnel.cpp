/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
/*
-----------------------------------------------------------------------------
Filename:    Fresnel.cpp
Description: Fresnel reflections and refractions
-----------------------------------------------------------------------------
*/

#include "Ogre.h"

#include "ExampleApplication.h"
#include "OgreProgressiveMesh.h"
#include "OgreStringConverter.h"

// Hacky globals
Camera* theCam;
Entity* pPlaneEnt;
std::vector<Entity*> aboveWaterEnts;
std::vector<Entity*> belowWaterEnts;

// Fish!
#define NUM_FISH 30
#define NUM_FISH_WAYPOINTS 10
#define FISH_PATH_LENGTH 200 
AnimationState* fishAnimations[NUM_FISH];
SimpleSpline fishSplines[NUM_FISH];
Vector3 fishLastPosition[NUM_FISH];
SceneNode* fishNodes[NUM_FISH];
Real animTime = 0.0f;


Plane reflectionPlane;


class RefractionTextureListener : public RenderTargetListener
{
public:
    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Hide plane and objects above the water
        pPlaneEnt->setVisible(false);
        std::vector<Entity*>::iterator i, iend;
        iend = aboveWaterEnts.end();
        for (i = aboveWaterEnts.begin(); i != iend; ++i)
        {
            (*i)->setVisible(false);
        }

    }
    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Show plane and objects above the water
        pPlaneEnt->setVisible(true);
        std::vector<Entity*>::iterator i, iend;
        iend = aboveWaterEnts.end();
        for (i = aboveWaterEnts.begin(); i != iend; ++i)
        {
            (*i)->setVisible(true);
        }
    }

};
class ReflectionTextureListener : public RenderTargetListener
{
public:
    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Hide plane and objects below the water
        pPlaneEnt->setVisible(false);
        std::vector<Entity*>::iterator i, iend;
        iend = belowWaterEnts.end();
        for (i = belowWaterEnts.begin(); i != iend; ++i)
        {
            (*i)->setVisible(false);
        }
        theCam->enableReflection(reflectionPlane);

    }
    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Show plane and objects below the water
        pPlaneEnt->setVisible(true);
        std::vector<Entity*>::iterator i, iend;
        iend = belowWaterEnts.end();
        for (i = belowWaterEnts.begin(); i != iend; ++i)
        {
            (*i)->setVisible(true);
        }
        theCam->disableReflection();
    }

};

class FresnelFrameListener : public ExampleFrameListener
{
public:

    FresnelFrameListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam, false, false)
    {}
    bool frameStarted(const FrameEvent &evt)
    {
        animTime += evt.timeSinceLastFrame;
        while (animTime > FISH_PATH_LENGTH)
            animTime -= FISH_PATH_LENGTH;

        for (size_t fish = 0; fish < NUM_FISH; ++fish)
        {
            // Animate the fish
            fishAnimations[fish]->addTime(evt.timeSinceLastFrame*2);
            // Move the fish
            Vector3 newPos = fishSplines[fish].interpolate(animTime / FISH_PATH_LENGTH);
            fishNodes[fish]->setPosition(newPos);
            // Work out the direction
            Vector3 direction = fishLastPosition[fish] - newPos;
            direction.normalise();
            Quaternion orientation = -Vector3::UNIT_X.getRotationTo(direction);
            fishNodes[fish]->setOrientation(orientation);
            fishLastPosition[fish] = newPos;

        }



        return ExampleFrameListener::frameStarted(evt);
    }

};

class FresnelApplication : public ExampleApplication
{
protected:
    RefractionTextureListener mRefractionListener;
    ReflectionTextureListener mReflectionListener;
public:
    FresnelApplication() {
    
    
    }

    ~FresnelApplication() 
    {
    }
protected:
    


    // Just override the mandatory create scene method
    void createScene(void)
    {

        // Check prerequisites first
		const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !(caps->hasCapability(RSC_FRAGMENT_PROGRAM)))
        {
            Except(1, "Your card does not support vertex and fragment programs, so cannot "
                "run this demo. Sorry!", 
                "Fresnel::createScene");
        }
        else
        {
            if (!GpuProgramManager::getSingleton().isSyntaxSupported("arbfp1") &&
                !GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0") &&
				!GpuProgramManager::getSingleton().isSyntaxSupported("ps_1_4")
				)
            {
                Except(1, "Your card does not support advanced fragment programs, "
                    "so cannot run this demo. Sorry!", 
                "Fresnel::createScene");
            }
        }

        theCam = mCamera;
        theCam->setPosition(-100,20,700);
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        Entity* pEnt;

        
        RenderTexture* rttTex = mRoot->getRenderSystem()->createRenderTexture( "Refraction", 512, 512 );
		
        {
            Viewport *v = rttTex->addViewport( mCamera );
            Material* mat = (Material*)MaterialManager::getSingleton().getByName("Examples/FresnelReflectionRefraction");
            mat->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName("Refraction");
            v->setOverlaysEnabled(false);
            rttTex->addListener(&mRefractionListener);
        }
        
		
        rttTex = mRoot->getRenderSystem()->createRenderTexture( "Reflection", 512, 512 );
        {
            Viewport *v = rttTex->addViewport( mCamera );
            Material* mat = (Material*)MaterialManager::getSingleton().getByName("Examples/FresnelReflectionRefraction");
            mat->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName("Reflection");
            v->setOverlaysEnabled(false);
            rttTex->addListener(&mReflectionListener);
        }
        
        
        // Define a floor plane mesh
        reflectionPlane.normal = Vector3::UNIT_Y;
        reflectionPlane.d = 0;
        MeshManager::getSingleton().createPlane("ReflectPlane",reflectionPlane,
            1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
        pPlaneEnt = mSceneMgr->createEntity( "plane", "ReflectPlane" );
        pPlaneEnt->setMaterialName("Examples/FresnelReflectionRefraction");
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

        
        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");

        // My node to which all objects will be attached
        SceneNode* myRootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        // Above water entities - NB all meshes are static
        pEnt = mSceneMgr->createEntity( "head1", "head1.mesh" );
        myRootNode->attachObject(pEnt);
        aboveWaterEnts.push_back(pEnt);
        pEnt = mSceneMgr->createEntity( "Pillar1", "Pillar1.mesh" );
        myRootNode->attachObject(pEnt);
        aboveWaterEnts.push_back(pEnt);
        pEnt = mSceneMgr->createEntity( "Pillar2", "Pillar2.mesh" );
        myRootNode->attachObject(pEnt);
        aboveWaterEnts.push_back(pEnt);
        pEnt = mSceneMgr->createEntity( "Pillar3", "Pillar3.mesh" );
        myRootNode->attachObject(pEnt);
        aboveWaterEnts.push_back(pEnt);
        pEnt = mSceneMgr->createEntity( "Pillar4", "Pillar4.mesh" );
        myRootNode->attachObject(pEnt);
        aboveWaterEnts.push_back(pEnt);
        pEnt = mSceneMgr->createEntity( "UpperSurround", "UpperSurround.mesh" );
        myRootNode->attachObject(pEnt);
        aboveWaterEnts.push_back(pEnt);

        // Now the below water ents
        pEnt = mSceneMgr->createEntity( "LowerSurround", "LowerSurround.mesh" );
        myRootNode->attachObject(pEnt);
        belowWaterEnts.push_back(pEnt);
        pEnt = mSceneMgr->createEntity( "PoolFloor", "PoolFloor.mesh" );
        myRootNode->attachObject(pEnt);
        belowWaterEnts.push_back(pEnt);

        for (size_t fishNo = 0; fishNo < NUM_FISH; ++fishNo)
        {
            pEnt = mSceneMgr->createEntity("fish" + StringConverter::toString(fishNo), "fish.mesh");
            fishNodes[fishNo] = myRootNode->createChildSceneNode();
            fishAnimations[fishNo] = pEnt->getAnimationState("swim");
            fishAnimations[fishNo]->setEnabled(true);
            fishNodes[fishNo]->attachObject(pEnt);
            belowWaterEnts.push_back(pEnt);


            // Generate a random selection of points for the fish to swim to
            fishSplines[fishNo].setAutoCalculate(false);
            Vector3 lastPos;
            for (size_t waypoint = 0; waypoint < NUM_FISH_WAYPOINTS; ++waypoint)
            {
                Vector3 pos = Vector3(
                    Math::SymmetricRandom() * 700, -10, Math::SymmetricRandom() * 700);
                if (waypoint > 0)
                {
                    // check this waypoint isn't too far, we don't want turbo-fish ;)
                    // since the waypoints are achieved every 5 seconds, half the length
                    // of the pond is ok
                    while ((lastPos - pos).length() > 750)
                    {
                        pos = Vector3(
                            Math::SymmetricRandom() * 700, -10, Math::SymmetricRandom() * 700);
                    }
                }
                fishSplines[fishNo].addPoint(pos);
                lastPos = pos;
            }
            // close the spline
            fishSplines[fishNo].addPoint(fishSplines[fishNo].getPoint(0));
            // recalc
            fishSplines[fishNo].recalcTangents();


        }




    }

    void createFrameListener(void)
    {
        mFrameListener= new FresnelFrameListener(mWindow, mCamera);
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);
    }

};



#if OGRE_PLATFORM == PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"


INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    FresnelApplication app;

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        fprintf(stderr, "An exception has occured: %s\n",
                e.getFullDescription().c_str());
#endif
    }


    return 0;
}







