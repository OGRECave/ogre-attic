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

#ifndef __SHADOWVOLUMEEXTRUDEPROGRAM_H__
#define __SHADOWVOLUMEEXTRUDEPROGRAM_H__

namespace Ogre {
    /** Static class containing source for vertex programs for extruding shadow volumes
    @remarks
        This exists so we don't have to be dependent on an external media files.
        Assembler is used so we don't have to rely on particular plugins.
        The assembler contents of this file were generated from the following Cg:
    @code
        // Point light shadow volume extrude
        void shadowVolumeExtrudePointLight_vp (
            float4 position			: POSITION,
            float  wcoord			: TEXCOORD0,

            out float4 oPosition	: POSITION,

            uniform float4x4 worldViewProjMatrix,
            uniform float4   lightPos // homogenous, object space
            )
        {
            // extrusion in object space
            // vertex unmodified if w==1, extruded if w==0
            float4 newpos = 
                (wcoord.xxxx * lightPos) + 
                float4(position.xyz - lightPos.xyz, 0);

            oPosition = mul(worldViewProjMatrix, newpos);

        }

        // Directional light extrude
        void shadowVolumeExtrudeDirLight_vp (
            float4 position			: POSITION,
            float  wcoord			: TEXCOORD0,

            out float4 oPosition	: POSITION,

            uniform float4x4 worldViewProjMatrix,
            uniform float4   lightPos // homogenous, object space
            )
        {
            // extrusion in object space
            // vertex unmodified if w==1, extruded if w==0
            float4 newpos = 
                (wcoord.xxxx * (position + lightPos)) - lightPos;

            oPosition = mul(worldViewProjMatrix, newpos);

        }
    @endcode
    */
    class _OgreExport ShadowVolumeExtrudeProgram
    {
    private:
        static String mPointArbvp1;
        static String mPointVs_1_1;
        static String mDirArbvp1;
        static String mDirVs_1_1;
        // same as above, except the color is set to 1 to enable debug volumes to be seen
        static String mPointArbvp1Debug;
        static String mPointVs_1_1Debug;
        static String mDirArbvp1Debug;
        static String mDirVs_1_1Debug;
    public:
        /// Get extruder program source for point lights, compatible with arbvp1
        static const String& getPointLightExtruderArbvp1(void) { return mPointArbvp1; }
        /// Get extruder program source for point lights, compatible with vs_1_1
        static const String& getPointLightExtruderVs_1_1(void) { return mPointVs_1_1; }
        /// Get extruder program source for directional lights, compatible with arbvp1
        static const String& getDirectionalLightExtruderArbvp1(void) { return mDirArbvp1; }
        /// Get extruder program source for directional lights, compatible with vs_1_1
        static const String& getDirectionalLightExtruderVs_1_1(void) { return mDirVs_1_1; }

        /// Get extruder program source for debug point lights, compatible with arbvp1
        static const String& getPointLightExtruderArbvp1Debug(void) { return mPointArbvp1Debug; }
        /// Get extruder program source for debug point lights, compatible with vs_1_1
        static const String& getPointLightExtruderVs_1_1Debug(void) { return mPointVs_1_1Debug; }
        /// Get extruder program source for debug directional lights, compatible with arbvp1
        static const String& getDirectionalLightExtruderArbvp1Debug(void) { return mDirArbvp1Debug; }
        /// Get extruder program source for debug directional lights, compatible with vs_1_1
        static const String& getDirectionalLightExtruderVs_1_1Debug(void) { return mDirVs_1_1Debug; }

    };
}
#endif 
