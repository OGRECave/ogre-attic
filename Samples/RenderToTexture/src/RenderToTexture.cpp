/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    \file 
        RenderToTexture.cpp
    \brief
        Shows OGRE's RenderToTexture, or scene blending features.
    \par
        Note that this is a little rudimentary - it's because whilst
        OGRE supports lots of blending options, the SceneManager has
        to ensure the rendering order is correct when object RenderToTexture
        is enabled. Right now this is not quite right in the default
        manager so this scene is kept deliberately simple.
*/

#include "Ogre.h"
#include "RenderToTexture.h"

int main(int argc, char ** argv)
{

    // Create application object
    RenderToTextureApplication app;

    app.go();

    return 0;
}
