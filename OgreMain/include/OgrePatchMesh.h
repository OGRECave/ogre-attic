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
#ifndef __PatchMesh_H__
#define __PatchMesh_H__

#include "OgrePrerequisites.h"
#include "OgreMesh.h"
#include "OgrePatchSurface.h"

namespace Ogre {

    /** Patch specialisation of Mesh. 
    @remarks
        Instances of this class should be created by calling MeshManager::createBezierPatch.
    */
    class _OgreExport PatchMesh : public Mesh
    {
    protected:
        /// Internal surface definition
        PatchSurface mSurface;
        /// Vertex declaration, cloned from the input
        VertexDeclaration* mDeclaration;
    public:
        /// Constructor, as defined in MeshManager::createBezierPatch
        PatchMesh(const String& name, void* controlPointBuffer, 
            VertexDeclaration *declaration, size_t width, size_t height,
            size_t uMaxSubdivisionLevel = PatchSurface::AUTO_LEVEL, 
            size_t vMaxSubdivisionLevel = PatchSurface::AUTO_LEVEL,
            PatchSurface::VisibleSide visibleSide = PatchSurface::VS_FRONT,
            HardwareBuffer::Usage vbUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
            HardwareBuffer::Usage ibUsage = HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY,
            bool vbUseShadow = false, bool ibUseShadow = false);

        /* Sets the current subdivision level as a proportion of full detail.
        @param factor Subdivision factor as a value from 0 (control points only) to 1 (maximum
            subdivision). */
        void setSubdivision(Real factor);

        /// Overridden from Resource
        void load(void);

    };

}

#endif
