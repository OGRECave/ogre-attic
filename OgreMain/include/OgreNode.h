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
#ifndef _Node_H__
#define _Node_H__

#include "OgrePrerequisites.h"

#include "OgreMatrix3.h"
#include "OgreMatrix4.h"
#include "OgreQuaternion.h"
#include "OgreAxisAlignedBox.h"

BEGIN_OGRE_NAMESPACE

/** Class representing a general-purpose node an articulated scene graph.
    @remarks
        A node in the scene graph is a node in a structured tree. A node contains
        information about the transformation which will apply to
        it and all of it's children. Child nodes can have transforms of their own, which
        are combined with their parent's transformations.
    @par
        This is an abstract class - concrete classes are based on this for specific purposes,
        e.g. SceneNode, Bone
*/
class _OgreExport Node
{

protected:
    Node* mParent;
    std::vector<Node*> mChildren;


    /// Stores the orientation of the node relative to it's parent.
    Quaternion mOrientation;

    /// Stores the position/translation of the node relative to its parent.
    Vector3 mPosition;

    /// Stores the scaling factor applied to this node
    Vector3 mScale;

    /// Stores whether this node inherits scale from it's parent
    bool mInheritScale;

    /// Flag indicating derived transform is out of date 
    bool mDerivedOutOfDate;

    /// Only available internally - notification of parent.
    void setParent(Node* parent);

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

    /** Cached combined scale.
        @par
            This member is the position derived by combining the
            local transformations and those of it's parents.
            This is updated when _updateFromParent is called by the
            SceneManager or the nodes parent.
    */
    Vector3 mDerivedScale;

    /** Triggers the node to update it's combined transforms.
        @par
            This method is called internally by Ogre to ask the node
            to update it's complete transformation based on it's parents
            derived transform.
    */
    virtual void _updateFromParent(void);

    /** Internal method for creating a new child node - must be overridden per subclass. */
    virtual Node* createChildImpl(void) = 0;

    /** Internal method for building a Matrix4 from orientation / scale / position. 
    @remarks
        Transform is performed in the order rotate, scale, translation, i.e. translation is independent
        of orientation axes, scale does not affect size of translation, rotation and scaling are always
        centered on the origin.
    */
    void makeTransform(
        const Vector3& position, 
        const Vector3& scale, 
        const Quaternion& orientation, 
        Matrix4& destMatrix );

public:
    /** Constructor, should only be called by parent, not directly.
    */
    Node();
    virtual ~Node();    

    /** Gets this node's parent (NULL if this is the root).
    */
    virtual Node* getParent(void) const;

    /** Returns a quaternion representing the nodes orientation.
    */
    virtual const Quaternion & getOrientation() const;

    /** Sets the orientation of this node via a quaternion.
    */
    virtual void setOrientation( const Quaternion& q );

    /** Resets the nodes orientation (local axes as world axes, no rotation).
    */
    virtual void resetOrientation(void);

    /** Sets the position of the node relative to it's parent.
    */
    virtual void setPosition(const Vector3& pos);

    /** Sets the position of the node relative to it's parent.
    */
    virtual void setPosition(Real x, Real y, Real z);

    /** Gets the position of the node relative to it's parent.
    */
    virtual const Vector3 & getPosition(void) const;

    /** Sets the scaling factor applied to this node.
    @remarks
        Scaling factors, unlike other transforms, are not always inherited by child nodes. 
        Whether or not scalings affect both the size and position of the child nodes depends on
        the setInheritScale option of the child. In some cases you want a scaling factor of a parent node
        to apply to a child node (e.g. where the child node is a part of the same object, so you
        want it to be the same relative size and position based on the parent's size), but
        not in other cases (e.g. where the child node is just for positioning another object,
        you want it to maintain it's own size and relative position). The default is to inherit
        as with other transforms.
    @par
        Note that like rotations, scalings are oriented around the node's origin.
    */
    virtual void setScale(const Vector3& scale);

    /** Sets the scaling factor applied to this node.
    @remarks
        Scaling factors, unlike other transforms, are not always inherited by child nodes. 
        Whether or not scalings affect both the size and position of the child nodes depends on
        the setInheritScale option of the child. In some cases you want a scaling factor of a parent node
        to apply to a child node (e.g. where the child node is a part of the same object, so you
        want it to be the same relative size and position based on the parent's size), but
        not in other cases (e.g. where the child node is just for positioning another object,
        you want it to maintain it's own size and relative position). The default is to inherit
        as with other transforms.
    @par
        Note that like rotations, scalings are oriented around the node's origin.
    */
    virtual void setScale(Real x, Real y, Real z);

    /** Gets the scaling factor of this node.
    */
    virtual const Vector3 & getScale(void) const;

    /** Tells the node whether it should inherit scaling factors from it's parent node.
    @remarks
        Scaling factors, unlike other transforms, are not always inherited by child nodes. 
        Whether or not scalings affect both the size and position of the child nodes depends on
        the setInheritScale option of the child. In some cases you want a scaling factor of a parent node
        to apply to a child node (e.g. where the child node is a part of the same object, so you
        want it to be the same relative size and position based on the parent's size), but
        not in other cases (e.g. where the child node is just for positioning another object,
        you want it to maintain it's own size and relative position). The default is to inherit
        as with other transforms.
    @param inherit If true, this node's scale and position will be affected by its parent's scale. If false,
        it will not be affected.
    */
    virtual void setInheritScale(bool inherit);

    /** Returns true if this node is affected by scaling factors applied to the parent node. 
    @remarks
        See setInheritScale for more info.
    */
    virtual bool getInheritScale(void) const;

    /** Scales the node, combining it's current scale with the passed in scaling factor. 
    @remarks
        This method applies an extra scaling factor to the node's existing scale, (unlike setScale
        which overwrites it) combining it's current scale with the new one. E.g. calling this 
        method twice with Vector3(2,2,2) would have the same effect as setScale(Vector3(4,4,4)) if
        the existing scale was 1.
    @par
        Note that like rotations, scalings are oriented around the node's origin.
    */
    virtual void scale(const Vector3& scale);

    /** Scales the node, combining it's current scale with the passed in scaling factor. 
    @remarks
        This method applies an extra scaling factor to the node's existing scale, (unlike setScale
        which overwrites it) combining it's current scale with the new one. E.g. calling this 
        method twice with Vector3(2,2,2) would have the same effect as setScale(Vector3(4,4,4)) if
        the existing scale was 1.
    @par
        Note that like rotations, scalings are oriented around the node's origin.
    */
    virtual void scale(Real x, Real y, Real z);

    /** Moves the node along the cartesian axes.
        @par
            This method moves the node by the supplied vector along the
            world cartesian axes, i.e. along world x,y,z
        @param 
            d Vector with x,y,z values representing the translation.
    */
    virtual void translate(const Vector3& d);
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
    virtual void translate(Real x, Real y, Real z);
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
    virtual void translate(const Matrix3& axes, const Vector3& move);
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
    virtual void translate(const Matrix3& axes, Real x, Real y, Real z);

    /** Rotate the node around the Z-axis.
    */
    virtual void roll(Real degrees);

    /** Rotate the node around the X-axis.
    */
    virtual void pitch(Real degrees);

    /** Rotate the node around the Y-axis.
    */
    virtual void yaw(Real degrees);

    /** Rotate the node around an arbitrary axis.
    */
    virtual void rotate(const Vector3& axis, Real degrees);

    /** Rotate the node around an aritrary axis using a Quarternion.
    */
    virtual void rotate(const Quaternion& q);

    /** Gets a matrix whose columns are the local axes based on
        the nodes orientation relative to it's parent. */
    virtual Matrix3 getLocalAxes(void);

    /** Creates a new Node as a child of this node.
    @remarks
        NOTE NON-VIRTUAL!
        This is actually in order to maintain the previous interfaces of the subclass
        SceneNode which must return a SceneNode pointer instead. Because of lack of
        support for covariant return types in some compilers (inc VC6) methods which
        differ only by return type cannot be virtual.
    @param
        translate Initial translation offset of child relative to parent
    @param
        rotate Initial rotation relative to parent
    */
    Node* createChild(
        const Vector3& translate = Vector3::ZERO, 
        const Quaternion& rotate = Quaternion::IDENTITY );

    /** Adds a (precreated) child scene node to this node.
    */
    virtual void addChild(Node* child);

    /** Reports the number of child nodes under this one.
    */
    virtual unsigned short numChildren(void) const;

    /** Gets a pointer to a child node.
    @remarks
        NOTE NON-VIRTUAL!
        This is actually in order to maintain the previous interfaces of the subclass
        SceneNode which must return a SceneNode pointer instead. Because of lack of
        support for covariant return types in some compilers (inc VC6) methods which
        differ only by return type cannot be virtual.
    */
    Node* getChild(unsigned short index) const;    

    /** Drops the specified child from this node. 
    @remarks
        Does not delete the node, just detaches it from
        this parent, potentially to be reattached elsewhere.
    @par
        NOTE NON-VIRTUAL!
        This is actually in order to maintain the previous interfaces of the subclass
        SceneNode which must return a SceneNode pointer instead. Because of lack of
        support for covariant return types in some compilers (inc VC6) methods which
        differ only by return type cannot be virtual.
    */
    Node* removeChild(unsigned short index);

    /** Removes all child Nodes attached to this node. Does not delete the nodes, just detaches them from
        this parent, potentially to be reattached elsewhere.
    */
    virtual void removeAllChildren(void);

    /** Gets the orientation of the node as derived from all parents.
    */
    virtual const Quaternion & _getDerivedOrientation(void);

    /** Gets the position of the node as derived from all parents.
    */
    virtual const Vector3 & _getDerivedPosition(void);

    /** Gets the scaling factor of the node as derived from all parents.
    */
    virtual const Vector3 & _getDerivedScale(void);

    /** Gets the full transformation matrix for this node.
        @remarks
            This method returns the full transformation matrix
            for this node, including the effect of any parent node
            transformations, provided they have been updated using the Node::_update method.
            This should only be called by a SceneManager which knows the
            derived transforms have been updated before calling this method.
            Applications using Ogre should just use the relative transforms.
    */
    virtual Matrix4 _getFullTransform(void);

    /** Internal method to update the Node.
        @note
            Updates this node and any relevant children to incorporate transforms etc.
            Don't call this yourself unless you are writing a SceneManager implementation.
        @param
            updateChildren If true, the update cascades down to all children. Specify false if you wish to
            update children separately, e.g. because of a more selective SceneManager implementation.
    */
    virtual void _update(bool updateChildren = true);

};

END_OGRE_NAMESPACE

#endif
