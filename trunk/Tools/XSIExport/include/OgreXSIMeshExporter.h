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
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreColourValue.h"
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
        /** This struct represents a unique vertex, identified from a unique 
        combination of components.
        */
        class UniqueVertex
        {
        public:
            bool initialised;
            Vector3 position;
            Vector3 normal;
            Vector2 uv[OGRE_MAX_TEXTURE_COORD_SETS];
            RGBA colour;
            // The index of the next component with the same base details
            // but with some variation
            size_t nextIndex;

            UniqueVertex();
            bool operator==(const UniqueVertex& rhs) const;

        };
        typedef std::vector<UniqueVertex> UniqueVertexList;
        // unique vertex list
        UniqueVertexList mUniqueVertices;
        // dynamic index list; 32-bit until we know the max vertex index
        typedef std::vector<size_t> IndexList;
        IndexList mIndices;


        /** Starts processing of a polygon mesh. 
        @remarks
            This clears the mUniqueVertices and mIndices collections and re-populates 
            mUniqueVertices with as many initialised entries as are contained 
            in the original position list. There must be at least this many vertices in the
            mesh, or more if there are discontinuities in the normal or UV
            data etc.
        @note must be called before first createOrRetrieveUniqueVertex for 
            each PolygonMesh
        */
        void startPolygonMesh(size_t origPositionCount, size_t indexCount);


        /** Try to look up an existing vertex with the same information, or
            create a new one.
        @remarks
            The routine is mastered on the original position index, since positions
            are the most basic variant data in a vertex. If the vertex at that
            index is uninitialised, then the data is simply populated. If it is
            initialised, then the values are compared. If any of the components
            disagree, then the value of nextIndex is examined. If zero, then 
            a new vertex is created at the end of the list, and the nextIndex
            value set. If it's non-zero, then that indexed vertex is looked up
            and compared again. This continues until a matching vertex is found
            or nextIndex is zero, in which case a new vertex is created.
        @par
            Note that this re-uses as many vertices as possible, and also places
            every unique vertex in it's final index in one pass, so the return value
            from this method can be used as an adjusted vertex index.
        @returns The index of the unique vertex
        */
        size_t createOrRetrieveUniqueVertex(size_t originalPositionIndex, 
            const UniqueVertex& vertex);

        /** Templatised method for writing indexes */
        template <typename T> void writeIndexes(T* buf);

        /** Create and fill a vertex buffer */
        void createVertexBuffer(VertexData* vd, unsigned short bufIdx);




    };

}
#endif

