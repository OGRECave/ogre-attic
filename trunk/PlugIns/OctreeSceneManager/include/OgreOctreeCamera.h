/***************************************************************************
octreecamera.h  -  description
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

#ifndef OCTREECAMERA_H
#define OCTREECAMERA_H

#include <OgreCamera.h>
#include <OgreHardwareBufferManager.h>
#include <OgreSimpleRenderable.h>

/**
*@author Jon Anderson
*/

namespace Ogre
{

class Octree;


/** Specialized viewpoint from which an Octree can be rendered.
@remarks
This class contains several speciliazations of the Ogre::Camera class. It
implements the getRenderOperation method inorder to return displayable geometry
for debuggin purposes. It also implements a visibility function that is more granular
than the default.
*/

class OctreeCamera : public Camera
{
public:

    /** Visibility types */
    enum Visibility
    {
        NONE,
        PARTIAL,
        FULL
    };

    /* Standard Constructor */
    OctreeCamera( const String& name, SceneManager* sm );
    /* Standard destructor */
    ~OctreeCamera();

    /** Returns the visiblity of the box
    */
    OctreeCamera::Visibility getVisibility( const AxisAlignedBox &bound );

};

}

#endif
