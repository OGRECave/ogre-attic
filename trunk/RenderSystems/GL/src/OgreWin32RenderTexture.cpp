/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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

#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"

#include "OgreWin32RenderTexture.h"
#include "OgreWin32GLSupport.h"
#include "OgreWin32Context.h"

// ATI float extension
#ifndef WGL_ATI_pixel_format_float
#define WGL_ATI_pixel_format_float  1

#define WGL_TYPE_RGBA_FLOAT_ATI             0x21A0
#define GL_TYPE_RGBA_FLOAT_ATI              0x8820
#define GL_COLOR_CLEAR_UNCLAMPED_VALUE_ATI  0x8835
#endif


namespace Ogre {

	Win32RenderTexture::Win32RenderTexture(Win32GLSupport &glsupport, const String & name, 
			unsigned int width, unsigned int height,
			TextureType texType, PixelFormat internalFormat, 
			const NameValuePairList *miscParams, bool useBind ):
		GLRenderTexture(name, width, height, texType, internalFormat, miscParams),
		mGLSupport(glsupport),
        mContext(0), mUseBind(useBind)
	{
	
		createPBuffer();

        // Create context
        mContext = new Win32Context(mHDC, mGlrc);
        // Register the context with the rendersystem
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_registerContext(this, mContext);        

		if(mUseBind)
		{
			// Bind texture
			glBindTexture(GL_TEXTURE_2D, static_cast<GLTexture*>(mTexture.get())->getGLID());
			wglBindTexImageARB(mPBuffer, WGL_FRONT_LEFT_ARB);
		}
	}
	Win32RenderTexture::~Win32RenderTexture() 
	{
		if(mUseBind)
		{
			// Unbind texture
			glBindTexture(GL_TEXTURE_2D,
				static_cast<GLTexture*>(mTexture.get())->getGLID());
			glBindTexture(GL_TEXTURE_2D,
				static_cast<GLTexture*>(mTexture.get())->getGLID());
			wglReleaseTexImageARB(mPBuffer, WGL_FRONT_LEFT_ARB);
		}
           
        // Unregister and destroy mContext
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_unregisterContext(this);
        delete mContext;        
           
		// Destroy pbuffer
		destroyPBuffer();
	}
	
	void Win32RenderTexture::_copyToTexture() 
	{
		if(!mUseBind)
		{
			// Use old fasioned copying
			GLRenderTexture::_copyToTexture();
		}
	}
 /*
	void Win32RenderTexture::firePreUpdate(void) 
	{
		//SwapBuffers(mHDC);
		// Enable current context
		mGLSupport.pushContext(mHDC, mGlrc);
		// Fire default preupdate
		GLRenderTexture::firePreUpdate();
	}
	void Win32RenderTexture::firePostUpdate(void) 
	{
		// Fire default postupdate
		GLRenderTexture::firePostUpdate();
		// Possibly enable previous context
		mGLSupport.popContext();
	}
*/
	void Win32RenderTexture::createPBuffer() 
	{
		LogManager::getSingleton().logMessage(
	    "Win32RenderTexture::Creating PBuffer"
	    );

		HDC old_hdc = wglGetCurrentDC();
		HGLRC old_context = wglGetCurrentContext();

		// Bind to RGB or RGBA texture
		int bttype = 0;
		if(mUseBind)
		{
			// Only provide bind type when actually binding
			bttype = PixelUtil::hasAlpha(mInternalFormat)?
				WGL_BIND_TO_TEXTURE_RGBA_ARB : WGL_BIND_TO_TEXTURE_RGB_ARB;
		}
		int texformat = PixelUtil::hasAlpha(mInternalFormat)?
			WGL_TEXTURE_RGBA_ARB : WGL_TEXTURE_RGB_ARB;
		// Make a float buffer?
        int pixeltype = PixelUtil::isFloatingPoint(mInternalFormat)?
			WGL_TYPE_RGBA_FLOAT_ATI : WGL_TYPE_RGBA_ARB;
		// Get R,G,B,A depths
		int depths[4];
		PixelUtil::getBitDepths(mInternalFormat, depths);

		int attrib[] = {
			WGL_RED_BITS_ARB,depths[0],
			WGL_GREEN_BITS_ARB,depths[1],
			WGL_BLUE_BITS_ARB,depths[2],
			WGL_ALPHA_BITS_ARB,depths[3],
			WGL_STENCIL_BITS_ARB,1,
			WGL_DEPTH_BITS_ARB,15,
			WGL_DRAW_TO_PBUFFER_ARB,true,
			WGL_SUPPORT_OPENGL_ARB,true,
			WGL_PIXEL_TYPE_ARB,pixeltype,
			//WGL_DOUBLE_BUFFER_ARB,true,
			//WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB, // Make sure it is accelerated
			bttype,true, // must be last, as bttype can be zero
			0
		};
		int pattrib[] = { 
			WGL_TEXTURE_FORMAT_ARB, texformat, 
			WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,
			0 
		};

		int format;
		unsigned int count;

		// Choose suitable pixel format
		wglChoosePixelFormatARB(old_hdc,attrib,NULL,1,&format,&count);
		if(count == 0)
			OGRE_EXCEPT(0, "wglChoosePixelFormatARB() failed", "Win32RenderTexture::createPBuffer");

		// Analyse pixel format
		const int piAttributes[]={
				WGL_RED_BITS_ARB,WGL_GREEN_BITS_ARB,WGL_BLUE_BITS_ARB,WGL_ALPHA_BITS_ARB,
				WGL_DEPTH_BITS_ARB,WGL_STENCIL_BITS_ARB
		};
		int piValues[sizeof(piAttributes)/sizeof(const int)];
		wglGetPixelFormatAttribivARB(old_hdc,format,0,sizeof(piAttributes)/sizeof(const int),piAttributes,piValues);

        StringUtil::StrStreamType str;
        str << "Win32RenderTexture::PBuffer -- Chosen pixel format rgba="
            << piValues[0] << ","  
            << piValues[1] << ","  
            << piValues[2] << ","  
            << piValues[3] 
            << " depth=" << piValues[4]
            << " stencil=" << piValues[5];
		LogManager::getSingleton().logMessage(
			LML_NORMAL, str.str());

		mPBuffer = wglCreatePbufferARB(old_hdc,format,mWidth,mHeight,pattrib);
		if(!mPBuffer)
			OGRE_EXCEPT(0, "wglCreatePbufferARB() failed", "Win32RenderTexture::createPBuffer");

		mHDC = wglGetPbufferDCARB(mPBuffer);
		if(!mHDC) {
			wglDestroyPbufferARB(mPBuffer);
			OGRE_EXCEPT(0, "wglGetPbufferDCARB() failed", "Win32RenderTexture::createPBuffer");
		}
			
		mGlrc = wglCreateContext(mHDC);
		if(!mGlrc) {
			wglReleasePbufferDCARB(mPBuffer,mHDC);
			wglDestroyPbufferARB(mPBuffer);
			OGRE_EXCEPT(0, "wglCreateContext() failed", "Win32RenderTexture::createPBuffer");
		}

		if(!wglShareLists(old_context,mGlrc)) {
			wglDeleteContext(mGlrc);
			wglReleasePbufferDCARB(mPBuffer,mHDC);
			wglDestroyPbufferARB(mPBuffer);
			OGRE_EXCEPT(0, "wglShareLists() failed", "Win32RenderTexture::createPBuffer");
		}
				
		// Query real width and height
		int iWidth, iHeight;
		wglQueryPbufferARB(mPBuffer, WGL_PBUFFER_WIDTH_ARB, &iWidth);
		wglQueryPbufferARB(mPBuffer, WGL_PBUFFER_HEIGHT_ARB, &iHeight);
        str.clear();
        str << "Win32RenderTexture::PBuffer created -- Real dimensions "
            << mWidth << "x" << mHeight;
		LogManager::getSingleton().logMessage(LML_NORMAL, str.str());
		mWidth = iWidth;  
		mHeight = iHeight;
	}
	void Win32RenderTexture::destroyPBuffer() 
	{
		wglDeleteContext(mGlrc);
		wglReleasePbufferDCARB(mPBuffer,mHDC);
		wglDestroyPbufferARB(mPBuffer);
	}


}
