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
#ifndef __GpuProgramUsage_H__
#define __GpuProgramUsage_H__

#include "OgrePrerequisites.h"
#include "OgreGpuProgram.h"


namespace Ogre 
{
    /** This class makes the usage of a vertex and fragment programs (low-level or high-level), 
        with a given set of parameters, explicit.
    @remarks
        Using a vertex or fragment program can get fairly complex; besides the fairly rudimentary
        process of binding a program to the GPU for rendering, managing usage has few
        complications, such as:
        <ul>
        <li>Programs can be high level (e.g. Cg, RenderMonkey) or low level (assembler). Using
        either should be relatively seamless, although high-level programs give you the advantage
        of being able to use named parameters, instead of just indexed registers</li>
        <li>Programs and parameters can be shared between multiple usages, in order to save
        memory</li>
        <li>When you define a user of a program, such as a material, you often want to be able to
        set up the definition but not load / compile / assemble the program at that stage, because
        it is not needed just yet. The program should be loaded when it is first needed, or
        earlier if specifically requested. The program may not be defined at this time, you
        may want to have scripts that can set up the definitions independent of the order in which
        those scripts are loaded.</li>
        </ul>
        This class packages up those details so you don't have to worry about them. For example,
        this class lets you define a high-level program and set up the parameters for it, without
        having loaded the program (which you normally could not do). When the program is loaded and
        compiled, this class will then validate the parameters you supplied earlier and turn them
        into runtime parameters.
    @par
        Just incase it wasn't clear from the above, this class provides linkage to both 
        GpuProgram and HighLevelGpuProgram, despite its name.
    */
    class _OgreExport GpuProgramUsage
    {
    protected:
        GpuProgramType mType;
        String mProgramName;
        bool mDeferValidation;

        // The program link
        GpuProgram* mProgram;
        //HighLevelGpuProgram* mHighLevelProgram;
        // TODO: add storage of named parameters pending setting on HLGP

        /// Low-level program parameters
        GpuProgramParametersSharedPtr mLowLevelParams;


    public:
        /** Default constructor.
        @param gptype The type of program to link to
        @param programName The name of the program to use. Note that at this stage the program is
            <strong>not</strong> looked up, so there is really no validation of this parameter
            until you call validate() or set the last parameter of this method to true.
        @param validateImmediately Set this to true if you want to immediately check the
            program name and any named parameters that you set.
        */
        GpuProgramUsage(GpuProgramType gptype, const String& programName = "", 
            bool validateImmediately = false);

		/** Copy constructor */
		GpuProgramUsage(const GpuProgramUsage& rhs);

        /** Gets the type of program we're trying to link to. */
        GpuProgramType getType(void) const { return mType; }
        /** Gets the name of the program we're trying to link to. */
        const String& getProgramName(void) const{ return mProgramName; }

		/** Sets the name of the program to use. */
		void setProgramName(const String& name);
        /** Turns on validation for this class, if it was left disabled on initial creation.
        @remarks
            Validation of the program name, and the named parameters which are used for
            high-level programs can be deferred in order to relax the ordering of using this
            class and defining the programs to which it refers. Eventually, however, these
            things do have to be checked, and this method turns on that checking permanently.
        */
        void enableValidation(void);

        /** Sets the low-level parameters that should be used; because parameters can be
            shared between multiple usages for efficiency, this method is here for you
            to register externally created parameter objects.
        @remarks
            For high-level programs you will want to use the alternative named parameter object
            version.
        */
        void setParameters(GpuProgramParametersSharedPtr params);
        /** Gets the low-level parameters being used here. */
        GpuProgramParametersSharedPtr getParameters(void);

		/** Gets the program this usage is linked to; only available after the usage has been
			validated either via enableValidation or by enabling validation on construction. */
		GpuProgram* getProgram(void);

        /// Load this usage (and ensure program is loaded)
        void _load(void);
        /// Unload this usage 
        void _unload(void);



        /* TODO - high-level params */

	protected:
		/// Internal validation function - checks the name of the program (and links)
		void validateName(void);
		/// Internal validation function - checks the named parameters
		void validateNamedParameters(void);



    };
}
#endif
