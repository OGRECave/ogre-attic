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
#ifndef _Material_H__
#define _Material_H__

#include "OgrePrerequisites.h"

#include "OgreColourValue.h"
#include "OgreException.h"
#include "OgreTextureManager.h"
#include "OgreBlendMode.h"
#include "OgreRoot.h"
#include "OgreCommon.h"
#include "OgreMatrix4.h"
#include "OgreGpuProgram.h"


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

    protected:

        /// Default material settings - set up by SceneManager
        static Material* mDefaultSettings;

        /** Numerical handle (constant).
        @remarks
        This is assigned automatically, and is useful where small system 
        indexes to materials are needed.
        */
        int mHandle;    


        /** Internal method which sets the material up from the default settings.
        */
        void applyDefaults(void);

        /** Method for retrieving next handle (internal only) */
        void assignNextHandle(void);

        typedef std::vector<Technique*> Techniques;
        Techniques mTechniques;
        Techniques mSupportedTechniques;
        /// Does this material require compilation?
        bool mCompilationRequired;

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

        /** Assignment operator to allow easy copying between materials.
        */
        Material& operator=( const Material& rhs );

        /** Gets the material's name (note - not a texture name).
        */
        const String& getName(void) const;

        /** Returns the numerical handle for this material, if a more compact refence is required.
        @note
        This handle will only be assigned after adding the material to the SceneManager.
        Before this time it will be zero.
        */
        int getHandle(void) const;



        /** Determines if the material has any transparency with the rest of the scene (derived from 
            whether any Techniques say they involve transparency).
        */
        bool isTransparent(void) const;

        /** Creates a new Technique for this Material.
        @remarks
            A Technique is a single way of rendering geometry in order to achieve the effect
            you are intending in a material. There are many reason why you would want more than
            one - the main one being to handle variable graphics card abilities; you might have
            one technique which is impressive but only runs on 4th-generation graphics cards, 
            for example. In this case you will want to create at least one fallback Technique.
            OGRE will work out which Techniques a card can support and pick the best one.
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
        /** Gets an iterator over the Techniques which are supported by the current card. */
        TechniqueIterator getSupportedTechniqueIterator(void);

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
        Material* clone(const String& newName);

        /** Copies the details of this material into another, preserving the target's handle and name
        (unlike operator=) but copying everything else.
        @param mat Pointer to material which will receive this material's settings.
        */
        void copyDetailsTo(Material* mat);

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


    };

} //namespace 

#endif
