/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
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
#ifndef __GuiElementEmitterCommands_H__
#define __GuiElementEmitterCommands_H__

#include "OgrePrerequisites.h"
#include "OgreStringInterface.h"

namespace Ogre  {


    namespace GuiElementCommands {
        /// Command object for GuiElement  - see ParamCommand 
        class _OgreExport CmdLeft : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /// Command object for GuiElement  - see ParamCommand 
        class _OgreExport CmdTop : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /// Command object for GuiElement  - see ParamCommand 
        class _OgreExport CmdWidth : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /// Command object for GuiElement  - see ParamCommand 
        class _OgreExport CmdHeight : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /// Command object for GuiElement  - see ParamCommand 
        class _OgreExport CmdMaterial : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /// Command object for GuiElement  - see ParamCommand 
        class _OgreExport CmdCaption : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /// Command object for GuiElement  - see ParamCommand 
        class _OgreExport CmdMetricsMode : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /// Command object for GuiElement  - see ParamCommand 
        class _OgreExport CmdHorizontalAlign : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /// Command object for GuiElement  - see ParamCommand 
        class _OgreExport CmdVerticalAlign : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

    }
}

#endif


