/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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
#ifndef _OgreExternalTextureSource_H
#define _OgreExternalTextureSource_H
 
/***************************************************************************
OgreExternalTextureSource.h  -  
	Base class that texture plugins need to derive from. This provides the hooks
	neccessary for a plugin developer to easily extend the functionality of dynamic textures.
	It makes creation/destruction of dynamic textures more streamlined. While the plugin
	will need to talk with Ogre for the actual modification of textures, this class allows
	easy integration with Ogre apps. Material script files can be used to aid in the 
	creation of dynamic textures. Functionality can be added that is not defined here
	through the use of the base dictionary. For an exmaple of how to use this class and the
	string interface see ffmpegVideoPlugIn.

-------------------
date                 : Jan 1 2004
email                : pjcast@yahoo.com
***************************************************************************/

#include "OgreStringInterface.h"

namespace Ogre
{
	/** Enum for type of texture play mode */
	enum eTexturePlayMode
	{
		TextureEffectPause = 0,			//! Video starts out paused
		TextureEffectPlay_ASAP = 1,		//! Video starts playing as soon as posible
		TextureEffectPlay_Looping = 2	//! Video Plays Instantly && Loops
	};

	/** Base class that texture plugins derive from. Any specific requirements that the plugin
	needs to have defined before texture/material creation must be define using the stringinterface
	before calling create defined texture... or it will fail, though, it is up to the plugin 
	to report errors to the log file, or raise an exception if need be. */
	class _OgreExport ExternalTextureSource : public StringInterface
	{
	public:
		/** Constructor */
		ExternalTextureSource();
		/** Virtual destructor */
		virtual ~ExternalTextureSource() {}

		//------------------------------------------------------------------------------//
		/* Command objects for specifying some base features							*/
		/* Any PlugIns wishing to add more specific params to "ExternalTextureSourcePlugins"*/
		/* dictionary, feel free to do so, that's why this is here						*/
        class CmdInputFileName : public ParamCommand
        {
        public:
			String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        class CmdFPS : public ParamCommand
        {
        public:
			String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        class CmdPlayMode : public ParamCommand
        {
        public:
			String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        class CmdTecPassState : public ParamCommand
        {
        public:
			String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
		//--------------------------------------------------------//
		//Base Functions that work with Command String Interface... Or can be called
		//manually to create video through code 

		//! Sets an input file name - if needed by plugin
		void setInputName( String sIN ) { mInputFileName = sIN; }
		//! Gets currently set input file name
		const String& getInputName( ) const	{ return mInputFileName; }
		//! Sets the frames per second - plugin may or may not use this
		void setFPS( int iFPS ) { mFramesPerSecond = iFPS; }
		//! Gets currently set frames per second
		const int getFPS( ) const { return mFramesPerSecond; }
		//! Sets a play mode
		void setPlayMode( eTexturePlayMode eMode )	{ mMode = eMode; }
		//! Gets currently set play mode
		eTexturePlayMode getPlayMode() const { return mMode; }

		//! Used for attaching texture to Technique, State, and texture unit layer
		void setTextureTecPassStateLevel( int t, int p, int s ) 
				{ mTechniqueLevel = t;mPassLevel = p;mStateLevel = s; }
		//! Get currently selected Textute attribs.
		void getTextureTecPassStateLevel( int& t, int& p, int& s ) const
				{t = mTechniqueLevel;	p = mPassLevel;	s = mStateLevel;}
		
		/** Call from derived classes to ensure the dictionary is setup */
		void addBaseParams();

		/** Returns the string name of this PlugIn (as set by the PlugIn)*/
		const String& getPlugInStringName( ) { return mPlugInName; }
		/** Returns dictionary name */
		const String& getDictionaryStringName( ) { return mDictionaryName; }

		//Pure virtual functions that plugins must Override
		/** Call this function from manager to init system */
		virtual bool Initialise() = 0;
		/** Shuts down PlugIn */
		virtual void ShutDown() = 0;

		/** Creates a texture into an already defined material or one that is created new
		(it's up to plugin to use a material or create one)
		Before calling, ensure that needed params have been defined via the stringInterface
		class setParameter( .. )*/
		virtual void createDefinedTexture( String sMaterialName ) = 0;
		/** Destroys the texture, material, and mem associated with this texture*/
		virtual void DestroyAdvancedTexture( String sTextureName ) = 0;

	protected:
        static CmdInputFileName msCmdInputFile;		//! Command for setting input file name
		static CmdFPS msCmdFramesPerSecond;			//! Command for setting frames per second
		static CmdPlayMode msCmdPlayMode;			//! Command for setting play mode
		static CmdTecPassState msCmdTecPassState;	//! Command for setting the tecnique, pass, & state level


		//! String Name of this PlugIn
		String mPlugInName;
	
		//------ Vars used for setting/getting dictionary stuff -----------//
		eTexturePlayMode mMode;
		
		String mInputFileName;
		
		bool mUpdateEveryFrame;
		
		int mFramesPerSecond,
			mTechniqueLevel,
			mPassLevel,	
			mStateLevel;
		//------------------------------------------------------------------//

	private:
		//! The string name of the dictionary name
		String mDictionaryName;
	};
}

#endif
