/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2003 The OGRE Team
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
#ifndef _Material_H__
#define _Material_H__

#include "OgrePrerequisites.h"

#include "OgreResource.h"
#include "OgreIteratorWrappers.h"
#include "OgreCommon.h"
#include "OgreColourValue.h"
#include "OgreBlendMode.h"


namespace Ogre {

    /** Class encapsulates rendering properties of an object.
    @remarks
    Ogre's material class encapsulates ALL aspects of the visual appearance,
    of an object. It also includes other flags which 
    might not be traditionally thought of as material properties such as 
    culling modes and depth buffer settings, but these affect the 
    appearance of the rendered object and are convenient to attach to the 
    material since it keeps all the settings in one place. This is 
    different to Direct3D which treats a material as just the colour 
    components (diffuse, specular) and not texture maps etc. An Ogre 
    Material can be thought of as equivalent to a 'Shader'.
    @par
    A Material can be rendered in multiple different ways depending on the
    hardware available. You may configure a Material to use high-complexity
    fragment shaders, but these won't work on every card; therefore a Technique
    is an approach to creating the visual effect you are looking for. You are advised
    to create fallback techniques with lower hardware requirements if you decide to
    use advanced features. In addition, you also might want lower-detail techniques
    for distant geometry.
    @par
    Each technique can be made up of multiple passes. A fixed-function pass
    may combine multiple texture layers using multitexrtuing, but Ogre can 
    break that into multiple passes automatically if the active card cannot
    handle that many simultaneous textures. Programmable passes, however, cannot
    be split down automatically, so if the active graphics card cannot handle the
    technique which contains these passes, OGRE will try to find another technique
    which the card can do. If, at the end of the day, the card cannot handle any of the
    techniques which are listed for the material, the engine will render the 
    geometry plain white, which should alert you to the problem.
    @par
    Ogre comes configured with a number of default settings for a newly 
    created material. These can be changed if you wish by retrieving the 
    default material settings through 
    SceneManager::getDefaultMaterialSettings. Any changes you make to the 
    Material returned from this method will apply to any materials created 
    from this point onward.
    */
    class _OgreExport Material : public Resource
    {
        friend class SceneManager;
        friend class MaterialManager;

    public:
        /// distance list used to specify LOD
        typedef std::vector<Real> LodDistanceList;
        typedef ConstVectorIterator<LodDistanceList> LodDistanceIterator;
    protected:

        /// Default material settings - set up by SceneManager
        static Material* mDefaultSettings;

        /** Internal method which sets the material up from the default settings.
        */
        void applyDefaults(void);

        typedef std::vector<Technique*> Techniques;
        Techniques mTechniques;
        Techniques mSupportedTechniques;
        typedef std::map<unsigned short, Technique*> BestTechniqueList;
        BestTechniqueList mBestTechniqueList;

        /// Does this material require compilation?
        bool mCompilationRequired;
        LodDistanceList mLodDistances;
        bool mReceiveShadows;

    public:

        /** Mandatory constructor - you must supply a name for the material.
        @note
        Normally you create materials by calling the relevant SceneManager since that is responsible for
        managing all scene state including materials.
        @param
        name A unique mateiral name
        */
        Material( const String& name);

        /** Default constructor - uses a generated material name.
        */
        Material();

        ~Material();
        /** Assignment operator to allow easy copying between materials.
        */
        Material& operator=( const Material& rhs );

        /** Gets the material's name (note - not a texture name).
        */
        const String& getName(void) const;

        /** Determines if the material has any transparency with the rest of the scene (derived from 
            whether any Techniques say they involve transparency).
        */
        bool isTransparent(void) const;

        /** Sets whether objects using this material will receive shadows.
        @remarks
            This method allows a material to opt out of receiving shadows, if
            it would otherwise do so. Shadows will not be cast on any objects
            unless the scene is set up to support shadows 
            (@see SceneManager::setShadowTechnique), and not all techniques cast
            shadows on all objects. In any case, if you have a need to prevent
            shadows being received by material, this is the method you call to
            do it.
        @note 
            Transparent materials never receive shadows despite this setting. 
            The default is to receive shadows.
        */
        void setReceiveShadows(bool enabled) { mReceiveShadows = enabled; }
        /** Returns whether or not objects using this material will receive shadows. */
        bool getReceiveShadows(void) const { return mReceiveShadows; }

        /** Creates a new Technique for this Material.
        @remarks
            A Technique is a single way of rendering geometry in order to achieve the effect
            you are intending in a material. There are many reason why you would want more than
            one - the main one being to handle variable graphics card abilities; you might have
            one technique which is impressive but only runs on 4th-generation graphics cards, 
            for example. In this case you will want to create at least one fallback Technique.
            OGRE will work out which Techniques a card can support and pick the best one.
        @par
            If multiple Techniques are available, the order in which they are created is 
            important - the engine will consider lower-indexed Techniques to be preferable
            to higher-indexed Techniques, ie when asked for the 'best' technique it will
            return the first one in the technique list which is supported by the hardware.
        */
        Technique* createTechnique(void);
        /** Gets the indexed technique. */
        Technique* getTechnique(unsigned short index);
        /** Removes the technique at the given index. */
        void removeTechnique(unsigned short index);
        /** Removes all the techniques in this Material. */
        void removeAllTechniques(void);
        typedef VectorIterator<Techniques> TechniqueIterator;
        /** Get an iterator over the Techniques in this Material. */
        TechniqueIterator getTechniqueIterator(void);
        /** Gets an iterator over all the Techniques which are supported by the current card. 
        @remarks
            The supported technique list is only available after this material has been compiled,
            which typically happens on loading the material. Therefore, if this method returns
            an empty list, try calling Material::load.
        */
        TechniqueIterator getSupportedTechniqueIterator(void);

        /** Gets the number of levels-of-detail this material has, based on 
            Technique::setLodIndex. 
        @remarks
            Note that this will not be up to date until the material has been compiled.
        */
        unsigned short getNumLodLevels(void) const { 
            return static_cast<unsigned short>(mBestTechniqueList.size()); }

        /** Gets the best supported technique. 
        @remarks
            This method returns the lowest-index supported Technique in this material
            (since lower-indexed Techniques are considered to be better than higher-indexed
            ones).
        @par
            The best supported technique is only available after this material has been compiled,
            which typically happens on loading the material. Therefore, if this method returns
            NULL, try calling Material::load.
		@param lodIndex The material lod index to use
        */
        Technique* getBestTechnique(unsigned short lodIndex = 0);

        /** Overridden from Resource.
        @remarks
        By default, Materials are not loaded, and adding additional textures etc do not cause those
        textures to be loaded. When the 'load' method is called, all textures are loaded (if they
        are not already), GPU programs are created if applicable, and Controllers are instantiated.
        Once a material has been loaded, all changes made to it are immediately loaded too.
        */
        void load(void);

        /** Unloads the material, frees resources etc.
        @see
        Resource
        */
        void unload(void);

        /** Creates a new copy of this material with the same settings but a new name.
        */
        Material* clone(const String& newName) const;

        /** Copies the details of this material into another, preserving the target's handle and name
        (unlike operator=) but copying everything else.
        @param mat Pointer to material which will receive this material's settings.
        */
        void copyDetailsTo(Material* mat) const;

        /** 'Compiles' this Material.
        @remarks
            Compiling a material involves determining which Techniques are supported on the
            card on which OGRE is currently running, and for fixed-function Passes within those
            Techniques, splitting the passes down where they contain more TextureUnitState 
            instances than the curren card has texture units.
        @par
            This process is automatically done when the Material is loaded, but may be
            repeated if you make some procedural changes.
        @param
            autoManageTextureUnits If true, when a fixed function pass has too many TextureUnitState
                entries than the card has texture units, the Pass in question will be split into
                more than one Pass in order to emulate the Pass. If you set this to false and
                this situation arises, an Exception will be thrown.
        */
        void compile(bool autoManageTextureUnits = true);

        // -------------------------------------------------------------------------------
        // The following methods are to make migration from previous versions simpler
        // and to make code easier to write when dealing with simple materials
        // They set the properties which have been moved to Pass for all Techniques and all Passes

        /** Sets the ambient colour reflectance properties for every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setAmbient
        */
        void setAmbient(Real red, Real green, Real blue);

        /** Sets the ambient colour reflectance properties for every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setAmbient
        */
        void setAmbient(const ColourValue& ambient);

        /** Sets the diffuse colour reflectance properties of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDiffuse
        */
        void setDiffuse(Real red, Real green, Real blue);

        /** Sets the diffuse colour reflectance properties of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDiffuse
        */
        void setDiffuse(const ColourValue& diffuse);

        /** Sets the specular colour reflectance properties of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSpecular
        */
        void setSpecular(Real red, Real green, Real blue);

        /** Sets the specular colour reflectance properties of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSpecular
        */
        void setSpecular(const ColourValue& specular);

        /** Sets the shininess properties of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setShininess
        */
        void setShininess(Real val);

        /** Sets the amount of self-illumination of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSelfIllumination
        */
        void setSelfIllumination(Real red, Real green, Real blue);

        /** Sets the amount of self-illumination of every Pass in every Technique.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSelfIllumination
        */
        void setSelfIllumination(const ColourValue& selfIllum);

		/** Sets whether or not each Pass renders with depth-buffer checking on or not.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDepthCheckEnabled
        */
        void setDepthCheckEnabled(bool enabled);

        /** Sets whether or not each Pass renders with depth-buffer writing on or not.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDepthWriteEnabled
        */
        void setDepthWriteEnabled(bool enabled);

        /** Sets the function used to compare depth values when depth checking is on.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDepthFunction
        */
        void setDepthFunction( CompareFunction func );

		/** Sets whether or not colour buffer writing is enabled for each Pass.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setColourWriteEnabled
		*/
		void setColourWriteEnabled(bool enabled);

        /** Sets the culling mode for each pass  based on the 'vertex winding'.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setCullingMode
        */
        void setCullingMode( CullingMode mode );

        /** Sets the manual culling mode, performed by CPU rather than hardware.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setManualCullingMode
        */
        void setManualCullingMode( ManualCullingMode mode );

        /** Sets whether or not dynamic lighting is enabled for every Pass.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setLightingEnabled
        */
        void setLightingEnabled(bool enabled);

        /** Sets the type of light shading required
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setShadingMode
        */
        void setShadingMode( ShadeOptions mode );

        /** Sets the fogging mode applied to each pass.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
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
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setDepthBias
        */
        void setDepthBias(ushort bias);

        /** Set texture filtering for every texture unit in every Technique and Pass
        @note
            This property has been moved to the TextureUnitState class, which is accessible via the 
            Technique and Pass. For simplicity, this method allows you to set these properties for 
            every current TeextureUnitState, If you need more precision, retrieve the Technique, 
            Pass and TextureUnitState instances and set the property there.
        @see TextureUnitState::setTextureFiltering
        */
        void setTextureFiltering(TextureFilterOptions filterType);
        /** Sets the anisotropy level to be used for all textures.
        @note
            This property has been moved to the TextureUnitState class, which is accessible via the 
            Technique and Pass. For simplicity, this method allows you to set these properties for 
            every current TeextureUnitState, If you need more precision, retrieve the Technique, 
            Pass and TextureUnitState instances and set the property there.
        @see TextureUnitState::setTextureAnisotropy
        */
        void setTextureAnisotropy(int maxAniso);

        /** Sets the kind of blending every pass has with the existing contents of the scene.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSceneBlending
        */
        void setSceneBlending( const SceneBlendType sbt );

        /** Allows very fine control of blending every Pass with the existing contents of the scene.
        @note
            This property has been moved to the Pass class, which is accessible via the 
            Technique. For simplicity, this method allows you to set these properties for 
            every current Technique, and for every current Pass within those Techniques. If 
            you need more precision, retrieve the Technique and Pass instances and set the
            property there.
        @see Pass::setSceneBlending
        */
        void setSceneBlending( const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor);


        /** Tells the material that it needs recompilation. */
        void _notifyNeedsRecompile(void);

        /** Sets the distance at which level-of-detail (LOD) levels come into effect.
        @remarks
            You should only use this if you have assigned LOD indexes to the Technique
            instances attached to this Material. If you have done so, you should call this
            method to determine the distance at which the lowe levels of detail kick in.
            The decision about what distance is actually used is a combination of this
            and the LOD bias applied to both the current Camera and the current Entity.
        @param lodDistances A vector of Reals which indicate the distance at which to 
            switch to lower details. They are listed in LOD index order, starting at index
            1 (ie the first level down from the highest level 0, which automatically applies
            from a distance of 0).
        */
        void setLodLevels(const LodDistanceList& lodDistances);
        /** Gets an iterator over the list of distances at which each LOD comes into effect. 
        @remarks
            Note that the iterator returned from this method is not totally anagolous to 
            the one passed in by calling setLodLevels - the list includes a zero
            entry at the start (since the highest LOD starts at distance 0), and
            the other distances are held as their squared value for efficiency.
        */
        LodDistanceIterator getLodDistanceIterator(void) const;

        /** Gets the LOD index to use at the given distance. */
        unsigned short getLodIndex(Real d) const;
        /** Gets the LOD index to use at the given squared distance. */
        unsigned short getLodIndexSquaredDepth(Real squaredDepth) const;

        /** @copydoc Resource::touch
        */
        void touch(void) 
        { 
            if (mCompilationRequired) 
                compile();
            // call superclass
            Resource::touch();
        }


    };

} //namespace 

#endif
