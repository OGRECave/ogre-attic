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
        std::map<String, MovableObject*> mObjectsByName;

        /// SceneManager which created this node
        SceneManager* mCreator;

        /// World-Axis aligned bounding box, updated only through _update
        AxisAlignedBox mWorldAABB;

        /** Tells the SceneNode to update the world bound info it stores.
        */
        void _updateBounds(void);

        /** See Node. */
        Node* createChildImpl(void);

        /** See Node. */
        Node* createChildImpl(const String& name);
    public:
        /** Constructor, only to be called by the creator SceneManager.
        @remarks
            Creates a node with a generated name.
        */
        SceneNode(SceneManager* creator);
        /** Constructor, only to be called by the creator SceneManager.
        @remarks
            Creates a node with a specified name.
        */
        SceneNode(SceneManager* creator, const String& name);
        ~SceneNode();

        /** Creates a new unnamed Node as a child of this node.
        @remarks
            This creates a node with no name (or rather, a generated name). If you want
            to give the node a specific name, use the alternate form which takes a name
            as the first parameter, which allows you to look the node up from SceneManager::getSceneNode.
            @param
                translate Initial translation offset of child relative to parent
            @param
                rotate Initial rotation relative to parent
        */
        SceneNode* createChild(const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);

        /** Creates a new named Node as a child of this node.
        @remarks
            This creates a child node with a given name, which allows you to look the node up from 
            SceneManager::getSceneNode.
            @param
                translate Initial translation offset of child relative to parent
            @param
                rotate Initial rotation relative to parent
        */
        SceneNode* createChild(const String& name, const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);

        /** Gets a pointer to a child node.*/
        SceneNode* getChild(unsigned short index);

        /** Drops the specified child from this node. Does not delete the node, just detaches it from
            this parent, potentially to be reattached elsewhere.
        */
        SceneNode* removeChild(unsigned short index);

        /** Adds an instance of a scene object to this node.
        @remarks
            Scene objects can include Entity objects, Camera objects, Light objects, 
            ParticleSystem objects etc. Anything that subclasses from MovableObject.
        */
        void attachObject(MovableObject* obj);

        /** Reports the number of objects attached to this node.
        */
        unsigned short numAttachedObjects(void);

        /** Retrieves a pointer to an attached object.
        @remarks Retrieves by index, see alternate version to retrieve by name.
        */
        MovableObject* getAttachedObject(unsigned short index);

        /** Retrieves a pointer to an attached object.
        @remarks Retrieves by object name, see alternate version to retrieve by index.
        */
        MovableObject* getAttachedObject(const String& name);

        /** Detaches the indexed Entity from this scene node.
        */
        MovableObject* detachObject(unsigned short index);

        /** Detaches all objects attached to this node.
        */
        void detachAllObjects(void);

        /** Adds a light to this node.
        @remarks
            DEPRECATED. Use attachObject instead now.
        */
        void attachLight(Light* l);

        /** Adds a Camera to this node.
        @remarks
            DEPRECATED. Use attachObject instead now.
        */
        void attachCamera(Camera* ent);

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
            @param
                displayNodes If true, the nodes themselves are rendered as a set of 3 axes as well
                    as the objects being rendered. For debugging purposes.
        */
        void _findVisibleObjects(Camera* cam, RenderQueue* queue, 
            bool includeChildren = true, bool displayNodes = false);

        /** Gets the axis-aligned bounding box of this node (and hence all subnodes).
        @remarks
            Recommended only if you are extending a SceneManager, because the bounding box returned
            from this method is only up to date after the SceneManager has called _update.
        */
        AxisAlignedBox _getWorldAABB(void) const;
    };


}// namespace

#endif
