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
#ifndef __HardwareVertexBuffer__
#define __HardwareVertexBuffer__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreHardwareBuffer.h"

namespace Ogre {
    /** Specialisation of HardwareBuffer for a vertex buffer. */
    class _OgreExport HardwareVertexBuffer : public HardwareBuffer
    {
	    public:
		    static const unsigned short MAX_BLEND_WEIGHTS = 4;
		    static const unsigned short MAX_TEXTURE_COORD_SETS = 7;
		    static const unsigned short MAX_TEXTURE_COORD_DIMENSIONS = 3;
		    /// Vertex content options, defintely not mutually exclusive but note that the order of the vertex components is FIXED
		    enum VertexFlags {
			    /// Position, 3 reals per vertex
			    VF_POSITION = 0x1,
			    /// Normal, 3 reals per vertex
			    VF_NORMAL = 0x2,
			    /// Blending weights, 1-4 reals per vertex determined by VertexFormat.numBlendWeights
			    VF_BLEND_WEIGHTS = 0x4,
			    /// Diffuse colours, 4 byte RGBA or ARGB per vertex (rendersystem determines format)
			    VF_DIFFUSE = 0x8,
			    /// Specular colours, 4 byte RGBA or ARGB per vertex (rendersystem determines format)
			    VF_SPECULAR = 0x10,
			    /// Texture coordinates, 1-7 entries of 1, 2 or 3 reals per vertex, as determined by VertexFormat.numTexCoords and VertexFormat.numTexCoordDimensions[]
			    VF_TEXTURE_COORDINATES = 0x20
		    };

		    /// Full vertex format declaration, fills in the remaining information required e.g. multiples
		    struct VertexFormat 
		    {
			    /// flags indicating the content
			    VertexFlags flags;
			    /// Number of blend weights per vertex, only applicable if (flags & VF_BLEND_WEIGHTS)
			    unsigned short numBlendWeights;
			    /// Number of texture coord sets per vertex, only applicable if (flags & VF_TEXTURE_COORDINATES)
			    unsigned short numTextureCoords;
			    /// Number of texture coord dimensions for each set, only applicable if (flags & VF_TEXTURE_COORDINATES) and only valid up to index (numTextureCoords - 1)
			    unsigned short numTextureCoordDimensions[MAX_TEXTURE_COORD_SETS];
    			
		    };

	    protected:
		    VertexFormat mFormat;
		    size_t mNumVertices;
            size_t mVertexSize;

            /// Internal method for calculating the size of a single vertex from the format
            size_t calcVertexSize(const VertexFormat& format);
	    public:
		    /// Should be called by HardwareBufferManager
		    HardwareVertexBuffer(const VertexFormat& format, size_t numVertices, 
                HardwareBuffer::Usage usage);
            /// Gets the size in bytes of a single vertex in this buffer
            size_t getVertexSize(void) { return mVertexSize; }
            /// Get the number of vertices in this buffer
            size_t getNumVertices(void) { return mNumVertices; }
            /// Getthe format of the vertices in this buffer
            const VertexFormat& getVertexFormat(void) { return mFormat; }
    		
		    // NB subclasses should override lock, unlock, readData, writeData
    	
    };
}
#endif

