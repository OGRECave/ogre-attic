/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#ifndef __KeyFrame_H__
#define __KeyFrame_H__

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"

namespace Ogre 
{

    /** A key frame in an animation sequence defined by an AnimationTrack.
    @remarks
        This class can be used as a basis for all kinds of key frames. 
        The unifying principle is that multiple KeyFrames define an 
        animation sequence, with the exact state of the animation being an 
        interpolation between these key frames.
    */
    class _OgreExport KeyFrame
    {
    public:
        /// Simple constructor, only used for creating temp objects
        KeyFrame();

        /** Default constructor, you should not call this but use AnimationTrack::createKeyFrame instead. */
        KeyFrame(Real time);

        /** Gets the time of this keyframe in the animation sequence. */
        Real getTime(void) const;
        /** Sets the translation associated with this keyframe. 
        @remarks    
            The translation factor affects how much the keyframe translates (moves) it's animable
            object at it's time index.
        @param trans The vector to translate by
        */
        void setTranslate(const Vector3& trans);

        /** Gets the translation applied by this keyframe. */
        const Vector3& getTranslate(void) const;

        /** Sets the scaling factor applied by this keyframe to the animable
        object at it's time index.
        @param scale The vector to scale by (beware of supplying zero values for any component of this
            vector, it will scale the object to zero dimensions)
        */
        void setScale(const Vector3& scale);

        /** Gets the scaling factor applied by this keyframe. */
        const Vector3& getScale(void) const;

        /** Sets the rotation applied by this keyframe.
        @param rot The rotation applied; use Quaternion methods to convert from angle/axis or Matrix3 if
            you don't like using Quaternions directly.
        */
        void setRotation(const Quaternion& rot);

        /** Gets the rotation applied by this keyframe. */
        const Quaternion& getRotation(void) const;


    protected:
        Real mTime;
        Vector3 mTranslate;
        Vector3 mScale;
        Quaternion mRotate;
    };

}


#endif

