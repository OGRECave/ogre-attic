/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef __CgProgram_H__
#define __CgProgram_H__

#include "OgreCgPrerequisites.h"
#include "OgreHighLevelGpuProgram.h"

namespace Ogre {
    class CgProgram : public HighLevelGpuProgram
    {
    protected:
        /// The CG context to use, passed in by factory
        CGcontext mCgContext;
        /// Program handle
        CGprogram mCgProgram;
        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void);
        /** Internal method for creating an appropriate low-level program from this
        high-level program, must be implemented by subclasses. */
        void createLowLevelImpl(void);
        /// Internal unload implementation, must be implemented by subclasses
        void unloadImpl(void);
        /// Populate the passed parameters with name->index map, must be overridden
        void populateParameterNames(GpuProgramParametersSharedPtr params);

        StringVector mProfiles;
        String mEntryPoint;
        String mSelectedProfile;
        CGprofile mSelectedCgProfile;
        /// Internal method which works out which profile to use for this program
        void selectProfile();


    public:
        CgProgram(const String& name, GpuProgramType gpType, const String& language, 
            CGcontext context);
        ~CgProgram();

    };
}

#endif
