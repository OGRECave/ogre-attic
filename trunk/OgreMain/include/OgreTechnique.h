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


    };


}
#endif
