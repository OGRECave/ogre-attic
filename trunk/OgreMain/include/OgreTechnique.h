/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
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
#ifndef __Technique_H__
#define __Technique_H__

#include "OgrePrerequisites.h"
#include "OgreIteratorWrappers.h"
#include "OgreBlendMode.h"
#include "OgreCommon.h"

namespace Ogre {
    /** Class representing an approach to rendering this particular Material. 
    @remarks
        Ogre will attempt to use the best technique supported by the active hardware, 
        unless you specifically request a lower detail technique (say for distant
        rendering).
    */
    class _OgreExport Technique
    {
    protected:
        typedef std::vector<Pass*> Passes;
        Passes mPasses;
        Material* mParent;
        bool mIsSupported;
    public:
        /// Constructor
        Technique(Material* parent);
        /// Copy constructor
        Technique(Material* parent, const Technique& oth);
        ~Technique();
        /** Indicates if this technique is supported by the current graphics card.
        @remarks
            This will only be correct after the Technique has been compiled, which is
            usually done from Material::compile.
        */
        bool isSupported(void);
        /** Internal compilation method; see Material::compile. */
        void _compile(bool autoManageTextureUnits);

        /** Creates a new Pass for this Technique.
        @remarks
            A Pass is a single rendering pass, ie a single draw of the given material.
            Note that if you create a non-programmable pass, during compilation of the
            material the pass may be split into multiple passes if the graphics card cannot
            handle the number of texture units requested. For programmable passes, however, 
            the number of passes you create will never be altered, so you have to make sure 
            that you create an alternative fallback Technique for if a card does not have 
            enough facilities for what you're asking for.
        */
        Pass* createPass(bool programmable);
        /** Retrieves the Pass with the given index. */
        Pass* getPass(unsigned short index);
        /** Retrieves the number of passes. */
        unsigned short getNumPasses(void);
        /** Removes the Pass with the given index. */
        void removePass(unsigned short index);
        /** Removes all Passes from this Technique. */
        void removeAllPasses(void);
        typedef VectorIterator<Passes> PassIterator;
        /** Gets an iterator over the passes in this Technique. */
        const PassIterator getPassIterator(void);
        /// Gets the parent Material
        Material* getParent(void) { return mParent; }

        /** Overloaded operator to copy on Technique to another. */
        Technique& operator=(const Technique& rhs);

		/** Returns true if this Technique involves transparency. 
		@remarks
			This basically boils down to whether the first pass
			has a scene blending factor. Even if the other passes 
			do not, the base colour, including parts of the original 
			scene, may be used for blending, therefore we have to treat
			the whole Technique as transparent.
		*/
		bool isTransparent(void);

        /** Internal load method, derived from call to Material::load. */
        void _load(void);
        /** Internal unload method, derived from call to Material::unload. */
        void _unload(void);

        // Is this loaded?
        bool isLoaded(void);

        /** Tells the technique that it needs recompilation. */
        void _notifyNeedsRecompile(void);


        // -------------------------------------------------------------------------------
        // The following methods are to make migration from previous versions simpler
        // and to make code easier to write when dealing with simple materials
        // They set the properties which have been moved to Pass for all Techniques and all Passes

        /** Sets the ambient colour reflectance properties for every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setAmbient
        */
        void setAmbient(Real red, Real green, Real blue);

        /** Sets the ambient colour reflectance properties for every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setAmbient
        */
        void setAmbient(const ColourValue& ambient);

        /** Sets the diffuse colour reflectance properties of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setDiffuse
        */
        void setDiffuse(Real red, Real green, Real blue);

        /** Sets the diffuse colour reflectance properties of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setDiffuse
        */
        void setDiffuse(const ColourValue& diffuse);

        /** Sets the specular colour reflectance properties of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setSpecular
        */
        void setSpecular(Real red, Real green, Real blue);

        /** Sets the specular colour reflectance properties of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setSpecular
        */
        void setSpecular(const ColourValue& specular);

        /** Sets the shininess properties of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setShininess
        */
        void setShininess(Real val);

        /** Sets the amount of self-illumination of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setSelfIllumination
        */
        void setSelfIllumination(Real red, Real green, Real blue);

        /** Sets the amount of self-illumination of every Pass in every Technique.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setSelfIllumination
        */
        void setSelfIllumination(const ColourValue& selfIllum);

		/** Sets whether or not each Pass renders with depth-buffer checking on or not.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setDepthCheckEnabled
        */
        void setDepthCheckEnabled(bool enabled);

        /** Sets whether or not each Pass renders with depth-buffer writing on or not.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setDepthWriteEnabled
        */
        void setDepthWriteEnabled(bool enabled);

        /** Sets the function used to compare depth values when depth checking is on.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setDepthFunction
        */
        void setDepthFunction( CompareFunction func );

		/** Sets whether or not colour buffer writing is enabled for each Pass.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setColourWriteEnabled
		*/
		void setColourWriteEnabled(bool enabled);

        /** Sets the culling mode for each pass  based on the 'vertex winding'.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setCullingMode
        */
        void setCullingMode( CullingMode mode );

        /** Sets the manual culling mode, performed by CPU rather than hardware.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setManualCullingMode
        */
        void setManualCullingMode( ManualCullingMode mode );

        /** Sets whether or not dynamic lighting is enabled for every Pass.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setLightingEnabled
        */
        void setLightingEnabled(bool enabled);

        /** Sets the type of light shading required
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setShadingMode
        */
        void setShadingMode( ShadeOptions mode );

        /** Sets the fogging mode applied to each pass.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setFog
        */
        void setFog(
            bool overrideScene,
            FogMode mode = FOG_NONE,
            const ColourValue& colour = ColourValue::White,
            Real expDensity = 0.001, Real linearStart = 0.0, Real linearEnd = 1.0 );

        /** Sets the depth bias to be used for each Pass.
        @note
            This property actually exists on the Pass class. For simplicity, this method allows 
            you to set these properties for every current Pass within this Technique. If 
            you need more precision, retrieve the Pass instance and set the
            property there.
        @see Pass::setDepthBias
        */
        void setDepthBias(ushort bias);



    };


}
#endif
