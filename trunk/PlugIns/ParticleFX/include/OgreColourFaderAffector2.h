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
#ifndef __ColourFaderAffector2_H__
#define __ColourFaderAffector2_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreStringInterface.h"

namespace Ogre {


    /** This plugin subclass of ParticleAffector allows you to alter the colour of particles.
    @remarks
        This class supplies the ParticleAffector implementation required to modify the colour of
        particle in mid-flight.
    */
    class _OgreParticleFXExport ColourFaderAffector2 : public ParticleAffector
    {
    public:

        /** Command object for red adjust (see ParamCommand).*/
        class CmdRedAdjust1 : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

        /** Command object for green adjust (see ParamCommand).*/
        class CmdGreenAdjust1 : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

        /** Command object for blue adjust (see ParamCommand).*/
        class CmdBlueAdjust1 : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

        /** Command object for alpha adjust (see ParamCommand).*/
        class CmdAlphaAdjust1 : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

        /** Command object for red adjust (see ParamCommand).*/
        class CmdRedAdjust2 : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

        /** Command object for green adjust (see ParamCommand).*/
        class CmdGreenAdjust2 : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

        /** Command object for blue adjust (see ParamCommand).*/
        class CmdBlueAdjust2 : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

        /** Command object for alpha adjust (see ParamCommand).*/
        class CmdAlphaAdjust2 : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

        /** Command object for alpha adjust (see ParamCommand).*/
        class CmdStateChange : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };


        /** Default constructor. */
        ColourFaderAffector2();

        /** See ParticleAffector. */
        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed);

        /** Sets the colour adjustment to be made per second to particles. 
        @param red, green, blue, alpha
            Sets the adjustment to be made to each of the colour components per second. These
            values will be added to the colour of all particles every second, scaled over each frame
            for a smooth adjustment.
        */
        void setAdjust1(Real red, Real green, Real blue, Real alpha = 0.0);
		void setAdjust2(Real red, Real green, Real blue, Real alpha = 0.0);
        /** Sets the red adjustment to be made per second to particles. 
        @param red
            The adjustment to be made to the colour component per second. This
            value will be added to the colour of all particles every second, scaled over each frame
            for a smooth adjustment.
        */
        void setRedAdjust1(Real red);
		void setRedAdjust2(Real red);

        /** Gets the red adjustment to be made per second to particles. */
        Real getRedAdjust1(void);
		Real getRedAdjust2(void);

        /** Sets the green adjustment to be made per second to particles. 
        @param green
            The adjustment to be made to the colour component per second. This
            value will be added to the colour of all particles every second, scaled over each frame
            for a smooth adjustment.
        */
        void setGreenAdjust1(Real green);
		void setGreenAdjust2(Real green);
        /** Gets the green adjustment to be made per second to particles. */
        Real getGreenAdjust1(void);
		Real getGreenAdjust2(void);
        /** Sets the blue adjustment to be made per second to particles. 
        @param blue
            The adjustment to be made to the colour component per second. This
            value will be added to the colour of all particles every second, scaled over each frame
            for a smooth adjustment.
        */
        void setBlueAdjust1(Real blue);
		void setBlueAdjust2(Real blue);
        /** Gets the blue adjustment to be made per second to particles. */
        Real getBlueAdjust1(void);
		Real getBlueAdjust2(void);

        /** Sets the alpha adjustment to be made per second to particles. 
        @param alpha
            The adjustment to be made to the colour component per second. This
            value will be added to the colour of all particles every second, scaled over each frame
            for a smooth adjustment.
        */
        void setAlphaAdjust1(Real alpha);
		void setAlphaAdjust2(Real alpha);
        /** Gets the alpha adjustment to be made per second to particles. */
        Real getAlphaAdjust1(void);
		Real getAlphaAdjust2(void);


        void setStateChange(Real NewValue );
        Real getStateChange(void);

        static CmdRedAdjust1 msRedCmd1;
		static CmdRedAdjust2 msRedCmd2;
        static CmdGreenAdjust1 msGreenCmd1;
		static CmdGreenAdjust2 msGreenCmd2;
        static CmdBlueAdjust1 msBlueCmd1;
		static CmdBlueAdjust2 msBlueCmd2;
        static CmdAlphaAdjust1 msAlphaCmd1;
		static CmdAlphaAdjust2 msAlphaCmd2;
        static CmdStateChange msStateCmd;

    protected:
        Real mRedAdj1, mRedAdj2;
        Real mGreenAdj1,  mGreenAdj2;
        Real mBlueAdj1, mBlueAdj2;
        Real mAlphaAdj1, mAlphaAdj2;
		Real StateChangeVal;

        /** Internal method for adjusting while clamping to [0,1] */
        inline void applyAdjustWithClamp(Real* pComponent, Real adjust)
        {
            *pComponent += adjust;
            // Limit to 0
            if (*pComponent < 0.0)
            {
                *pComponent = 0.0f;
            }
            // Limit to 1
            else if (*pComponent > 1.0)
            {
                *pComponent = 1.0f;
            }
        }

    };


}


#endif

