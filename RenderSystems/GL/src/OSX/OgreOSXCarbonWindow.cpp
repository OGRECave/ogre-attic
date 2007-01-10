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

#include "OgreOSXCarbonWindow.h"
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

namespace Ogre
{

//-------------------------------------------------------------------------------------------------//
OSXCarbonWindow::OSXCarbonWindow()
{
	mActive = mClosed = mHasResized = false;
	mAGLContext = NULL;
	mContext = NULL;
	mWindow = NULL;
	mView = NULL;
}

//-------------------------------------------------------------------------------------------------//
OSXCarbonWindow::~OSXCarbonWindow()
{
}

//-------------------------------------------------------------------------------------------------//
void OSXCarbonWindow::create( const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams )
{
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
	
	if(fullScreen)
	{
		GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
		OSXContext *mainContext = (OSXContext*)rs->_getMainContext();
		
		CGLContextObj share = NULL;
		if(mainContext == 0)
		{
			share = NULL;
		}
		else if(mainContext->getContextType() == "AGL")
		{
			OSXCarbonContext* aglShare = static_cast<OSXCarbonContext*>(mainContext);
			aglGetCGLContext(aglShare->getContext(), &((void*)share));
		}
		else if(mainContext->getContextType() == "CGL")
		{
			OSXCGLContext* cglShare = static_cast<OSXCGLContext*>(mainContext);
			share = cglShare->getContext();
		}
		
		// create the context
		createCGLFullscreen(width, height, depth, fsaa_samples, share);		
	}
	else
	{
		int i = 0;
		AGLPixelFormat pixelFormat;
		GLint attribs[ 20 ];
		
		attribs[ i++ ] = AGL_NO_RECOVERY;
		attribs[ i++ ] = GL_TRUE;
		attribs[ i++ ] = AGL_ACCELERATED;
		attribs[ i++ ] = GL_TRUE;
		attribs[ i++ ] = AGL_RGBA;
		attribs[ i++ ] = AGL_DOUBLEBUFFER;
		attribs[ i++ ] = AGL_ALPHA_SIZE;
		attribs[ i++ ] = 8;
		attribs[ i++ ] = AGL_STENCIL_SIZE;
		attribs[ i++ ] = 8;
		attribs[ i++ ] = AGL_DEPTH_SIZE;
		attribs[ i++ ] = depth;
	
		if(fsaa_samples > 1)
		{
			attribs[ i++ ] = AGL_MULTISAMPLE;
			attribs[ i++ ] = 1;
			attribs[ i++ ] = AGL_SAMPLE_BUFFERS_ARB;
			attribs[ i++ ] = fsaa_samples;
		}
	
		attribs[ i++ ] = AGL_NONE;
	
		pixelFormat = aglChoosePixelFormat( NULL, 0, attribs );
	
		// Create the AGLContext from our pixel format
		// Share it with main
		GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
		OSXContext* mainContext = static_cast<OSXContext*>( rs->_getMainContext() );
		if(mainContext == 0)
		{
			mAGLContext = aglCreateContext(pixelFormat, NULL);
		}
		else if(mainContext->getContextType() == "AGL")
		{
			OSXCarbonContext* context = static_cast<OSXCarbonContext*>( rs->_getMainContext() );
			AGLContext shared = context->getContext();
			mAGLContext = aglCreateContext(pixelFormat, context->getContext());
		}
		else
		{
			// If we do not have an AGL, we can not clone it using this window
			LogManager::getSingleton().logMessage( "Warning: You asked to create a second window, "
				"when the previous window was not of this type.  OgreOSXCarbonWindow can only share "
				"with an AGL context.");
		}
		
		NameValuePairList::const_iterator opt = 0;
		if(miscParams)
			opt = miscParams->find("externalWindowHandle");
		if(!miscParams || opt == miscParams->end())
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
			
			// Install the event handler for the window
			InstallStandardEventHandler(GetWindowEventTarget(mWindow));
			
			// Display and select our window
			ShowWindow(mWindow);
			SelectWindow(mWindow);
		}
		else
		{
			// TODO: The Contol is going to report the incorrect location with a
			// Metalic / Textured window.  The default windows work just fine.
			
			// First get the HIViewRef / ControlRef
			mView = (HIViewRef)StringConverter::parseUnsignedLong(opt->second);
			mWindow = GetControlOwner(mView);
			
			// Lets try hiding the HIView
			//HIViewSetVisible(mView, false);
					
			// Get the rect bounds
			::Rect ctrlBounds;
			GetControlBounds(mView, &ctrlBounds);
			GLint bufferRect[4];

			bufferRect[0] = ctrlBounds.left;					// left edge
			bufferRect[1] = ctrlBounds.bottom;					// bottom edge
			bufferRect[2] =	ctrlBounds.right - ctrlBounds.left; // width of buffer rect
			bufferRect[3] = ctrlBounds.bottom - ctrlBounds.top; // height of buffer rect
			aglSetInteger(mAGLContext, AGL_BUFFER_RECT, bufferRect);
			aglEnable (mAGLContext, AGL_BUFFER_RECT);
		}
		
		// Set the drawable, and current context
		// If you do this last, there is a moment before the rendering window pops-up
		// This could go once inside each case above, before the window is displayed,
		// if desired.
		aglSetDrawable(mAGLContext, GetWindowPort(mWindow));
		aglSetCurrentContext(mAGLContext);

		// Give a copy of our context to the render system
		mContext = new OSXCarbonContext(mAGLContext);
	}
	
	WindowEventUtilities::_addRenderWindow(this);
	mName = name;
	mWidth = width;
	mHeight = height;
	mActive = true;
}

//-------------------------------------------------------------------------------------------------//
void OSXCarbonWindow::destroy(void)
{
	LogManager::getSingleton().logMessage( "OSXCarbonWindow::destroy()" );
	
	if(mIsFullScreen)
		destroyCGLFullscreen();
		
	WindowEventUtilities::_removeRenderWindow( this );

	if(mWindow)
		DisposeWindow(mWindow);

	mActive = false;

	Root::getSingleton().getRenderSystem()->detachRenderTarget(this->getName());
}

//-------------------------------------------------------------------------------------------------//
bool OSXCarbonWindow::isActive() const
{
	return mActive;
}

//-------------------------------------------------------------------------------------------------//
bool OSXCarbonWindow::isClosed() const
{
	return false;
}

//-------------------------------------------------------------------------------------------------//
void OSXCarbonWindow::reposition(int left, int top)
{
	//LogManager::getSingleton().logMessage( "OSXCarbonWindow::reposition()" );
	if(mWindow)
		MoveWindow(mWindow, left, top, true);
}

//-------------------------------------------------------------------------------------------------//
void OSXCarbonWindow::resize(unsigned int width, unsigned int height)
{
	//LogManager::getSingleton().logMessage( "OSXCarbonWindow::resize()" );
	if(!mWindow)
		return;

	// Check if the window size really changed
	if(mWidth == width && mHeight == height)
		return;

	mWidth = width;
	mHeight = height;

	SizeWindow(mWindow, width, height, true);
}

//-------------------------------------------------------------------------------------------------//
void OSXCarbonWindow::windowHasResized()
{
	mHasResized = true;
	
	// Ensure the context is current
	if(!mIsFullScreen)
		aglSwapBuffers(mAGLContext);
	else
		swapCGLBuffers();
}

//-------------------------------------------------------------------------------------------------//
void OSXCarbonWindow::windowResized()
{
	LogManager::getSingleton().logMessage( "OSXCarbonWindow::windowResized()" );
	
	// Ensure the context is current
	if(!mIsFullScreen)
	{
		aglUpdateContext(mAGLContext);
		::Rect rect;
		GetWindowBounds( mWindow, kWindowContentRgn, &rect );
		mWidth = rect.left + rect.right;
		mHeight = rect.top + rect.bottom;
		mLeft = rect.left;
		mTop = rect.top;
	}
	else
		swapCGLBuffers();
	
	for( ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it )
	{
		( *it ).second->_updateDimensions();
	}
}

//-------------------------------------------------------------------------------------------------//
void OSXCarbonWindow::windowMovedOrResized()
{
	// External windows will call this method.
	if(mView != NULL)
	{
		//update our drawing region
		::Rect ctrlBounds;
		GetControlBounds(mView, &ctrlBounds);
		GLint bufferRect[4];

		bufferRect[0] = ctrlBounds.left; // 0 = left edge
		bufferRect[1] = ctrlBounds.bottom; // 0 = bottom edge
		bufferRect[2] =	ctrlBounds.right - ctrlBounds.left; // width of buffer rect
		bufferRect[3] = ctrlBounds.bottom - ctrlBounds.top; // height of buffer rect
		aglSetInteger(mAGLContext, AGL_BUFFER_RECT, bufferRect);
		aglEnable (mAGLContext, AGL_BUFFER_RECT);
		swapBuffers(true);
	}
	
	for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
	{
	(*it).second->_updateDimensions();
	}
}

//-------------------------------------------------------------------------------------------------//
void OSXCarbonWindow::swapBuffers( bool waitForVSync )
{
	if(!mIsFullScreen)
	{
		if(mAGLContext != aglGetCurrentContext())
			aglSetCurrentContext(mAGLContext);
			
		aglSwapBuffers(mAGLContext);
	}
	else
		swapCGLBuffers();
		
	if(mHasResized)
	{
		windowResized();
		mHasResized = false;
	}
}

//-------------------------------------------------------------------------------------------------//
void OSXCarbonWindow::getCustomAttribute( const String& name, void* pData )
{
	if( name == "GLCONTEXT" ) 
	{
		*static_cast<OSXContext**>(pData) = mContext;
		return;
	}
	else if( name == "WINDOW" )
	{
		if(mIsFullScreen)
		{
			// A fullscreen application uses CGL and thus has no window.
			pData = 0;
			return;
		}
		else
		{
				*static_cast<WindowRef*>(pData) = mWindow;
		}
		return;
	}
}
}
