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
#ifndef _SceneNode_H__
#define _SceneNode_H__

#include "OgrePrerequisites.h"

#include "OgreNode.h"
#include "OgreIteratorWrappers.h"

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
    public:
        typedef HashMap<String, MovableObject*, _StringHash> ObjectMap;
        typedef MapIterator<ObjectMap> ObjectIterator;

    protected:
        ObjectMap mObjectsByName;

		/// Pointer to a Wire Bounding Box for this Node
		WireBoundingBox *mWireBoundingBox;
		/// Flag that determines if the bounding box of the node should be displayed
		bool mShowBoundingBox;

        /// SceneManager which created this node
        SceneManager* mCreator;

        /// World-Axis aligned bounding box, updated only through _update
        AxisAlignedBox mWorldAABB;

        /** Tells the SceneNode to update the world bound info it stores.
        */
        virtual void _updateBounds(void);

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

        /** Adds an instance of a scene object to this node.
        @remarks
            Scene objects can include Entity objects, Camera objects, Light objects, 
            ParticleSystem objects etc. Anything that subclasses from MovableObject.
        */
        virtual void attachObject(MovableObject* obj);

        /** Reports the number of objects attached to this node.
        */
        virtual unsigned short numAttachedObjects(void);

        /** Retrieves a pointer to an attached object.
        @remarks Retrieves by index, see alternate version to retrieve by name. The index
        of an object may change as other objects are added / removed.
        */
        virtual MovableObject* getAttachedObject(unsigned short index);

        /** Retrieves a pointer to an attached object.
        @remarks Retrieves by object name, see alternate version to retrieve by index.
        */
        virtual MovableObject* getAttachedObject(const String& name);

        /** Detaches the indexed object from this scene node.
        @remarks
            Detaches by index, see the alternate version to detach by name. Object indexes
            may change as other objects are added / removed.
        */
        virtual MovableObject* detachObject(unsigned short index);

        /** Detaches the named object from this node and returns a pointer to it. */
        virtual MovableObject* detachObject(const String& name);

        /** Detaches all objects attached to this node.
        */
        virtual void detachAllObjects(void);

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
                updateChildren If true, the update cascades down to all children. Specify false if you wish to
                update children separately, e.g. because of a more selective SceneManager implementation.
            @param
                parentHasChanged This flag indicates that the parent xform has changed,
                    so the child should retrieve the parent's xform and combine it with its own
                    even if it hasn't changed itself.
        */
        virtual void _update(bool updateChildren, bool parentHasChanged);

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
        virtual void _findVisibleObjects(Camera* cam, RenderQueue* queue, 
            bool includeChildren = true, bool displayNodes = false);

        /** Gets the axis-aligned bounding box of this node (and hence all subnodes).
        @remarks
            Recommended only if you are extending a SceneManager, because the bounding box returned
            from this method is only up to date after the SceneManager has called _update.
        */
        virtual AxisAlignedBox _getWorldAABB(void) const;

        /** Retrieves an iterator which can be used to efficiently step through the objects 
            attached to this node.
        @remarks
            This is a much faster way to go through <B>all</B> the objects attached to the node
            than using getAttachedObject. But the iterator returned is only valid until a change
            is made to the collection (ie an addition or removal) so treat the returned iterator
            as transient, and don't add / remove items as you go through the iterator, save changes
            until the end, or retrieve a new iterator after making the change. Making changes to
            the object returned through the iterator is OK though.
        */
        virtual ObjectIterator getAttachedObjectIterator(void);


        /** Gets the creator of this scene node. 
        @remarks
            This method returns the SceneManager which created this node.
            This can be useful for destroying this node.
        */
        SceneManager* getCreator(void);

        /** This method removes and destroys the named child and all of its children.
        @remarks
            Unlike removeChild, which removes a single named child from this
            node but does not destroy it, this method destroys the child
            and all of it's children. 
        @par
            Use this if you wish to recursively destroy a node as well as 
            detaching it from it's parent. Note that any objects attached to
            the nodes will be detached but will not themselves be destroyed.
        */
        virtual void removeAndDestroyChild(const String& name);

        /** This method removes and destroys the child and all of its children.
        @remarks
            Unlike removeChild, which removes a single named child from this
            node but does not destroy it, this method destroys the child
            and all of it's children. 
        @par
            Use this if you wish to recursively destroy a node as well as 
            detaching it from it's parent. Note that any objects attached to
            the nodes will be detached but will not themselves be destroyed.
        */
        virtual void removeAndDestroyChild(unsigned short index);

        /** Removes and destroys all children of this node.
        @remarks
            Use this to destroy all child nodes of this node and remove
            them from the scene graph. Note that all objects attached to this
            node will be detached but will not be destroyed.
        */
        virtual void removeAndDestroyAllChildren(void);

        /** Allows the showing of the node's bounding box.
        @remarks
            Use this to show or hide the bounding box of the node.
        */
		virtual void showBoundingBox(bool bShow);

        /** Add the bounding box to the rendering queue.
        */
		virtual void _addBoundingBoxToQueue(RenderQueue* queue);

        /** This allows scene managers to determine if the node's bounding box
			should be added to the rendering queue.
        @remarks
            Scene Managers that implement their own _findVisibleObjects will have to 
			check this flag and then use _addBoundingBoxToQueue to add the bounding box
			wireframe.
        */
		virtual bool getShowBoundingBox();

        /** Creates an unnamed new SceneNode as a child of this node.
        @param
            translate Initial translation offset of child relative to parent
        @param
            rotate Initial rotation relative to parent
        */
        virtual SceneNode* createChildSceneNode(
            const Vector3& translate = Vector3::ZERO, 
            const Quaternion& rotate = Quaternion::IDENTITY );

        /** Creates a new named SceneNode as a child of this node.
        @remarks
            This creates a child node with a given name, which allows you to look the node up from 
            the parent which holds this collection of nodes.
            @param
                translate Initial translation offset of child relative to parent
            @param
                rotate Initial rotation relative to parent
        */
        virtual SceneNode* createChildSceneNode(const String& name, const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);

    };


}// namespace

#endif
