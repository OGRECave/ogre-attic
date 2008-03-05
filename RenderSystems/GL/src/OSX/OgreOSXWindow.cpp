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
#include "OgreOSXCGLContext.h"
#include "OgreRoot.h"
#include "OgreGLRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include <OpenGL/gl.h>
#define GL_EXT_texture_env_combine 1
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>

namespace Ogre
{

//-------------------------------------------------------------------------------------------------//
OSXWindow::OSXWindow() : mCGLContext(NULL)
{
}

//-------------------------------------------------------------------------------------------------//
OSXWindow::~OSXWindow()
{
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
			+ filename + "' - invalid extension.", "OSXCarbonWindow::writeContentsToFile" );

	while( pos != filename.length() - 1 )
		extension += filename[++pos];

	// Get the codec
	Codec * pCodec = Codec::getCodec(extension);

	// Write out
	Codec::CodecDataPtr codecDataPtr(imgData);
	pCodec->codeToFile(streamFlipped, filename, codecDataPtr);

	delete [] pBuffer;
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::createCGLFullscreen(unsigned int width, unsigned int height, unsigned int depth, unsigned int fsaa, CGLContextObj sharedContext)
{
		// Find the best match to what was requested
		boolean_t exactMatch;
		CFDictionaryRef displayMode = CGDisplayBestModeForParameters(kCGDirectMainDisplay, depth, width, height, &exactMatch);
		
		if(!exactMatch)
		{
			// TODO: Report the size difference
			// That mode is not available, using the closest match
			String request = StringConverter::toString(width) + String(" x ") + StringConverter::toString(height) + String(" @ ") + 
				StringConverter::toString(depth) + "bpp. ";
				
			String recieved = StringConverter::toString(width) + String(" x ") + StringConverter::toString(height) + String(" @ ") + 
				StringConverter::toString(depth) + "bpp. "; 
				
			LogManager::getSingleton().logMessage(String("RenderSystem Warning:  You requested a fullscreen mode of ") + request +
				String(" This mode is not available and you will recieve the closest match."));
		}
		
		// Do the fancy display fading
		CGDisplayFadeReservationToken reservationToken;
		CGAcquireDisplayFadeReservation(kCGMaxDisplayReservationInterval,
											&reservationToken);
		CGDisplayFade(reservationToken,
					  0.5,
					  kCGDisplayBlendNormal,
					  kCGDisplayBlendSolidColor,
					  0.0, 0.0, 0.0,
					  true);
		
		// Grab the main display and save it for later.
		// You could render to any display, but picking what display
		// to render to could be interesting.
		CGDisplayCapture(kCGDirectMainDisplay);
		
		// Switch to the correct resolution
		CGDisplaySwitchToMode(kCGDirectMainDisplay, displayMode);
		
		// Get a pixel format that best matches what we are looking for
		CGLPixelFormatAttribute attribs[] = { 
			kCGLPFADoubleBuffer,
			kCGLPFAAlphaSize,     (CGLPixelFormatAttribute)8,
			kCGLPFADepthSize,     (CGLPixelFormatAttribute)depth,
			kCGLPFAStencilSize,   (CGLPixelFormatAttribute)8,
			kCGLPFASampleBuffers, (CGLPixelFormatAttribute)0,
			kCGLPFASamples,       (CGLPixelFormatAttribute)0,
			kCGLPFAFullScreen,
			kCGLPFASingleRenderer,
			kCGLPFAAccelerated,
			kCGLPFADisplayMask,   (CGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
			(CGLPixelFormatAttribute)0
		};
		
		// Set up FSAA if it was requested
		if(fsaa > 1)
		{
				// turn on kCGLPFASampleBuffers
				attribs[8] = (CGLPixelFormatAttribute)1;
				// set the samples for kCGLPFASamples
				attribs[10] = (CGLPixelFormatAttribute)fsaa;
		}
		
        
        CGLError err;
		CGLPixelFormatObj pixelFormatObj;
#if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_4)
        GLint numPixelFormats = 0;
        err = CGLChoosePixelFormat(attribs, &pixelFormatObj, &numPixelFormats);
#else
		long numPixelFormats = 0;
		err = CGLChoosePixelFormat(attribs, &pixelFormatObj, &numPixelFormats);
#endif
		if(err != 0)
		{
			CGReleaseAllDisplays();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, String("CGL Error: " + String(CGLErrorString(err))), "OSXCarbonWindow::create");
		}

		// Create the CGLcontext from our pixel format, share it with the sharedContext passed in
		err = CGLCreateContext(pixelFormatObj, sharedContext, &mCGLContext);
		if(err != 0)
		{
			CGReleaseAllDisplays();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, String("CGL Error: " + String(CGLErrorString(err))), "OSXCarbonWindow::create");
		}
				
		// Once we have the context we can destroy the pixel format
        // In order to share contexts you must keep a pointer to the context object around
        // Our context class will now manage the life of the pixelFormatObj
		//CGLDestroyPixelFormat(pixelFormatObj); 
				
		// Set the context to drawable
		CGLSetFullScreen(mCGLContext);
		
		// Set the context as current
		CGLSetCurrentContext(mCGLContext);
		
		// This synchronizes CGL with the vertical retrace
		// Apple docs suggest that OpenGL blocks rendering calls when waiting for
		// a vertical retrace anyhow.
#if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_4)
        GLint swapInterval = 1;
        CGLSetParameter(mCGLContext, kCGLCPSwapInterval, &swapInterval);
#else
		long swapInterval = 1;
		CGLSetParameter(mCGLContext, kCGLCPSwapInterval, &swapInterval);
#endif
		
		// Give a copy of our context to the rendersystem
		mContext = new OSXCGLContext(mCGLContext, pixelFormatObj);
		
		// Let everyone know we are fullscreen now
		mIsFullScreen = true;
		
		CGDisplayFade(reservationToken,
                  2.0,
                  kCGDisplayBlendSolidColor,
                  kCGDisplayBlendNormal,
                  0.0, 0.0, 0.0,
                  false);
		CGReleaseDisplayFadeReservation(reservationToken);
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::destroyCGLFullscreen(void)
{
	CGReleaseAllDisplays();
}

//-------------------------------------------------------------------------------------------------//
void OSXWindow::swapCGLBuffers(void)
{
	CGLFlushDrawable(mCGLContext);
	CGLContextObj curCtx = CGLGetCurrentContext();
	if(curCtx != mCGLContext)
	{
		CGLSetCurrentContext(mCGLContext);
		CGLSetFullScreen(mCGLContext);
	}
}

}
