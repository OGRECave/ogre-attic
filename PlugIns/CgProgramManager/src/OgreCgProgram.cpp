/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2003 The OGRE Team
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
#include "OgreCgProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreStringConverter.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    CgProgram::CmdEntryPoint CgProgram::msCmdEntryPoint;
    CgProgram::CmdProfiles CgProgram::msCmdProfiles;
    CgProgram::CmdArgs CgProgram::msCmdArgs;
    //-----------------------------------------------------------------------
    void CgProgram::selectProfile(void)
    {
        mSelectedProfile = "";
        mSelectedCgProfile = CG_PROFILE_UNKNOWN;

        StringVector::iterator i, iend;
        iend = mProfiles.end();
        GpuProgramManager& gpuMgr = GpuProgramManager::getSingleton();
        for (i = mProfiles.begin(); i != iend; ++i)
        {
            if (gpuMgr.isSyntaxSupported(*i))
            {
                mSelectedProfile = *i;
                mSelectedCgProfile = cgGetProfile(mSelectedProfile);
                // Check for errors
                checkForCgError("CgProgram::selectProfile", 
                    "Unable to find CG profile enum for program " + mName + ": ", mCgContext);
                break;
            }
        }
    }
    //-----------------------------------------------------------------------
    void CgProgram::buildArgs(void)
    {
        StringVector args;
        if (!mCompileArgs.empty())
            args = mCompileArgs.split();

        StringVector::const_iterator i;
        if (mSelectedCgProfile == CG_PROFILE_VS_1_1)
        {
            // Need the 'dcls' argument whenever we use this profile
            // otherwise compilation of the assembler will fail
            bool dclsFound = false;
            for (i = args.begin(); i != args.end(); ++i)
            {
                if (*i == "dcls")
                {
                    dclsFound = true;
                    break;
                }
            }
            if (!dclsFound)
            {
                args.push_back("-profileopts dcls");
            }
        }
        // Now split args into that god-awful char** that Cg insists on
        freeCgArgs();
        mCgArguments = new char*[args.size() + 1];
        int index = 0;
        for (i = args.begin(); i != args.end(); ++i, ++index)
        {
            mCgArguments[index] = new char[i->length() + 1];
            strcpy(mCgArguments[index], i->c_str());
        }
        // Null terminate list
        mCgArguments[index] = 0;


    }
    //-----------------------------------------------------------------------
    void CgProgram::freeCgArgs(void)
    {
        if (mCgArguments)
        {
            int index = 0;
            char* current = mCgArguments[index];
            while (current)
            {
                delete [] current;
                current = mCgArguments[++index];
            }
            delete [] mCgArguments;
            mCgArguments = 0;
        }
    }
    //-----------------------------------------------------------------------
    void CgProgram::loadFromSource(void)
    {
        // Create Cg Program
        selectProfile();
        buildArgs();
        mCgProgram = cgCreateProgram(mCgContext, CG_SOURCE, mSource.c_str(), 
            mSelectedCgProfile, mEntryPoint.c_str(), const_cast<const char**>(mCgArguments));

        // Test
        //LogManager::getSingleton().logMessage(cgGetProgramString(mCgProgram, CG_COMPILED_PROGRAM));

        // Check for errors
        checkForCgError("CgProgram::loadFromSource", 
            "Unable to compile Cg program " + mName + ": ", mCgContext);

    }
    //-----------------------------------------------------------------------
    void CgProgram::createLowLevelImpl(void)
    {
        // Create a low-level program, give it the same name as us
        mAssemblerProgram = 
            GpuProgramManager::getSingleton().createProgramFromString(
                mName, 
                cgGetProgramString(mCgProgram, CG_COMPILED_PROGRAM),
                mType, 
                mSelectedProfile);

    }
    //-----------------------------------------------------------------------
    void CgProgram::unloadImpl(void)
    {
        // Unload Cg Program
        // Lowlevel program will get unloaded elsewhere
        if (mCgProgram)
        {
            cgDestroyProgram(mCgProgram);
            checkForCgError("CgProgram::unloadImpl", 
                "Error while unloading Cg program " + mName + ": ", 
                mCgContext);
            mCgProgram = 0;
        }
    }
    //-----------------------------------------------------------------------
    void CgProgram::populateParameterNames(GpuProgramParametersSharedPtr params)
    {
        // Derive parameter names from Cg
        assert(mCgProgram && "Cg program not loaded!");
        // Note use of 'leaf' format so we only get bottom-level params, not structs
        CGparameter parameter = cgGetFirstLeafParameter(mCgProgram, CG_PROGRAM);
        while (parameter != 0) 
        {
            // Look for uniform (non-sampler) parameters only
            // Don't bother enumerating unused parameters, especially since they will
            // be optimised out and therefore not in the indexed versions
            CGtype paramType = cgGetParameterType(parameter);
            
            // *** test
            //String tempName = cgGetParameterName(parameter);
            //size_t tempindex = cgGetParameterResourceIndex(parameter);
            //LogManager::getSingleton().logMessage(
            //    tempName + " -> " + StringConverter::toString(tempindex));

            // *** end test

            if (cgGetParameterVariability(parameter) == CG_UNIFORM &&
                paramType != CG_SAMPLER1D &&
                paramType != CG_SAMPLER2D &&
                paramType != CG_SAMPLER3D &&
                paramType != CG_SAMPLERCUBE &&
                paramType != CG_SAMPLERRECT &&
                cgGetParameterDirection(parameter) != CG_OUT && 
                cgIsParameterReferenced(parameter))
            {
                String paramName = cgGetParameterName(parameter);
                size_t index = cgGetParameterResourceIndex(parameter);
                params->_mapParameterNameToIndex(paramName, index);
            }
            // Get next
            parameter = cgGetNextLeafParameter(parameter);
        }

        
    }
    //-----------------------------------------------------------------------
    CgProgram::CgProgram(const String& name, GpuProgramType gpType, 
        const String& language, CGcontext context)
        : HighLevelGpuProgram(name, gpType, language), mCgContext(context), 
        mCgProgram(0), mSelectedCgProfile(CG_PROFILE_UNKNOWN), mCgArguments(0)
    {
        if (createParamDictionary("CgProgram"))
        {
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("entry_point", 
                "The entry point for the Cg program.",
                PT_STRING),&msCmdEntryPoint);
            dict->addParameter(ParameterDef("profiles", 
                "Space-separated list of Cg profiles supported by this profile.",
                PT_STRING),&msCmdProfiles);
            dict->addParameter(ParameterDef("compile_arguments", 
                "A string of compilation arguments to pass to the Cg compiler.",
                PT_STRING),&msCmdArgs);
        }
        
    }
    //-----------------------------------------------------------------------
    CgProgram::~CgProgram()
    {
        freeCgArgs();
        // unload will be called by superclass
    }
    //-----------------------------------------------------------------------
    bool CgProgram::isSupported(void)
    {
        StringVector::iterator i, iend;
        iend = mProfiles.end();
        // Check to see if any of the profiles are supported
        for (i = mProfiles.begin(); i != iend; ++i)
        {
            if (GpuProgramManager::getSingleton().isSyntaxSupported(*i))
            {
                return true;
            }
        }
        return false;

    }
    //-----------------------------------------------------------------------
    void CgProgram::setProfiles(const StringVector& profiles)
    {
        mProfiles.clear();
        StringVector::const_iterator i, iend;
        iend = profiles.end();
        for (i = profiles.begin(); i != iend; ++i)
        {
            mProfiles.push_back(*i);
        }
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String CgProgram::CmdEntryPoint::doGet(void *target)
    {
        return static_cast<CgProgram*>(target)->getEntryPoint();
    }
    void CgProgram::CmdEntryPoint::doSet(void *target, const String& val)
    {
        static_cast<CgProgram*>(target)->setEntryPoint(val);
    }
    //-----------------------------------------------------------------------
    String CgProgram::CmdProfiles::doGet(void *target)
    {
        return StringConverter::toString(
            static_cast<CgProgram*>(target)->getProfiles() );
    }
    void CgProgram::CmdProfiles::doSet(void *target, const String& val)
    {
        static_cast<CgProgram*>(target)->setProfiles(val.split());
    }
    //-----------------------------------------------------------------------
    String CgProgram::CmdArgs::doGet(void *target)
    {
        return static_cast<CgProgram*>(target)->getCompileArguments();
    }
    void CgProgram::CmdArgs::doSet(void *target, const String& val)
    {
        static_cast<CgProgram*>(target)->setCompileArguments(val);
    }

}
