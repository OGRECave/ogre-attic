/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
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

#ifndef __MovableObject_H__
#define __MovableObject_H__

// Precompiler options
#include "OgrePrerequisites.h"

namespace Ogre {


    /** Abstract class definining a movable object in a scene.
        @remarks
            Instances of this class are discrete, relatively small, movable objects
            which are attached to SceneNode objects to define their position.
        @todo
            Add local OBB / convex hull
    */
    class _OgreExport MovableObject
    {
    public:
        /// Constructor
        MovableObject();

        /** Virtual destructor - read Scott Meyers if you don't know why this is needed.
        */
        virtual ~MovableObject() {}

        /** Returns the name of this object. */
        virtual const String& getName(void) const = 0;

        /** Returns the type name of this object. */
        virtual const String getMovableType(void) const = 0;

        /** Returns the node to which this object is attached.
            @remarks
                An object is not visible in the scene unless attached to a SceneNode.
            @par
                Attaching an object is done via the SceneNode::attachObject method.
        */
        virtual SceneNode* getParentNode(void);

        /** Internal method called to notify the object that it has been attached to a node.
        */
        virtual void _notifyAttached(SceneNode* parent);

        /** Returns true if this object is attached to a SceneNode. */
        virtual bool isAttached(void) const;

        /** Internal method to notify the object of the camera to be used for the next rendering operation.
            @remarks
                Certain objects may want to do specific processing based on the camera position. This method notifies
                them incase they wish to do this.
        */
        virtual void _notifyCurrentCamera(Camera* cam) = 0;

        /** Retrieves the local axis-aligned bounding box for this object.
            @remarks
                This bounding box is in local coordinates so will need to be transformed and
                converted into a world bounding box by the SceneNode.
        */
        virtual const AxisAlignedBox& getBoundingBox(void) const = 0;

        /** Internal method by which the movable object must add Renderable subclass instances to the rendering queue.
            @remarks
                The engine will call this methof when this object is to be rendered. The object must then create one or more
                Renderable subclass instances which it places on the passed in Queue for rendering.
        */
        virtual void _updateRenderQueue(RenderQueue* queue) = 0;

    protected:
        SceneNode* mParentNode;
    };

}
#endif
