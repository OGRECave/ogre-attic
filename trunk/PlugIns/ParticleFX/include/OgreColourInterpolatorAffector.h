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
#ifndef __ColourInterpolatorAffector_H__
#define __ColourInterpolatorAffector_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreStringInterface.h"
#include "OgreColourValue.h"

namespace Ogre {


    class _OgreParticleFXExport ColourInterpolatorAffector : public ParticleAffector
    {
    public:
		// this is something of a hack.. 
		// needs to be replaced with something more.. 
		// ..elegant
		enum { MAX_STAGES = 6 };


        /** Command object for red adjust (see ParamCommand).*/
        class CmdColourAdjust : public ParamCommand
        {
		public:
			int		mIndex;

		public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

        /** Command object for red adjust (see ParamCommand).*/
		class CmdTimeAdjust : public ParamCommand
        {
        public:
			int		mIndex;

		public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
		};

        /** Default constructor. */
        ColourInterpolatorAffector();

        /** See ParticleAffector. */
        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed);

		void setColourAdjust(int index, ColourValue colour);
        ColourValue getColourAdjust(int index);
        
		void setTimeAdjust(int index, Real time);
        Real getTimeAdjust(int index);
        
        
        static CmdColourAdjust	msColourCmd[MAX_STAGES];
        static CmdTimeAdjust	msTimeCmd[MAX_STAGES];

    protected:
        ColourValue				mColourAdj[MAX_STAGES];
		Real					mTimeAdj[MAX_STAGES];

    };


}


#endif

