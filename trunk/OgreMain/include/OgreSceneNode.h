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

#include "OgreMatrix3.h"
#include "OgreMatrix4.h"
#include "OgreQuaternion.h"
#include "OgreAxisAlignedBox.h"

namespace Ogre {

    /** Class representing a node in the scene graph.
        @remarks
            A node in the scene graph is a node in a structured tree
            containing all objects in the scene. A node contains
            information about the transformation which will apply to
            it and all of it's children.
        @par
            A SceneNode and it's descendants are related through an inherited
            transformation, and through a hierarchical set of bounding volumes.
            Child nodes are contained within the bounds of the parent, and so on down the
            tree, allowing for fast culling.
    */
    class _OgreExport SceneNode
    {

    protected:
        SceneNode* mParent;
        std::vector<SceneNode*> mChildren;
        std::vector<MovableObject*> mObjects;
        std::vector<Camera*> mCameras;
        std::vector<Light*> mLights;

        /// SceneManager which created this
        SceneManager* mCreator;

        /// Stores the orientation of the node relative to it's parent.
        Quaternion mOrientation;

        /// Stores the position/translation of the node relative to its parent.
        Vector3 mPosition;

        /// Flag indicating derived transform is out of date 
        bool mDerivedOutOfDate;

        /// Only available internally - notification of parent.
        void setParent(SceneNode* parent);

    private:
        /// World-Axis aligned bounding box, updated only through _update
        AxisAlignedBox mWorldAABB;

        /** Cached combined orientation.
            @par
                This member is the orientation derived by combining the
                local transformations and those of it's parents.
                This is updated when _updateFromParent is called by the
                SceneManager or the nodes parent.
        */
        Quaternion mDerivedOrientation;

        /** Cached combined position.
            @par
                This member is the position derived by combining the
                local transformations and those of it's parents.
                This is updated when _updateFromParent is called by the
                SceneManager or the nodes parent.
        */
        Vector3 mDerivedPosition;

        /** Triggers the node to update it's combined transforms.
            @par
                This method is called internally by Ogre to ask the node
                to update it's complete transformation based on it's parents
                derived transform.
        */
        void _updateFromParent(void);

        /** Tells the SceneNode to update the world bound info it stores.
        */
        void _updateBounds(void);

    public:
        /** Constructor, only to be called by the creator SceneManager.
        */
        SceneNode(SceneManager* creator);
        ~SceneNode();

        /** Gets this node's parent (0 if this is the root).
        */
        SceneNode* getParent(void);

        /** Returns a quaternion representing the nodes orientation.
        */
        Quaternion getOrientation();

        /** Sets the orientation of this node via a quaternion.
        */
        void setOrientation(Quaternion q);

        /** Resets the nodes orientation (local axes as world axes, no rotation).
        */
        void resetOrientation(void);

        /** Sets the position of the node relative to it's parent.
        */
        void setPosition(const Vector3& pos);

        /** Sets the position of the node relative to it's parent.
        */
        void setPosition(Real x, Real y, Real z);

        /** Gets the position of the node relative to it's parent.
        */
        Vector3 getPosition(void);


        /** Moves the node along the cartesian axes.
            @par
                This method moves the node by the supplied vector along the
                world cartesian axes, i.e. along world x,y,z
            @param 
                d Vector with x,y,z values representing the translation.
        */
        void translate(const Vector3& d);
        /** Moves the node along the cartesian axes.
            @par
                This method moves the node by the supplied vector along the
                world cartesian axes, i.e. along world x,y,z
            @param 
                x
            @param
                y
            @param
                z Real x, y and z values representing the translation.
        */
        void translate(Real x, Real y, Real z);
        /** Moves the node along arbitrary axes.
            @remarks
                This method translates the node by a vector which is relative to
                a custom set of axes.
            @param 
                axes A 3x3 Matrix containg 3 column vectors each representing the
                axes X, Y and Z respectively. In this format the standard cartesian
                axes would be expressed as:
                <pre>
                1 0 0
                0 1 0
                0 0 1
                </pre>
                i.e. the identity matrix.
            @param 
                move Vector relative to the axes above.
        */
        void translate(const Matrix3& axes, const Vector3& move);
        /** Moves the node along arbitrary axes.
            @remarks
            This method translates the node by a vector which is relative to
            a custom set of axes.
            @param 
                axes A 3x3 Matrix containg 3 column vectors each representing the
                axes X, Y and Z respectively. In this format the standard cartesian
                axes would be expressed as
                <pre>
                1 0 0
                0 1 0
                0 0 1
                </pre>
                i.e. the identity matrix.
            @param 
                x,y,z Translation components relative to the axes above.
        */
        void translate(const Matrix3& axes, Real x, Real y, Real z);

        /** Rotate the node around the Z-axis.
        */
        void roll(Real degrees);

        /** Rotate the node around the X-axis.
        */
        void pitch(Real degrees);

        /** Rotate the node around the Y-axis.
        */
        void yaw(Real degrees);

        /** Rotate the node around an arbitrary axis.
        */
        void rotate(const Vector3& axis, Real degrees);

        /** Rotate the node around an aritrary axis using a Quarternion.
        */
        void rotate(const Quaternion& q);

        /** Gets a matrix whose columns are the local axes based on
            the nodes orientation relative to it's parent. */
        Matrix3 getLocalAxes(void);

        /** Creates a new SceneNode as a child of this node.
            @param
                translate Initial translation offset of child relative to parent
            @param
                rotate Initial rotation relative to parent
        */
        SceneNode* createChild(const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);

        /** Adds a (precreated) child scene node to this node.
        */
        void addChild(SceneNode* child);

        /** Reports the number of child nodes under this one.
        */
        unsigned short numChildren(void);

        /** Gets a pointer to a child node.*/
        SceneNode* getChild(unsigned short index);

        /** Drops the specified child from this node. Does not delete the node, just detaches it from
            this parent, potentially to be reattached elsewhere.
        */
        SceneNode* removeChild(unsigned short index);

        /** Removes all child SceneNodes attached to this node. Does not delete the nodes, just detaches them from
            this parent, potentially to be reattached elsewhere.
        */
        void removeAllChildren(void);

        /*  Attached objects are listed below

            Remarks:
                Note that I could have abstracted this using a base class
                for Entity, Camera and Light, but these objects are used
                heavily in the rendering loop so I didn't want virtual functions
                to be used. It's less elegant, but faster this way.
        */

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

        /** Gets the orientation of the node as derived from all parents.
            @note
                Should only be used if you know the node has been updated from
                it's parent. Designed for use internally only.
        */
        Quaternion _getDerivedOrientation(void);

        /** Gets the position of the node as derived from all parents.
            @note
                Should only be used if you know the node has been updated from
                it's parent. Designed for use internally only.
        */
        Vector3 _getDerivedPosition(void);

        /** Gets the full transformation matrix for this node.
            @remarks
                This method returns the full transformation matrix
                for this node, including the effect of any parent node
                transformations, provided they have been updated using the SceneNode::_update method.
                This should only be called by a SceneManager which knows the
                derived transforms have been updated before calling this method.
                Applications using Ogre should just use the relative transforms.
        */
        Matrix4 _getFullTransform(void);

        /** Internal method to update the SceneNode.
            @note
                Updates this scene node and any relevant children to incorporate transforms etc.
                Don't call this yourself unless you are writing a SceneManager implementation.
            @param
                cam Pointer to Camera object to be used in the next render
            @param
                updateChildren If true, the update cascades down to all children. Specify false if you wish to
                update children separately, e.g. because of a more selective SceneManager implementation.
        */
        void _update(Camera* cam, bool updateChildren = true);

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
    };


}// namespace

#endif
