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

#ifndef __BoneTrack_H__
#define __BoneTrack_H__

#include "OgrePrerequisites.h"
#include "OgreAnimationTrack.h"


namespace Ogre
{

    /** Specialisation of AnimationTrack for animating bones in a skeleton. 
    @remarks
        This class specialises the AnimationTrack class for animating bones
        in a skeleton, for eventually animating meshes. The track links back 
        to the bone which it affects via the bone index; note that the
        parent Animation is owned by the Skeleton in question so there is
        no question about which skeleton this refers to (and thus no
        Skeleton pointer).
    */
    class _OgreExport BoneTrack : public AnimationTrack
    {
    public:
        /** Constructor. */
        BoneTrack(Animation* parent);
        ~BoneTrack();

        /** Returns the bine index affected by this track. */
        unsigned short getBoneIndex(void);

        /** Sets the index of the bone affected by this track. */
        void setBoneIndex(unsigned short idx);

    protected:
        /// The handle of the root bone this animation affects
        unsigned short mBoneIndex;

    };

}

#endif
