/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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
#ifndef __XSIMESHEXPORTER_H__
#define __XSIMESHEXPORTER_H__

#include "OgrePrerequisites.h"
#include "OgreMesh.h"
#include "OgreHardwareBufferManager.h"
#include <xsi_x3dobject.h>
#include <xsi_string.h>
#include <xsi_application.h>
#include <xsi_geometry.h>

namespace Ogre {

    /** Class for performing a mesh export from XSI.
    */
    class XsiMeshExporter
    {
    public:
        XsiMeshExporter();
        virtual ~XsiMeshExporter();

        /** Perform an export to Ogre .mesh.
        @remarks
            Every PolygonMesh object is exported as a different SubMesh. Other
            object types are ignored.
        @param fileName Target file name
        @param objectName Optional object name to start the export at. If left
            blank, every PolygonMesh object in the scene is exported as part of
            the same mesh
        @param edgeLists Whether to calculate edge lists
        @param tangents Whether to calculate tangents
        */
        void exportMesh(const XSI::CString& fileName, const XSI::CString& objectName, 
            bool edgeLists, bool tangents);

    protected:
        /// Internal recursive method for exporting a node in the scene
        void exportX3DObject(Mesh* pMesh, XSI::X3DObject& x3dObj);
        /// Export a submesh from the attached information
        void exportSubMesh(Mesh* pMesh, XSI::X3DObject& x3dObj, XSI::Geometry& geom);

        // Ogre Objects
        LogManager* mLogManager;
        MeshManager* mMeshManager;
        HardwareBufferManager* mBufferManager;

        // XSI Objects
        XSI::Application mXsiApp;


    };

}
#endif

