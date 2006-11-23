#include "OgreStableHeaders.h"

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"

#include "OgreGLRenderSystem.h"

#include "OgreOSXRenderTexture.h"
#include "OgreOSXContext.h"

namespace Ogre
{
	OSXPBuffer::OSXPBuffer( PixelComponentType format, size_t width, size_t height ) : GLPBuffer( format, width, height ), mContext( NULL )
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::OSXPBuffer()" );
		createPBuffer();
		// Create context
		mContext = new OSXContext( mAGLContext );
    }
	
	OSXPBuffer::~OSXPBuffer()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::~OSXPBuffer()" );
		delete mContext;
		destroyPBuffer();
	}

	GLContext* OSXPBuffer::getContext()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::getContext()" );
		return mContext;
	}
	
	void OSXPBuffer::createPBuffer()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::createPBuffer()" );
		mAGLContext = aglGetCurrentContext();
		aglCreatePBuffer( mWidth, mHeight, GL_TEXTURE_2D, GL_RGBA, 0, &mPBuffer );
		GLint vs = aglGetVirtualScreen( mAGLContext );
		aglSetPBuffer( mAGLContext, mPBuffer, 0, 0, vs ); 
	}
	
	void OSXPBuffer::destroyPBuffer()
	{
		LogManager::getSingleton().logMessage( "OSXPBuffer::destroyPBuffer()" );
		aglDestroyPBuffer( mPBuffer );
	}
}

