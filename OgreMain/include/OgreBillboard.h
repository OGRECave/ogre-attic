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

#ifndef __Billboard_H__
#define __Billboard_H__

#include "OgrePrerequisites.h"

#include "OgreVector3.h"
#include "OgreColourValue.h"


namespace Ogre {

    /** A billboard is a primitive which always faces the camera in every frame.
        @remarks
            Billboards can be used for special effects or some other trickery which requires the
            triangles to always facing the camera no matter where it is. Ogre groups billboards into
            sets for efficiency, so you should never create a billboard on it's own (it's ok to have a
            set of one if you need it).
        @par
            Billboards have their geometry generated every frame depending on where the camera is. It is most
            beneficial for all billboards in a set to be identically sized since Ogre can take advantage of this and
            save some calculations - useful when you have sets of hundreds of billboards as is possible with special
            effects. You can deviate from this if you wish (example: a smoke effect would probably have smoke puffs
            expanding as they rise, so each billboard will legitimately have it's own size) but be aware the extra
            overhead this brings and try to avoid it if you can.
        @par
            Billboards are just the mechanism for rendering a range of effects such as particles. It is other classes
            which use billboards to create their individual effects, so the methods here are quite generic.
        @see
            BollboardSet
    */

    class _OgreExport Billboard
    {
        friend class BillboardSet;
    protected:
        bool mOwnDimensions;
        Real mWidth;
        Real mHeight;
    public:
        // Note the intentional public access to main internal variables used at runtime
        // Forcing access via get/set would be too costly for 000's of billboards
        Vector3 mPosition;
        // NB direction only relevant for rendering when billboard type = BBT_ORIENTED_SELF
        Vector3 mDirection;
        BillboardSet* mParentSet;
        ColourValue mColour;
		Real mRotation;

        /** Default constructor.
        */
        Billboard();

        /** Default destructor.
        */
        ~Billboard();

        /** Normal constructor as called by BillboardSet.
        */
        Billboard(const Vector3& position, BillboardSet* owner, const ColourValue& colour = ColourValue::White);

        /** Get the rotation of the billboard.
            @remarks
                This rotation is relative to the center of the billboard.
				Internally we use radians, externally degrees
        */
        Real getRotation(void) const;

        /** Set the rotation of the billboard.
            @remarks
                This rotation is relative to the center of the billboard.
				Internally we use radians, externally degrees
        */
        void setRotation(Real rotation);

        /** Set the position of the billboard.
            @remarks
                This position is relative to a point on the quad which is the billboard. Depending on the BillboardSet,
                this may be the center of the quad, the top-left etc. See BillboardSet::setBillboardOrigin for more info.
        */
        void setPosition(const Vector3& position);

        /** Set the position of the billboard.
            @remarks
                This position is relative to a point on the quad which is the billboard. Depending on the BillboardSet,
                this may be the center of the quad, the top-left etc. See BillboardSet::setBillboardOrigin for more info.
        */
        void setPosition(Real x, Real y, Real z);

        /** Get the position of the billboard.
            @remarks
                This position is relative to a point on the quad which is the billboard. Depending on the BillboardSet,
                this may be the center of the quad, the top-left etc. See BillboardSet::setBillboardOrigin for more info.
        */
        const Vector3& getPosition(void) const;

        /** Sets the width and height for this billboard.
            @remarks
                Note that it is most efficient for every billboard in a BillboardSet to have the same dimensions. If you
                choose to alter the dimensions of an individual billboard the set will be less efficient. Do not call
                this method unless you really need to have different billboard dimensions within the same set. Otherwise
                just call the BillboardSet::setDefaultDimensions method instead.
        */
        void setDimensions(Real width, Real height);

        /** Resets this Billboard to use the parent BillboardSet's dimensions instead of it's own. */
        void resetDimensions(void) { mOwnDimensions = false; }
        /** Sets the colour of this billboard.
            @remarks
                Billboards can be tinted based on a base colour. This allows variations in colour irresective of the
                base colour of the material allowing more varied billboards. The default colour is white.
                The tinting is effected using vertex colours.
        */
        void setColour(const ColourValue& colour);

        /** Gets the colour of this billboard.
        */
        const ColourValue& getColour(void) const;

        /** Returns true if this billboard deviates from the BillboardSet's default dimensions (i.e. if the
            Billboard::setDimensions method has been called for this instance).
            @see
                Billboard::setDimensions
        */
        bool hasOwnDimensions(void) const;

        /** Retrieves the billboard's personal width, if hasOwnDimensions is true. */
        Real getOwnWidth(void) const;

        /** Retrieves the billboard's personal width, if hasOwnDimensions is true. */
        Real getOwnHeight(void) const;

        /** Internal method for notifying the billboard of it's owner.
        */
        void _notifyOwner(BillboardSet* owner);

    };

}

#endif
