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
#ifndef __Common_H__
#define __Common_H__
// Common stuff

namespace Ogre {

	enum ResourceType
    {
        /// All resource types
        RESTYPE_ALL,
        /// Textures
        RESTYPE_TEXTURES,
        /// Models
        RESTYPE_MODELS,
    };

    /** Comparison functions used for the depth/stencil buffer operations and 
		others. */
    enum CompareFunction
    {
        CMPF_ALWAYS_FAIL,
        CMPF_ALWAYS_PASS,
        CMPF_LESS,
        CMPF_LESS_EQUAL,
        CMPF_EQUAL,
        CMPF_NOT_EQUAL,
        CMPF_GREATER_EQUAL,
        CMPF_GREATER
    };

    /** Texture filtering defining the different minification and magnification 
		supersampling modes. */
    enum TextureFilterOptions
    {
        TFO_NONE,
        TFO_BILINEAR,
        TFO_TRILINEAR
    };

    /** Light shading modes. */
    enum ShadeOptions
    {
        SO_FLAT,
        SO_GOURAUD,
        SO_PHONG
    };

    /** Fog modes. */
    enum FogMode
    {
        /// No fog. Duh.
        FOG_NONE,
        /// Fog density increases  exponentially from the camera (fog = 1/e^(distance * density))
        FOG_EXP,
        /// Fog density increases at the square of FOG_EXP, i.e. even quicker (fog = 1/e^(distance * density)^2)
        FOG_EXP2,
        /// Fog density increases linearly between the start and end distances
        FOG_LINEAR
    };

    /** Hardware culling modes based on vertex winding.
        This setting applies to how the hardware API culls triangles it is sent. */
    enum CullingMode
    {
        /// Hardware never culls triangles and renders everything it receives.
        CULL_NONE = 1,
        /// Hardware culls triangles whose vertices are listed clockwise in the view (default).
        CULL_CLOCKWISE = 2,
        /// Hardware culls triangles whose vertices are listed anticlockwise in the view.
        CULL_ANTICLOCKWISE = 3
    };

    /** Manual culling modes based on vertex normals.
        This setting applies to how the software culls triangles before sending them to the 
		hardware API. This culling mode is used by scene managers which choose to implement it -
		normally those which deal with large amounts of fixed world geometry which is often 
		planar (software culling movable variable geometry is expensive). */
    enum ManualCullingMode
    {
        /// No culling so everything is sent to the hardware.
        MANUAL_CULL_NONE = 1,
        /// Cull triangles whose normal is pointing away from the camera (default).
        MANUAL_CULL_BACK = 2,
        /// Cull triangles whose normal is pointing towards the camera.
        MANUAL_CULL_FRONT = 3
    };

    /** Enumerates the wave types usable with the Ogre engine. */
    enum WaveformType
    {
        /// Standard sine wave which smoothly changes from low to high and back again.
        WFT_SINE,
        /// An angular wave with a constant increase / decrease speed with pointed peaks.
        WFT_TRIANGLE,
        /// Half of the time is spent at the min, half at the max with instant transition between.
        WFT_SQUARE,
        /// Gradual steady increase from min to max over the period with an instant return to min at the end.
        WFT_SAWTOOTH,
        /// Gradual steady decrease from max to min over the period, with an instant return to max at the end.
        WFT_INVERSE_SAWTOOTH
    };

    /** The broad type of detail for rendering. */
    enum SceneDetailLevel
    {
		/// Only points are rendered.
        SDL_POINTS,
		/// Wireframe models are rendered.
        SDL_WIREFRAME,
		/// Solid polygons are rendered.
        SDL_SOLID
    };

	/** The pixel format used for textures. */
	enum PixelFormat
	{
		/// Unknown pixel format.
		PF_UNKNOWN,
		/// 8-bit pixel format, all bits luminace.
		PF_L8,
		/// 8-bit pixel format, all bits alpha.
		PF_A8,
		/// 8-bit pixel format, 4 bits alpha, 4 bits luminace.
		PF_A4L4,
		/// 8-bit pixel format, 4 bits luminace, 4 bits alpha.
		PF_L4A4,
		/// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
		PF_R5G6B5,
		/// 16-bit pixel format, 5 bits blue, 6 bits green, 5 bits red.
		PF_B5G6R5,
		/// 16-bit pixel format, 4 bits for alpha, red, green and blue.
		PF_A4R4G4B4,
		/// 16-bit pixel format, 4 bits for blue, green, red and alpha.
		PF_B4G4R4A4,
		/// 24-bit pixel format, 8 bits for red, green and blue.
		PF_R8G8B8,
		/// 24-bit pixel format, 8 bits for blue, green and red.
		PF_B8R8G8,
		/// 32-bit pixel format, 8 bits for alpha, red, green and blue.
		PF_A8R8G8B8,
		/// 32-bit pixel format, 8 bits for blue, green, red and alpha.
		PF_B8G8R8A8,
		/// 32-bit pixel format, 2 bits for alpha, 10 bits for red, green and blue.
		PF_A2R10G10B10,
		/// 32-bit pixel format, 10 bits for blue, green and red, 2 bits for alpha.
		PF_B10G10R10A2
	};

#ifdef OGRE_NONCLIENT_BUILD
	PixelFormat ilFormat2OgreFormat( int ImageFormat, int BytesPerPixel );
	std::pair< int, int > OgreFormat2ilFormat( PixelFormat format );
#endif
}

#endif
