/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
 
Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"

#include "OgreOSXGLSupport.h"

#include "OgreOSXWindow.h"
#include "OgreGLTexture.h"
#include "OgreOSXRenderTexture.h"

#include <OpenGL/OpenGL.h>
#include <mach-o/dyld.h>

namespace Ogre {

OSXGLSupport::OSXGLSupport()
{
	LogManager::getSingleton().logMessage( "OSXGLSupport::OSXGLSupport()" );
}

OSXGLSupport::~OSXGLSupport()
{
	LogManager::getSingleton().logMessage( "OSXGLSupport::~OSXGLSupport()" );
}

void OSXGLSupport::addConfig( void )
{
	LogManager::getSingleton().logMessage( "OSXGLSupport::addConfig()" );
	ConfigOption optFullScreen;
	ConfigOption optVideoMode;
	ConfigOption optBitDepth;
    ConfigOption optFSAA;
	ConfigOption optRTTMode;

	// FS setting possiblities
	optFullScreen.name = "Full Screen";
	optFullScreen.possibleValues.push_back( "Yes" );
	optFullScreen.possibleValues.push_back( "No" );
	optFullScreen.currentValue = "No";
	optFullScreen.immutable = false;

	optBitDepth.name = "Colour Depth";
	optBitDepth.possibleValues.push_back( "32" );
	optBitDepth.possibleValues.push_back( "16" );
	optBitDepth.currentValue = "32";
	optBitDepth.immutable = false;

    mOptions[ optFullScreen.name ] = optFullScreen;
	mOptions[ optBitDepth.name ] = optBitDepth;

	CGLRendererInfoObj rend;
	long nrend;
	CGLQueryRendererInfo( CGDisplayIDToOpenGLDisplayMask( kCGDirectMainDisplay ), &rend, &nrend );

	long maxSamples;
	CGLDescribeRenderer( rend, 0, kCGLRPMaxSamples, &maxSamples );

    //FSAA possibilities
    optFSAA.name = "FSAA";
    optFSAA.possibleValues.push_back( "0" );

	switch( maxSamples )
	{
		case 6:
			optFSAA.possibleValues.push_back( "2" );
			optFSAA.possibleValues.push_back( "4" );
			optFSAA.possibleValues.push_back( "6" );
			break;
		case 4:
			optFSAA.possibleValues.push_back( "2" );
			optFSAA.possibleValues.push_back( "4" );
			break;
		case 2:
			optFSAA.possibleValues.push_back( "2" );
			break;
		default: break;
	}

    optFSAA.currentValue = "0";
    optFSAA.immutable = false;

    mOptions[ optFSAA.name ] = optFSAA;

	// Video mode possiblities
	optVideoMode.name = "Video Mode";
	optVideoMode.immutable = false;

/*
	CFArrayRef display_modes = CGDisplayAvailableModes( CGMainDisplayID() );
	CFIndex num_modes = CFArrayGetCount( display_modes );
	CFMutableArrayRef resolutions = NULL;
	resolutions = CFArrayCreateMutable( kCFAllocatorDefault, 0, NULL );
	
	if( resolutions )
	{
		for( int i = 0; i < num_modes; i++ )
		{
			CFDictionaryRef modeInfo = ( CFDictionaryRef ) CFArrayGetValueAtIndex( display_modes, i );
			if( CFDictionaryGetValue( modeInfo, kCGDisplayModeIsSafeForHardware ) == kCFBooleanTrue )
			{
				int width = 0;
				int height = 0;
				char charMode[ 12 ];
				CFNumberRef value;
				
				// width
				value = ( CFNumberRef ) CFDictionaryGetValue( modeInfo, kCGDisplayWidth );
				CFNumberGetValue( value, kCFNumberIntType, &width );
				// height
				value = ( CFNumberRef ) CFDictionaryGetValue( modeInfo, kCGDisplayHeight );
				CFNumberGetValue( value, kCFNumberIntType, &height );

				printf( charMode, "%d x %d", width, height );
//				CFMutableStringRef res = NULL;
//				res = CFStringCreateMutable( kCFAllocatorDefault, 0 );
//				CFStringAppendFormat( res, NULL, CFSTR( "%d x %d" ), width, height );
				
				CFRange range;
				range.location = 0;
				range.length = CFArrayGetCount( resolutions );
				if( !CFArrayContainsValue( resolutions, range, charMode ) )
					CFArrayAppendValue( resolutions, charMode );
			}
		}
	}
	
	CFIndex resCount = CFArrayGetCount( resolutions );
	
	for( int i = 0; i < resCount; i++ )
	{
		LogManager::getSingleton().logMessage( "Added resolution: " );
		LogManager::getSingleton().logMessage( ( char* ) CFArrayGetValueAtIndex( resolutions, i ) );
		optVideoMode.possibleValues.push_back( ( char* ) CFArrayGetValueAtIndex( resolutions, i ) );
	}
*/
	optVideoMode.possibleValues.push_back( "1024 x 768" );
	optVideoMode.possibleValues.push_back( "800 x 600" );
	optVideoMode.possibleValues.push_back( "640 x 480" );
	optVideoMode.currentValue = "640 x 480";
	
	optRTTMode.name = "RTT Preferred Mode";
	optRTTMode.possibleValues.push_back( "FBO" );
	optRTTMode.possibleValues.push_back( "PBuffer" );
	optRTTMode.possibleValues.push_back( "Copy" );
	optRTTMode.currentValue = "FBO";
	optRTTMode.immutable = false;


	mOptions[optFullScreen.name] = optFullScreen;
	mOptions[optVideoMode.name] = optVideoMode;
    mOptions[optFSAA.name] = optFSAA;
	mOptions[optRTTMode.name] = optRTTMode;
}

String OSXGLSupport::validateConfig( void )
{
	LogManager::getSingleton().logMessage( "OSXGLSupport::validateConfig()" );
	return String( "" );
}

RenderWindow* OSXGLSupport::createWindow( bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle ) 
{
	LogManager::getSingleton().logMessage( "OSXGLSupport::createWindow()" );
	if( autoCreateWindow )
	{
		ConfigOptionMap::iterator opt = mOptions.find( "Full Screen" );
		if( opt == mOptions.end() )
			OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "Can't find full screen options!", "OSXGLSupport::createWindow" );
		bool fullscreen = ( opt->second.currentValue == "Yes" );

		opt = mOptions.find( "Video Mode" );
		if( opt == mOptions.end() )
			OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "Can't find video mode options!", "OSXGLSupport::createWindow" );
		String val = opt->second.currentValue;
		String::size_type pos = val.find( 'x' );
		if( pos == String::npos )
			OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "Invalid Video Mode provided", "OSXGLSupport::createWindow" );

		unsigned int w = StringConverter::parseUnsignedInt( val.substr( 0, pos ) );
		unsigned int h = StringConverter::parseUnsignedInt( val.substr( pos + 1 ) );

        // Parse FSAA config
		NameValuePairList winOptions;
		winOptions[ "title" ] = windowTitle;
        int fsaa_x_samples = 0;
        opt = mOptions.find( "FSAA" );
        if( opt != mOptions.end() )
        {
			winOptions[ "FSAA" ] = opt->second.currentValue;
        }

		return renderSystem->createRenderWindow( windowTitle, w, h, fullscreen, &winOptions );
	}
	else
	{
		// XXX What is the else?
		return NULL;
	}
}

RenderWindow* OSXGLSupport::newWindow( const String &name, unsigned int width, unsigned int height, 
	bool fullScreen, const NameValuePairList *miscParams )
{
	LogManager::getSingleton().logMessage( "OSXGLSupport::newWindow()" );
	OSXWindow* window = new OSXWindow();
	window->create( name, width, height, fullScreen, miscParams );
	return window;
}

void OSXGLSupport::start()
{
	LogManager::getSingleton().logMessage(
	        "*******************************************\n"
	        "*** Starting native OSX (AGL) Subsystem ***\n"
	        "*******************************************");
}

void OSXGLSupport::stop()
{
	LogManager::getSingleton().logMessage(
	        "*******************************************\n"
	        "*** Stopping native OSX (AGL) Subsystem ***\n"
	        "*******************************************");
}

void* OSXGLSupport::getProcAddress( const char* name )
{
	NSSymbol symbol;
    char *symbolName;
    // Prepend a '_' for the Unix C symbol mangling convention
    symbolName = (char*)malloc (strlen (name) + 2);
    strcpy(symbolName + 1, name);
    symbolName[0] = '_';
    symbol = NULL;
    if (NSIsSymbolNameDefined (symbolName))
        symbol = NSLookupAndBindSymbol (symbolName);
    free (symbolName);
    return symbol ? NSAddressOfSymbol (symbol) : NULL;
}

void* OSXGLSupport::getProcAddress( const String& procname )
{
	return getProcAddress( procname.c_str() );
}

bool OSXGLSupport::supportsPBuffers()
{
	LogManager::getSingleton().logMessage( "OSXGLSupport::supportsPBuffers()" );
	return true;
}

GLPBuffer* OSXGLSupport::createPBuffer( PixelComponentType format, size_t width, size_t height )
{
	LogManager::getSingleton().logMessage( "OSXGLSupport::createPBuffer()" );
	return new OSXPBuffer( format, width, height );
}

}
