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
#ifndef _SceneNode_H__
#define _SceneNode_H__

#include "OgrePrerequisites.h"

#include "OgreNode.h"

namespace Ogre {

    /** Class representing a node in the scene graph.
        @remarks
            A SceneNode is a type of Node which is used to organise objects in a scene.
            It has the same hierarchical transformation properties of the generic Node class,
            bu also adds the ability to attach world objects to the node, and stores hierarchical
            bounding volumes of the nodes in the tree.
            Child nodes are contained within the bounds of the parent, and so on down the
            tree, allowing for fast culling.
    */
    class _OgreExport SceneNode : public Node
    {

    protected:
        std::vector<MovableObject*> mObjects;
        std::vector<Camera*> mCameras;
        std::vector<Light*> mLights;

        /// SceneManager which created this node
        SceneManager* mCreator;

        /// World-Axis aligned bounding box, updated only through _update
        AxisAlignedBox mWorldAABB;

        /** Tells the SceneNode to update the world bound info it stores.
        */
        void _updateBounds(void);

        /** See Node. */
        Node* createChildImpl(void);

    public:
        /** Constructor, only to be called by the creator SceneManager.
        */
        SceneNode(SceneManager* creator);
        ~SceneNode();

        /** Creates a new Node as a child of this node.
            @param
                translate Initial translation offset of child relative to parent
            @param
                rotate Initial rotation relative to parent
        */
        SceneNode* createChild(const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);

        /** Gets a pointer to a child node.*/
        SceneNode* getChild(unsigned short index);

        /** Drops the specified child from this node. Does not delete the node, just detaches it from
            this parent, potentially to be reattached elsewhere.
        */
        SceneNode* removeChild(unsigned short index);

        /** Adds an instance of a scene object to this node.
        */
        void attachObject(MovableObject* obj);

        /** Reports the number of objects attached to this node.
        */
        unsigned short numAttachedObjects(void);

        /** Retrieves a pointer to an attached entity.
        */
        MovableObject* getAttachedObject(unsigned short index);

        /** Detaches the indexed Entity from this scene node.
        */
        MovableObject* detachObject(unsigned short index);

        /** Detaches all objects attached to this node.
        */
        void detachAllObjects(void);

        /** Adds a light to this node.
        */
        void attachLight(Light* l);

        /** Reports the number of lights attached to this node.
        */
        unsigned short numAttachedLights(void);

        /** Retrieves a pointer to an attached light.
        */
        Light* getAttachedLight(unsigned short index);

        /** Detaches the indexed Light from this scene node.
        */
        Light* detachLight(unsigned short index);

        /** Detaches all lights attached to this node.
        */
        void detachAllLights(void);

        /** Adds a Camera to this node.
        */
        void attachCamera(Camera* ent);

        /** Reports the number of cameras attached to this node.
        */
        unsigned short numAttachedCameras(void);

        /** Retrieves a pointer to an attached camera.
        */
        Camera* getAttachedCamera(unsigned short index);

        /** Detaches the indexed Camera from this scene node.
        */
        Camera* detachCamera(unsigned short index);

        /** Detaches all cameras from this node.
        */
        void detachAllCameras(void);

        /** Internal method to update the Node.
            @note
                Updates this scene node and any relevant children to incorporate transforms etc.
                Don't call this yourself unless you are writing a SceneManager implementation.
            @param
                cam Pointer to Camera object to be used in the next render
            @param
                updateChildren If true, the update cascades down to all children. Specify false if you wish to
                update children separately, e.g. because of a more selective SceneManager implementation.
        */
        virtual void _update(Camera* cam, bool updateChildren = true);

        /** Internal method which locates any visible objects attached to this node and adds them to the passed in queue.
            @remarks
                Should only be called by a SceneManager implementation, and only after the _updat method has been called to
                ensure transforms and world bounds are up to date.
                SceneManager implementations can choose to let the search cascade automatically, or choose to prevent this
                and select nodes themselves based on some other criteria.
            @param
                cam The active camera
            @param
                queue The SceneManager's rendering queue
            @param
                includeChildren If true, the call is cascaded down to all child nodes automatically.
        */
        void _findVisibleObjects(Camera* cam, RenderQueue* queue, bool includeChildren = true);

        /** Gets the axis-aligned bounding box of this node (and hence all subnodes).
        @remarks
            Recommended only if you are extending a SceneManager, because the bounding box returned
            from this method is only up to date after the SceneManager has called _update.
        */
        AxisAlignedBox _getWorldAABB(void) const;
    };


}// namespace

#endif
