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
#ifndef __GeometryData_H__
#define __GeometryData_H__

#include "OgrePrerequisites.h"
#include "OgreRenderOperation.h"


namespace Ogre {

    /** Common structure containing info about geometry.
    */
    struct GeometryData {
        /// Count of the number of vertices contained herein.
        unsigned short numVertices;
        /// If true, vertex normals are present in the data.
        bool hasNormals;
        /// Number of texture coordinates sets present in the data.
        unsigned short numTexCoords;
        /// Number of texture dimensions in each set of texture coords.
        unsigned short numTexCoordDimensions[OGRE_MAX_TEXTURE_COORD_SETS];
        /// If true, vertex colours are present in the data.
        bool hasColours;

        /// Position (x,y,z) components of shared vertices.
        Real* pVertices;
        /// Texture coordinate components of shared vertices.
        Real* pTexCoords[OGRE_MAX_TEXTURE_COORD_SETS];
        /// Vertex normal components of shared vertices.
        Real* pNormals;
        /// Vertex colour components of shared vertices - RGBA packed.
        unsigned long* pColours;

        /// The 'Stride' between sets of vertex data. 0 indicates data is packed with no gaps.
        unsigned short vertexStride;
        /// The 'Stride' between sets of normal data. 0 indicates data is packed with no gaps.
        unsigned short normalStride;
        /// The 'Stride' between each set of texture data. 0 indicates data is packed with no gaps.
        unsigned short texCoordStride[OGRE_MAX_TEXTURE_COORD_SETS];
        /// The 'Stride' between sets of diffuse colour data. 0 indicates data is packed with no gaps.
        unsigned short colourStride;

        /// The number of vertex blending weights per vertex
        unsigned short numBlendWeightsPerVertex;

        /** Optional pointer to a list of vertex blending details, organised in vertex order. 
            The number of weights per vertex is recorded in numBlendWeightsPerVertex - there must
            be this many for every vertex: set the weight to 0 for those vertices that don't 
            use all the entries (if some vertices have more than others)
        */
        RenderOperation::VertexBlendData* pBlendingWeights;


    };

    /** Helper struct when dealing with buffers.
    */
    typedef struct _GeomVector {
        Real x;
        Real y;
        Real z;
    } GeomVertexPosition, GeomVertexNormal;

    /** Helper struct when dealing with buffers.
    */
    struct GeomTexCoord2D {
        Real u;
        Real v;
    };
}// namespace

#endif
