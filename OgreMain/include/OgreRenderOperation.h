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
#ifndef _RenderOperation_H__
#define _RenderOperation_H__

#include "OgrePrerequisites.h"
#include "OgreColourValue.h"

namespace Ogre {
    /** Class describing a rendering operation like how to draw a triangle.
        @remarks
            This class encapsulates the description of a rendering operation,
            including the type (single triangle, list of triangles, triangle strip,
            etc), the vertex format (e.g. whether vertex colours, texture
            co-ords are present).
        @par
            Render options like filtering, z-buffer options etc
            are not provided here, but via, various 'set' methods of the
            RenderSystem class, since render state changes are typically expensive, so the
            assumption is that render states will remain fairly constant
            between rendering operations if possible.
        @par
            Notes on indexed triangle lists: Vertices are not duplicated in the
            rendering pipeline where they are shared by multiple
            triangles. Be aware, however, that when sharing vertices
            ALL aspects are shared, including the vertex normal and
            texture coordinates which is not always appropriate
            (e.g. if you want a 'hard' edge to a join, or at the
            seam of a texture). You should pre-duplicate the vertices
            where this is a problem with the separate texture and
            normal values. The Ogre library does this automatically
            with imported models.
        @par
            In all cases, remember that Ogre uses a right-handed world co-ordinate
            system, (hold your right-hand flat in the X-axis direction and
            curl your fingers toward the Y-Axis: your thumb points in the
            positive Z-axis direction). Note that in screen space this gets
            flipped so higher Z is into the screen. This is a standard
            system as used by most maths texts and modelling tools (and OpenGL, but not D3D).
            In addition, the FRONT of triangles is deemed to be the side where
            the vertices are in counter-clockwise in RH co-ords as viewed from
            the camera.
     */
    class RenderOperation {
    public:
        enum OpType {
            OT_POINT_LIST,
            OT_LINE_LIST,
            OT_LINE_STRIP,
            OT_TRIANGLE_LIST,
            OT_TRIANGLE_STRIP,
            OT_TRIANGLE_FAN
        };
        /** Vertex options - which elements to include.
            @remarks
                Vertices must include their elements in the following order:
                position, normal, texture co-ords (1-3 dimensions, 1-4 sets),
                diffuse colour, specular colour. Only position is mandatory,
                although at least ONE OF the following should be specified,
                even if ambient light on flat coloured objects only is being used.
         */
        enum VertexOptions {
            /// vertex normals included (for lighting)
            VO_NORMALS = 1,
            /// at least one set of texture coords (exact number specified in class)
            VO_TEXTURE_COORDS = 2,
            /// Vertex colours - diffuse
            VO_DIFFUSE_COLOURS = 4,
            /// Vertex colours - specular
            VO_SPECULAR_COLOURS = 8,
            /// Vertex blend weights
            VO_BLEND_WEIGHTS = 16
        };

        /** Vertex blend info */
        struct VertexBlendData
        {
            unsigned short matrixIndex;
            Real blendWeight;
        };

        // true to use pIndexes to reference individual lines/triangles rather than embed. Allows vertex reuse.
        bool useIndexes;

        /// Number of vertices (applies to all components)
        unsigned int numVertices;

        /// The number of vertex blending weights per vertex, only applicable if VO_BLEND_WEIGHTS supplied
        unsigned short numBlendWeightsPerVertex;

        // No memory allocation here,
        // assumed that all pointers are pointing
        // elsewhere e.g. model class data

        /** Pointer to list of vertices (float {x, y z} * numVertices).
            @remarks
                If useIndexes is false each group of 3 vertices describes a face (anticlockwise winding) in
                trianglelist mode.
        */
        Real* pVertices;

        /// The 'Stride' between sets of vertex data. 0 indicates data is packed with no gaps.
        unsigned short vertexStride;

        /// Optional vertex normals for vertices (float {x, y, z} * numVertices).
        Real* pNormals;

        /// The 'Stride' between sets of normal data. 0 indicates data is packed with no gaps.
        unsigned short normalStride;

        /** Optional texture coordinates for vertices (float {u, [v], [w]} * numVertices).
            @remarks
                There can be up to 8 sets of texture coordinates, and the number of components per
                vertex depends on the number of texture dimensions (2 is most common).
        */
        Real* pTexCoords[OGRE_MAX_TEXTURE_COORD_SETS];

        /// The 'Stride' between each set of texture data. 0 indicates data is packed with no gaps.
        unsigned short texCoordStride[OGRE_MAX_TEXTURE_COORD_SETS];

        /// Number of groups of u,[v],[w].
        int numTextureCoordSets;

        /** Number of dimensions in each corresponding texture coordinate set.
            @note
                There should be 1-4 dimensions on each set.
        */
        int numTextureDimensions[OGRE_MAX_TEXTURE_COORD_SETS];

        /// Optional pointer to a list of diffuse vertex colours (32-bit RGBA * numVertices).
        RGBA* pDiffuseColour;

        /// The 'Stride' between sets of diffuse colour data. 0 indicates data is packed with no gaps.
        unsigned short diffuseStride;

        /// Optional pointer to a list of specular vertex colours (32-bit RGBA * numVertices)
        RGBA* pSpecularColour;

        /// The 'Stride' between sets of specular colour data. 0 indicates data is packed with no gaps.
        unsigned short specularStride;

        /** Optional pointer to a list of vertex blending details, organised in vertex order. 
            The number of weights per vertex is recorded in numBlendWeightsPerVertex - there must
            be this many for every vertex: set the weight to 0 for those vertices that don't 
            use all the entries (if some vertices have more than others)
        */
        VertexBlendData* pBlendingWeights;

        /** Pointer to a list of vertex indexes describing faces (only used if useIndexes is true).
            @note
                Each group of 3 describes a face (anticlockwise winding order).
        */
        unsigned short* pIndexes;

        /// The number of vertex indexes (must be a multiple of 3).
        unsigned int numIndexes;

        /// Flags indicating vertex types
        int vertexOptions;
        /// The type of rendering operation.
        OpType operationType;

        RenderOperation()
        {
            // Initialise all things
            vertexStride = normalStride = diffuseStride = specularStride = 0;
            numBlendWeightsPerVertex = 0;
            for (int i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; ++i)
            {
                texCoordStride[i] = 0;
                pTexCoords[0] = 0;
            }

            pVertices = 0;
            pNormals = 0;
            pDiffuseColour = 0;
            pSpecularColour = 0;
            pBlendingWeights = 0;
        }
    };

    /* Example usage (camera at (0,0,0) pointing down -Z (lookAt(0,0,-300))

        RenderOperation ro;
        float vertexData[9] = {100,   0, -300,
                                   0, 200, -300,
                              -100,   0, -300 };
        float normalData[9] = { 0, 0, 1,
                                0, 0, 1,
                                0, 0, 1};
        ro.operationType = RenderOperation::OT_TRIANGLE_LIST;
        ro.numVertices = 3;
        ro.useIndexes = false;
        ro.vertexOptions = RenderOperation::VO_NORMAL;
        ro.pVertices = vertexData;
        ro.pNormals = normalData;
        mDestRenderSystem->_render(ro);
    */
}



#endif
