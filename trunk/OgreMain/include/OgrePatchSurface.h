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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/
#ifndef __PatchSurface_H__
#define __PatchSurface_H__

#include "OgrePrerequisites.h"

#include "OgreVector3.h"
#include "OgreGeometryData.h"
#include "OgreString.h"

namespace Ogre {

    /** A surface which is defined by curves of some kind to form a patch, e.g. a Bezier patch.
        @remarks
            This object will take a list of control points with various assorted data, and will
            subdivide it into a patch mesh. Currently only Bezier curves are supported for defining
            the surface, but other techniques such as NURBS would follow the same basic approach.
    */
    class _OgreExport PatchSurface
    {
    public:
        PatchSurface();
        ~PatchSurface();

        enum PatchSurfaceType
        {
            /// A patch defined by a set of bezier curves
            PST_BEZIER
        };

        /// Constant for indicating automatic determination of subdivision level for patches
        enum
        {
            AUTO_LEVEL = -1
        };

        enum VisibleSide {
            /// The side from which u goes right and v goes up (as in texture coords)
            VS_FRONT,
            /// The side from which u goes right and v goes down (reverse of texture coords)
            VS_BACK,
            /// Both sides are visible - warning this creates 2x the number of triangles and adds extra overhead for calculating normals
            VS_BOTH
        };
        /** Sets up the surface by defining it's control points, type and initial subdivision level.
            @remarks
                This method initialises the surface by passing it a set of control points (in the form of
                a GeometryData object, since there are many optional components). The type of curves to be used
                are also defined here, although the only supported option currently is a bezier patch. You can also
                specify a global subdivision level here if you like, although it is recommended that the parameter
                is left as AUTO_LEVEL, which means the system decides how much subdivision is required (based on the
                curvature of the surface)
            @param
                meshName The name to give to the mesh which is created, for MeshManager registration purposes (this allows you
                to create entities from this at a later time if you wish)
            @param
                controlPoints A set of vertex data which defines control points of the curves rather than actual vertices.
            @param
                width Specifies the width of the patch in control points. It is assumed that controlPoints.numVertices / width is the height.
            @param
                pType The type of surface - currently only PST_BEZIER is supported
            @param
                subdivisionLevel If you want to manually set the level of subdivision, do it here, otherwise let the system decide.
            @param
                visibleSide Determines which side of the patch (or both) triangles are generated for.
        */
        void defineSurface(String meshName, const GeometryData& controlPoints, int width,
            PatchSurfaceType pType = PST_BEZIER, int subdivisionLevel = AUTO_LEVEL,
            VisibleSide visibleSide = VS_FRONT);

        /** Tells the system to build the mesh relating to the surface.
            @remarks
                Calling getMesh will automatically do this if the mesh is out of date, however if you wish to force this
                to happen during a non-critical period then call this.
        */
        void build(void);

        /** Alters the level of subdivision for this surface.
            @remarks
                This method is provided to alter the subdivision level of the surface at some point after defineSurface
                has been called. Overrides any automatic determination of the subdivision level. Takes effect next time
                PatchSurface::build or PatchSurface::getMesh is called.
            @see
                PatchSurface::build, PatchSurface::getMesh
        */
        void setSubdivisionLevel(int level = AUTO_LEVEL);

        /** Retrieves a pointer to the mesh which has been built for this surface.
            @remarks
                If PatchSurface::build has not already been called, this method calls it for you to ensure the
                mesh is up to date.
            @note
                The mesh geometry data will be produced in the same format (vertex components, data strides etc)
                as the control point data passed in using PatchSurface::defineSurface.
            @see
                PatchSurface::defineSurface
        */
        Mesh* getMesh(void);
    protected:
        /// MeshManager registered name
        String mMeshName;
        /// Control points
        GeometryData mCtlPointData;
        /// The mesh
        Mesh* mMesh;
        /// Flag indicating build required
        bool mNeedsBuild;
        /// Type of surface
        PatchSurfaceType mType;
        /// Width in control points
        int mCtlWidth;
        /// Height in control points
        int mCtlHeight;
        /// U-direction subdivision level
        int mULevel;
        /// V-direction subdivision level
        int mVLevel;
        /// Width of the subdivided mesh
        int mMeshWidth;
        /// Height of the subdivided mesh
        int mMeshHeight;
        /// Which side is visible
        VisibleSide mVSide;

        bool mSharedVertexData;
        bool mMemoryAllocated;

        std::vector<Vector3> mVecCtlPoints;

        // Steps for use in buffers
        int mBufPosStep, mBufNormStep, mBufColourStep, mBufTexCoordStep[OGRE_MAX_TEXTURE_COORD_SETS] ;

        /** Internal method for finding the subdivision level given 3 control points.
        */
        int findLevel( Vector3& a, Vector3& b, Vector3& c);

        void allocateMemory(void);
        void deallocateMemory(void);
        void distributeControlPoints(void);
        void subdivideCurve(int startIdx, int stepSize, int numSteps, int iterations);
        void interpolateVertexData(int leftIndex, int rightIndex, int destIndex);
        void makeTriangles(void);



    };


} // namespace

#endif
