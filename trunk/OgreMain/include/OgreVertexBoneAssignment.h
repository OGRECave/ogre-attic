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

#ifndef __VertexBoneAssignment_H__
#define __VertexBoneAssignment_H__

#include "OgrePrerequisites.h"


namespace Ogre 
{
    
    /** Records the assignment of a single vertex to a single bone with the corresponding weight.
    @remarks
        This simple struct simply holds a vertex index, bone index and weight representing the
        assignment of a vertex to a bone for skeletal animation. There may be many of these
        per vertex if blended vertex assignments are allowed.
    */
    typedef struct VertexBoneAssignment_s
    {
        unsigned short vertexIndex;
        unsigned short boneIndex;
        Real weight;

    } VertexBoneAssignment;


}

#endif
