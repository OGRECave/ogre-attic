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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreParticleEmitterCommands.h"
#include "OgreParticleEmitter.h"
#include "OgreStringConverter.h"


namespace Ogre {

    namespace EmitterCommands {

        //-----------------------------------------------------------------------
        String CmdAngle::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getAngle() );
        }
        void CmdAngle::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setAngle(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdColour::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getColour() );
        }
        void CmdColour::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setColour(StringConverter::parseColourValue(val));
        }
        //-----------------------------------------------------------------------
        String CmdColourRangeStart::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getColourRangeStart() );
        }
        void CmdColourRangeStart::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setColourRangeStart(StringConverter::parseColourValue(val));
        }
        //-----------------------------------------------------------------------
        String CmdColourRangeEnd::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getColourRangeEnd() );
        }
        void CmdColourRangeEnd::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setColourRangeEnd(StringConverter::parseColourValue(val));
        }
        //-----------------------------------------------------------------------
        String CmdDirection::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getDirection() );
        }
        void CmdDirection::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setDirection(StringConverter::parseVector3(val));
        }
        //-----------------------------------------------------------------------
        String CmdEmissionRate::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getEmissionRate() );
        }
        void CmdEmissionRate::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setEmissionRate(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMaxTTL::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getMaxTimeToLive() );
        }
        void CmdMaxTTL::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMaxTimeToLive(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMinTTL::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getMinTimeToLive() );
        }
        void CmdMinTTL::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMinTimeToLive(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMaxVelocity::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getMaxParticleVelocity() );
        }
        void CmdMaxVelocity::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMaxParticleVelocity(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMinVelocity::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getMinParticleVelocity() );
        }
        void CmdMinVelocity::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMinParticleVelocity(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdPosition::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getPosition() );
        }
        void CmdPosition::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setPosition(StringConverter::parseVector3(val));
        }
        //-----------------------------------------------------------------------
        String CmdTTL::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getTimeToLive() );
        }
        void CmdTTL::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setTimeToLive(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdVelocity::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getParticleVelocity() );
        }
        void CmdVelocity::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setParticleVelocity(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdDuration::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getDuration() );
        }
        void CmdDuration::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setDuration(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMinDuration::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getMinDuration() );
        }
        void CmdMinDuration::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMinDuration(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMaxDuration::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getMaxDuration() );
        }
        void CmdMaxDuration::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMaxDuration(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdRepeatDelay::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getRepeatDelay() );
        }
        void CmdRepeatDelay::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setRepeatDelay(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMinRepeatDelay::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getMinRepeatDelay() );
        }
        void CmdMinRepeatDelay::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMinRepeatDelay(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMaxRepeatDelay::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<ParticleEmitter*>(target)->getMaxRepeatDelay() );
        }
        void CmdMaxRepeatDelay::doSet(void* target, const String& val)
        {
            static_cast<ParticleEmitter*>(target)->setMaxRepeatDelay(StringConverter::parseReal(val));
        }
 

    
    }
}

