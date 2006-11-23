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

#include "OgreOSXWindow.h"
#include "OgreRoot.h"
#include "OgreGLRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreWindowEventUtilities.h"

#include <OpenGL/gl.h>
#define GL_EXT_texture_env_combine 1
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#include <AGL/agl.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glx.h>

namespace Ogre
{

//-------------------------------------------------------------------------------------------------//
OSXWindow::OSXWindow()
{
	LogManager::getSingleton().logMessage( "OSXWindow::OSXWindow()" );
	mActive = mClosed = mHasResized = false;
	mAGLContext = NULL;
	mContext = NULL;
	mWindow = NULL;
}

//-------------------------------------------------------------------------------------------------//
OSXWindow::~OSXWindow()
{
	LogManager::getSingleton().logMessage( "OSXWindow::~OSXWindow()" );
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::create( const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams )
{
	LogManager::getSingleton().logMessage( "OSXWindow::create()" );

	bool hasDepthBuffer;
	String title = name;
	size_t fsaa_samples = 0;
	int left = 0;
	int top = 0;
	int depth = 32;
	
	if( miscParams )
	{
		NameValuePairList::const_iterator opt = NULL;
		// Full screen anti aliasing
		opt = miscParams->find( "FSAA" );
		if( opt != miscParams->end() )
			fsaa_samples = StringConverter::parseUnsignedInt( opt->second );

		opt = miscParams->find( "left" );
		if( opt != miscParams->end() )
			left = StringConverter::parseUnsignedInt( opt->second );

		opt = miscParams->find( "top" );
		if( opt != miscParams->end() )
			top = StringConverter::parseUnsignedInt( opt->second );

		opt = miscParams->find( "title" );
		if( opt != miscParams->end() )
			title = opt->second;

		opt = miscParams->find( "depthBuffer" );
		if( opt != miscParams->end() )
			hasDepthBuffer = StringConverter::parseBool( opt->second );
		
		opt = miscParams->find( "colourDepth" );
		if( opt != miscParams->end() )
			depth = StringConverter::parseUnsignedInt( opt->second );
	}

	AGLPixelFormat pixelFormat;
	GLint attribs[ 20 ];
	
	int i = 0;
	
	if( fullScreen )
	{
		attribs[ i++ ] = AGL_FULLSCREEN;
	}
	
	attribs[ i++ ] = AGL_RGBA;
	attribs[ i++ ] = AGL_DOUBLEBUFFER;
	attribs[ i++ ] = AGL_ALPHA_SIZE;
	attribs[ i++ ] = 0;
	attribs[ i++ ] = AGL_STENCIL_SIZE;
	attribs[ i++ ] = 1;
	attribs[ i++ ] = AGL_DEPTH_SIZE;
	attribs[ i++ ] = depth;
	
	if( fsaa_samples > 1 )
	{
		attribs[ i++ ] = AGL_MULTISAMPLE;
		attribs[ i++ ] = 1;
		attribs[ i++ ] = AGL_SAMPLE_BUFFERS_ARB;
		attribs[ i++ ] = fsaa_samples;
	}
	
	attribs[ i++ ] = AGL_NONE;
	
	pixelFormat = aglChoosePixelFormat( NULL, 0, attribs );
	
	GLRenderSystem *rs = static_cast< GLRenderSystem* >( Root::getSingleton().getRenderSystem() );
	OSXContext* mainContext = ( OSXContext* ) rs->_getMainContext();
	AGLContext shareContext = mainContext == 0 ? NULL : mainContext->getContext();
	mAGLContext = aglCreateContext( pixelFormat, shareContext );
	
	// You do not need to create a window in full screen mode, if the user requested full screen
	// call AGL to do so, however we need to save the current display settings for restoring after
	// we quit.
	if(fullScreen)
	{
		// TODO: FullScreen
		aglSetFullScreen(mAGLContext, width, height, 0, 0);
		//aglSetDrawable( mAGLContext, GetWindowPort( mWindow ) );
		aglSetCurrentContext( mAGLContext );
		
		mName = name;
		mWidth = width;
		mHeight = height;
		mContext = new OSXContext( mAGLContext );
		
	}
	else
	{
		// create the window rect in global coords
		::Rect windowRect;
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = width;
		windowRect.bottom = height;
		
		// set the default attributes for the window
		WindowAttributes windowAttrs = kWindowStandardDocumentAttributes
			| kWindowStandardHandlerAttribute 
			| kWindowInWindowMenuAttribute
			| kWindowHideOnFullScreenAttribute;
		
		// Create the window
		CreateNewWindow(kDocumentWindowClass, windowAttrs, &windowRect, &mWindow);
		
		// Color the window background black
		SetThemeWindowBackground (mWindow, kThemeBrushBlack, true);
		
		// Set the title of our window
		CFStringRef titleRef = CFStringCreateWithCString( kCFAllocatorDefault, title.c_str(), kCFStringEncodingASCII );
		SetWindowTitleWithCFString( mWindow, titleRef );
		
		// Center our window on the screen
		RepositionWindow( mWindow, NULL, kWindowCenterOnMainScreen );
		
		aglSetDrawable( mAGLContext, GetWindowPort( mWindow ) );
		aglSetCurrentContext( mAGLContext );
		
		mName = name;
		mWidth = width;
		mHeight = height;
		mContext = new OSXContext( mAGLContext );

		WindowEventUtilities::_addRenderWindow( this );
	
		InstallStandardEventHandler( GetWindowEventTarget( mWindow ) );
		ShowWindow( mWindow );
		SelectWindow( mWindow );
	}
		
	mActive = true;
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::destroy(void)
{
	LogManager::getSingleton().logMessage( "OSXWindow::destroy()" );

	delete mContext;
	
	WindowEventUtilities::_removeRenderWindow( this );

	if( mWindow )
		DisposeWindow( mWindow );

	mActive = false;

	Root::getSingleton().getRenderSystem()->detachRenderTarget( this->getName() );
}

//-------------------------------------------------------------------------------------------------//
bool OSXWindow::isActive() const
{
	return mActive;
}

//-------------------------------------------------------------------------------------------------//
bool OSXWindow::isClosed() const
{
	return false;
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::reposition(int left, int top)
{
	LogManager::getSingleton().logMessage( "OSXWindow::reposition()" );
	if( mWindow )
		MoveWindow( mWindow, left, top, true );
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::resize(unsigned int width, unsigned int height)
{
	LogManager::getSingleton().logMessage( "OSXWindow::resize()" );
	if( !mWindow )
		return;

	// Check if the window size really changed
	if( mWidth == width && mHeight == height )
		return;

	mWidth = width;
	mHeight = height;

	SizeWindow( mWindow, width, height, true );
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::windowHasResized()
{
	LogManager::getSingleton().logMessage( "OSXWindow::windowHasResized()" );
	mHasResized = true;
	aglUpdateContext( mAGLContext );
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::windowResized()
{
	LogManager::getSingleton().logMessage( "OSXWindow::windowResized()" );
	aglUpdateContext( mAGLContext );
	::Rect rect;
	GetWindowBounds( mWindow, kWindowContentRgn, &rect );
	mWidth = rect.left + rect.right;
	mHeight = rect.top + rect.bottom;
	mLeft = rect.left;
	mTop = rect.top;

	for( ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it )
	{
		( *it ).second->_updateDimensions();
	}
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::swapBuffers( bool waitForVSync )
{
	aglSwapBuffers( mAGLContext );
	if( mHasResized )
	{
		windowResized();
		mHasResized = false;
	}
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::getCustomAttribute( const String& name, void* pData )
{
	if( name == "GLCONTEXT" ) 
	{
		*static_cast< OSXContext** >( pData ) = mContext;
		return;
	}
	else if( name == "WINDOW" )
	{
		*static_cast< WindowRef* >( pData ) = mWindow;
		return;
	}
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::writeContentsToFile(const String& filename)
{
	ImageCodec::ImageData* imgData = new ImageCodec::ImageData;
	imgData->width = mWidth;
	imgData->height = mHeight;
	imgData->format = PF_BYTE_RGB;

	// Allocate buffer
	uchar* pBuffer = new uchar[mWidth * mHeight * 3];

	// Read pixels
	// I love GL: it does all the locking & colour conversion for us
	glReadPixels(0,0, mWidth-1, mHeight-1, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);

	// Wrap buffer in a memory stream
	DataStreamPtr stream(new MemoryDataStream(pBuffer, mWidth * mHeight * 3, false));

	// Need to flip the read data over in Y though
	Image img;
	img.loadRawData(stream, mWidth, mHeight, PF_BYTE_RGB );
	img.flipAroundX();

	MemoryDataStreamPtr streamFlipped(new MemoryDataStream(img.getData(), stream->size(), false));

	// Get codec
	size_t pos = filename.find_last_of(".");
	String extension;
	if( pos == String::npos )
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Unable to determine image type for '" 
			+ filename + "' - invalid extension.", "OSXWindow::writeContentsToFile" );

	while( pos != filename.length() - 1 )
		extension += filename[++pos];

	// Get the codec
	Codec * pCodec = Codec::getCodec(extension);

	// Write out
	Codec::CodecDataPtr codecDataPtr(imgData);
	pCodec->codeToFile(streamFlipped, filename, codecDataPtr);

	delete [] pBuffer;
}
}
