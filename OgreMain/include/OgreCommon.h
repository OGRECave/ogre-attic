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
#ifndef __Common_H__
#define __Common_H__
// Common stuff

namespace Ogre {

    /// Comparison functions
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

    /** Texture filtering ie minification and magnification supersampling modes. */
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
        This setting applies to how the hardware API culls triangles it is sent.
    */
    enum CullingMode
    {
        /// Hardware never culls triangles and renders everything it is sent
        CULL_NONE = 1,
        /// Hardware culls triangles whose vertices are listed clockwise in the view (default)
        CULL_CLOCKWISE = 2,
        /// Hardware culls triangles whose vertices are listed anticlockwise in the view
        CULL_ANTICLOCKWISE = 3
    };
    /** Manual culling modes based on vertex normals.
        This setting applies to how the software culls triangles before sending them to the hardware API.
        Only used by SceneManager's which choose to use it, normally those which deal with large amounts of
        fixed world geometry which is often planar (software culling movable variable geometry is expensive)
    */
    enum ManualCullingMode
    {
        /// No software culling - all triangles are sent to the hardware
        MANUAL_CULL_NONE = 1,
        /// Cull triangles whose normal is pointing away from the camera (default)
        MANUAL_CULL_BACK = 2,
        /// Cull triangles whose normal is pointing towards the camera
        MANUAL_CULL_FRONT = 3
    };
    /** Enumerates the wave types usable with the Ogre engine. */
    enum WaveformType
    {
        /** Standard sine wave which smoothly changes from low to high and back again. */
        WFT_SINE,
        /** An angular wave with a constant increase / decrease speed with pointed peaks. */
        WFT_TRIANGLE,
        /** Half of the time is spent at the min, half at the max with instant transition between. */
        WFT_SQUARE,
        /** Gradual steady increase from min to max over the period with an instant return to min at the end. */
        WFT_SAWTOOTH,
        /** Gradual steady decrease from max to min over the period, with an instant return to max at the end. */
        WFT_INVERSE_SAWTOOTH
    };




}

#endif
