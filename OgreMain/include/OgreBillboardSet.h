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

#ifndef __BillboardSet_H__
#define __BillboardSet_H__

#include "OgrePrerequisites.h"

#include "OgreMovableObject.h"
#include "OgreRenderable.h"
#include "OgreAxisAlignedBox.h"
#include "OgreBillboard.h"
#include "OgreString.h"
#include "OgreColourValue.h"
#include "OgreStringInterface.h"

namespace Ogre {

    /** Enum covering what exactly a billboard's position means (center,
        top-left etc).
        @see
            BillboardSet::setBillboardOrigin
    */
    enum BillboardOrigin
    {
        BBO_TOP_LEFT,
        BBO_TOP_CENTER,
        BBO_TOP_RIGHT,
        BBO_CENTER_LEFT,
        BBO_CENTER,
        BBO_CENTER_RIGHT,
        BBO_BOTTOM_LEFT,
        BBO_BOTTOM_CENTER,
        BBO_BOTTOM_RIGHT
    };
    /** The type of billboard to use. */
    enum BillboardType
    {
        /// Standard point billboard (default), always faces the camera completely and is always upright
        BBT_POINT,
        /// Billboards are oriented around a shared direction vector (used as Y axis) and only rotate around this to face the camera
        BBT_ORIENTED_COMMON,
        /// Billboards are oriented around their own direction vector (their own Y axis) and only rotate around this to face the camera
        BBT_ORIENTED_SELF

    };

    /** A collection of billboards (faces which are always facing the camera) with the same (default) dimensions, material
        and which are fairly close proximity to each other.
        @remarks
            Billboards are rectangles made up of 2 tris which are always facing the camera. They are typically used
            for special effects like particles. This class collects together a set of billboards with the same (default) dimensions,
            material and relative locality in order to process them more efficiently. The entire set of billboards will be
            culled as a whole (by default, although this can be changed if you want a large set of billboards
            which are spread out and you want them culled individually), individual Billboards have locations which are relative to the set (which itself derives it's
            position from the SceneNode it is attached to since it is a MoveableObject), they will be rendered as a single rendering operation,
            and some calculations will be sped up by the fact that they use the same dimensions so some workings can be reused.
        @par
            A BillboardSet can be created using the SceneManager::createBillboardSet method. They can also be used internally
            by other classes to create effects.
    */
    class _OgreExport BillboardSet : public StringInterface, public MovableObject, public Renderable
    {

    protected:
        /** Private constructor (instances cannot be created directly).
        */
        BillboardSet();

        /// Name of the entity; used for location in the scene.
        String mName;

        /// Bounds of all billboards in this set
        AxisAlignedBox mAABB;

        /// Origin of each billboard
        BillboardOrigin mOriginType;

        /// Default width of each billboard
        Real mDefaultWidth;
        /// Default height of each billboard
        Real mDefaultHeight;

        /// Name of the material to use
        String mMaterialName;
        /// Pointer to the material to use
        Material* mpMaterial;

        /// True if no billboards in this set have been resized - greater efficiency.
        bool mAllDefaultSize;

        /// Flag indicating whether to autoextend pool
        bool mAutoExtendPool;

        typedef std::list<Billboard*> ActiveBillboardList;
        typedef std::deque<Billboard*> FreeBillboardQueue;
        typedef std::vector<Billboard*> BillboardPool;

        /** Active billboard list.
            @remarks
                This is a linked list of pointers to billboards in the billboard pool.
            @par
                This allows very fast instertions and deletions from anywhere in the list to activate / deactivate billboards
                (required for particle systems etc)    as well as resuse of Billboard instances in the pool
                without construction & destruction which avoids memory thrashing.
        */
        ActiveBillboardList mActiveBillboards;

        /** Free billboard queue.
            @remarks
                This contains a list of the billboards free for use as new instances
                as required by the set. Billboard instances are preconstructed up to the estimated size in the
                mBillboardPool vector and are referenced on this deque at startup. As they get used this deque
                reduces, as they get released back to to the set they get added back to the deque.
        */
        FreeBillboardQueue mFreeBillboards;

        /** Pool of billboard instances for use and reuse in the active billboard list.
            @remarks
                This vector will be preallocated with the estimated size of the set,and will extend as required.
        */
        BillboardPool mBillboardPool;


        /// The vertex position data for all billboards in this set.
        Real* mpPositions;

        /// The vertex colour data for all billboards in this set
        RGBA* mpColours;

        /// The texture coordinates for all billboards in this set
        Real* mpTexCoords;

        /// The vertex index data for all billboards in this set (1 set only)
        unsigned short* mpIndexes;

        /// Flag indicating whether each billboard should be culled separately (default: false)
        bool mCullIndividual;

        /// The type of billboard to render
        BillboardType mBillboardType;

        /// Common direction for billboards of type BBT_ORIENTED_COMMON
        Vector3 mCommonDirection;

        /// Internal method for culling individual billboards
        inline bool billboardVisible(Camera* cam, ActiveBillboardList::iterator bill);

        // Number of visible billboards (will be == getNumBillboards if mCullIndividual == false)
        unsigned short mNumVisibleBillboards;

        /// Internal method for increasing pool size
        virtual void increasePool(unsigned int size);


        //-----------------------------------------------------------------------
        // The internal methods which follow are here to allow maximum flexibility as to 
        //  when various components of the calculation are done. Depending on whether the
        //  billboards are of fixed size and whether they are point or oriented type will
        //  determine how much calculation has to be done per-billboard. NOT a one-size fits all approach.
        //-----------------------------------------------------------------------
        /** Internal method for generating billboard corners. 
        @remarks
            Optional parameter pBill is only present for type BBT_ORIENTED_SELF
        */
        virtual void genBillboardAxes(Camera& cam, Vector3* pX, Vector3 *pY, const Billboard* pBill = 0);

        /** Internal method, generates parametric offsets based on origin.
        */
        inline void getParametricOffsets(Real& left, Real& right, Real& top, Real& bottom);

        /** Internal method for generating vertex data. 
        @param pPos Pointer to pointer to vertex positions, will be updated
        @param pCol Pointer to pointer to vertex colours, will be updated
        @param offsets Array of 4 Vector3 offsets
        @param pBillboard Pointer to billboard
        @returns new vertex index
        */
        inline void genVertices(Real **pPos, RGBA** pCol, const Vector3* offsets, const Billboard* pBillboard);

        /** Internal method generates vertex offsets.
        @remarks
            Takes in parametric offsets as generated from getParametericOffsets, width and height values
            and billboard x and y axes as generated from genBillboardAxes. 
            Fills output array of 4 vectors with vector offsets
            from origin for left-top, right-top, left-bottom, right-bottom corners.
        */
        inline void genVertOffsets(Real inleft, Real inright, Real intop, Real inbottom,
            Real width, Real height,
            const Vector3& x, const Vector3& y, Vector3* pDestVec);
    public:

        /** Usual constructor - this is called by the SceneManager.
            @param
                name The name to give the billboard set (must be unique)
            @param
                poolSize The initial size of the billboard pool. Estimate of the number of billboards
                which will be required, and pass it using this parameter. The set will
                preallocate this number to avoid memory fragmentation. The default behaviour
                once this pool has run out is to double it.
            @see
                BillboardSet::setAutoextend
        */
        BillboardSet( const String& name, unsigned int poolSize = 20);

        virtual ~BillboardSet();

        /** Creates a new billboard and adds it to this set.
            @remarks
                Behaviour once the billboard pool has been exhausted depends on the
                BillboardSet::setAutoextendPool option.
            @param
                position The position of the new billboard realtive to the certer of the set
            @param
                colour Optional base colour of the billboard.
            @returns
                On success, a pointer to a newly created Billboard is
                returned.
            @par
                On failiure (i.e. no more space and can't autoextend),
                <b>NULL</b> is returned.
            @see
                BillboardSet::setAutoextend
        */
        Billboard* createBillboard(
            const Vector3& position,
            const ColourValue& colour = ColourValue::White );

        /** Creates a new billboard and adds it to this set.
            @remarks
                Behaviour once the billboard pool has been exhausted depends on the
                BillboardSet::setAutoextendPool option.
            @param
                x
            @param
                y
            @param
                z The position of the new billboard realtive to the certer of the set
            @param
                colour Optional base colour of the billboard.
            @returns
                On success, a pointer to a newly created Billboard is
                returned.
            @par
                On failiure (i.e. no more space and can't autoextend),
                <b>NULL</b> is returned.
            @see
                BillboardSet::setAutoextend
        */
        Billboard* createBillboard(
            Real x, Real y, Real z,
            const ColourValue& colour = ColourValue::White );

        /** Returns the number of active billboards which currently make up this set.
        */
        virtual int getNumBillboards(void) const;

        /** Tells the set whether to allow automatic extension of the pool of billboards.
            @remarks
                A BillboardSet stores a pool of pre-constructed billboards which are used as needed when
                a new billboard is requested. This allows applications to create / remove billboards efficiently
                without incurring construction / destruction costs (a must for sets with lots of billboards like
                particle effects). This method allows you to configure the behaviour when a new billboard is requested
                but the billboard pool has been exhausted.
            @par
                The default behaviour is to allow the pool to extend (typically this allocates double the current
                pool of billboards when the pool is expended), equivalent to calling this method with
                autoExtend = true. If you set the parameter to false however, any attempt to create a new billboard
                when the pool has expired will simply fail silently, returning a null pointer.
            @param autoextend true to double the pool every time it runs out, false to fail silently.
        */
        virtual void setAutoextend(bool autoextend);

        /** Returns true if the billboard pool automatically extends.
            @see
                BillboardSet::setAutoextend
        */
        virtual bool getAutoextend(void) const;

        /** Adjusts the size of the pool of billboards available in this set.
            @remarks
                See the BillboardSet::setAutoextend method for full details of the billboard pool. This method adjusts
                the preallocated size of the pool. If you try to reduce the size of the pool, the set has the option
                of ignoring you if too many billboards are already in use. Bear in mind that calling this method will
                incur significant construction / destruction calls so should be avoided in time-critical code. The same
                goes for auto-extension, try to avoid it by estimating the pool size correctly up-front.
            @param
                size The new size for the pool.
        */
        virtual void setPoolSize(unsigned int size);

        /** Returns the current size of the billboard pool.
            @returns
                The current size of the billboard pool.
            @see
                BillboardSet::setAutoextend
        */
        virtual unsigned int getPoolSize(void) const;


        /** Empties this set of all billboards.
        */
        virtual void clear();

        /** Returns a pointer to the billboard at the supplied index.
            @note
                This method requires linear time since the billboard list is a linked list.
            @param
                index The index of the billboard that is requested.
            @returns
                On success, a valid pointer to the requested billboard is
                returned.
            @par
                On failiure, <b>NULL</b> is returned.
        */
        virtual Billboard* getBillboard(unsigned int index) const;

        /** Removes the billboard at the supplied index.
            @note
                This method requires linear time since the billboard list is a linked list.
        */
        virtual void removeBillboard(unsigned int index);

        /** Removes a billboard from the set.
            @note
                This version is more efficient than removing by index.
        */
        virtual void removeBillboard(Billboard* pBill);

        /** Sets the point which acts as the origin point for all billboards in this set.
            @remarks
                This setting controls the fine tuning of where a billboard appears in relation to it's
                position. It could be that a billboard's position represents it's center (e.g. for fireballs),
                it could mean the center of the bottom edge (e.g. a tree which is positioned on the ground),
                the top-left corner (e.g. a cursor).
            @par
                The default setting is BBO_CENTER.
            @param
                origin A member of the BillboardOrigin enum specifying the origin for all the billboards in this set.
        */
        virtual void setBillboardOrigin(BillboardOrigin origin);

        /** Gets the point which acts as the origin point for all billboards in this set.
            @returns
                A member of the BillboardOrigin enum specifying the origin for all the billboards in this set.
        */
        virtual BillboardOrigin getBillboardOrigin(void) const;

        /** Sets the default dimensions of the billboards in this set.
            @remarks
                All billboards in a set are created with these default dimensions. The set will render most efficiently if
                all the billboards in the set are the default size. It is possible to alter the size of individual
                billboards at the expense of extra calculation. See the Billboard class for more info.
            @param width
                The new default width for the billboards in this set.
            @param height
                The new default height for the billboards in this set.
        */
        virtual void setDefaultDimensions(Real width, Real height);

        /** See setDefaultDimensions - this sets 1 component individually. */
        virtual void setDefaultWidth(Real width);
        /** See setDefaultDimensions - this gets 1 component individually. */
        virtual Real getDefaultWidth(void);
        /** See setDefaultDimensions - this sets 1 component individually. */
        virtual void setDefaultHeight(Real height);
        /** See setDefaultDimensions - this gets 1 component individually. */
        virtual Real getDefaultHeight(void);

        /** Sets the name of the material to be used for this billboard set.
            @param
                name The new name of the material to use for this set.
        */
        virtual void setMaterialName(const String& name);

        /** Sets the name of the material to be used for this billboard set.
            @returns The name of the material that is used for this set.
        */
        virtual const String& getMaterialName(void) const;

        /** Overridden from MovableObject
            @see
                MovableObject
        */
        virtual void _notifyCurrentCamera(Camera* cam);

        /** Overridden from MovableObject
            @see
                MovableObject
        */
        virtual const AxisAlignedBox& getBoundingBox(void) const;

        /** Overridden from MovableObject
            @see
                MovableObject
        */
        virtual void _updateRenderQueue(RenderQueue* queue);

        /** Overridden from MovableObject
            @see
                MovableObject
        */
        virtual Material* getMaterial(void) const;

        /** Overridden from MovableObject
            @see
                MovableObject
        */
        virtual void getRenderOperation(RenderOperation& rend);

        /** Overridden from MovableObject
            @see
                MovableObject
        */
        virtual void getWorldTransform(Matrix4& xform);

        /** Internal callback used by Billboards to notify their parent that they have been resized.
        */
        virtual void _notifyBillboardResized(void);

        /** Returns whether or not billbards in this are tested individually for culling. */
        virtual bool getCullIndividually(void);
        /** Sets whether culling tests billboards in this individually as well as in a group.
        @remarks
            Billboard sets are always culled as a whole group, based on a bounding box which 
            encloses all billboards in the set. For fairly localised sets, this is enough. However, you
            can optionally tell the set to also cull individual billboards in the set, i.e. to test
            each individual billboard before rendering. The default is not to do this.
        @par
            This is useful when you have a large, fairly distributed set of billboards, like maybe 
            trees on a landscape. You probably still want to group them into more than one
            set (maybe one set per section of landscape), which will be culled coarsely, but you also
            want to cull the billboards individually because they are spread out. Whilst you could have
            lots of single-tree sets which are culled separately, this would be inefficient to render
            because each tree would be issued as it's own rendering operation.
        @par
            By calling this method with a parameter of true, you can have large billboard sets which 
            are spaced out and so get the benefit of batch rendering and coarse culling, but also have
            fine-grained culling so unnecessary rendering is avoided.
        @param cullIndividual If true, each billboard is tested before being sent to the pipeline as well 
            as the whole set having to pass the coarse group bounding test.
        */
        virtual void setCullIndividually(bool cullIndividual);

        /** Sets the type of billboard to render.
        @remarks
            The default sort of billboard (BBT_POINT), always has both x and y axes parallel to 
            the camera's local axes. This is fine for 'point' style billboards (e.g. flares,
            smoke, anything which is symmetrical about a central point) but does not look good for
            billboards which have an orientation (e.g. an elongated raindrop). In this case, the
            oriented billboards are more suitable (BBT_ORIENTED_COMMON or BBT_ORIENTED_SELF) since they retain an independant Y axis
            and only the X axis is generated, perpendicular to both the local Y and the camera Z.
        @param bbt The type of billboard to render
        */
        virtual void setBillboardType(BillboardType bbt);

        /** Returns the billboard type in use. */
        virtual BillboardType getBillboardType(void);

        /** Use this to specify the common direction given to billboards of type BBT_ORIENTED_COMMON.
        @remarks
            Use BBT_ORIENTED_COMMON when you want oriented billboards but you know they are always going to 
            be oriented the same way (e.g. rain in calm weather). It is faster for the system to calculate
            the billboard vertices if they have a common direction.
        @param vec The direction for all billboards.
        */
        virtual void setCommonDirection(const Vector3& vec);

        /** Gets the common direction for all billboards (BBT_ORIENTED_COMMON) */
        virtual Vector3 getCommonDirection(void);
    };

}


#endif
