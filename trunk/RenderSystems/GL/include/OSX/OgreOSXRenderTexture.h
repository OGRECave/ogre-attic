#ifndef __OSXRenderTexture_H__
#define __OSXRenderTexture_H__

#include "OgrePrerequisites.h"

#include "OgreGLPBuffer.h"
#include "OgreOSXContext.h"

namespace Ogre
{
    class OSXPBuffer : public GLPBuffer
    {
    public:
        OSXPBuffer( PixelComponentType format, size_t width, size_t height );
        ~OSXPBuffer();
        
        virtual GLContext *getContext();

    protected:
        void createPBuffer();
		void destroyPBuffer();

	private:
		AGLPbuffer mPBuffer;
        Ogre::OSXContext* mContext;
		AGLContext mAGLContext;
    };
}
#endif

