/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
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
#include "OgreGLHardwareIndexBuffer.h"
#include "OgreException.h"

namespace Ogre {

	//---------------------------------------------------------------------
    GLHardwareIndexBuffer::GLHardwareIndexBuffer(IndexType idxType,
        size_t numIndexes, HardwareBuffer::Usage usage)
        : HardwareIndexBuffer(idxType, numIndexes, usage)
    {
        glGenBuffersARB( 1, &mBufferId );

        if (!mBufferId)
        {
            Except(Exception::ERR_INTERNAL_ERROR, 
                "Cannot create GL index buffer", 
                "GLHardwareIndexBuffer::GLHardwareIndexBuffer");
        }
    }
	//---------------------------------------------------------------------
    GLHardwareIndexBuffer::~GLHardwareIndexBuffer()
    {
        glDeleteBuffersARB(1, &mBufferId);
    }
	//---------------------------------------------------------------------
    void* GLHardwareIndexBuffer::lock(size_t offset, 
        size_t length, LockOptions options)
    {
        GLenum access = 0;
        
        if(options == HBL_DISCARD)
        {
            if(mUsage != HBU_DYNAMIC)
            {
                Except(Exception::ERR_INTERNAL_ERROR, 
                    "HBL_DISCARD is not allowed on a non-dynamic buffer",
                        "GLHardwareIndexBuffer::lock");
            }

            glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, length, NULL, 
                GL_STREAM_DRAW_ARB);

            access = (mUsage == HBU_WRITE_ONLY) ? GL_WRITE_ONLY_ARB : GL_READ_WRITE_ARB;

        }
        else if(options == HBL_READ_ONLY)
        {
            if(mUsage == HBU_WRITE_ONLY)
            {
                Except(Exception::ERR_INTERNAL_ERROR, 
                    "Invalid attempt to lock a write-only vertex buffer as read-only",
                    "GLHardwareIndexBuffer::lock");
            }
            access = GL_READ_ONLY_ARB;
        }
        else if(options == HBL_NORMAL)
        {
            access = (mUsage == HBU_WRITE_ONLY) ? GL_WRITE_ONLY_ARB : GL_READ_WRITE_ARB;
        }
        else
        {
            Except(Exception::ERR_INTERNAL_ERROR, 
                "Invalid locking option set", "GLHardwareIndexBuffer::lock");
        }

        void* pBuffer = 
          glMapBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, access );

        if(pBuffer == NULL)
        {
            Except(Exception::ERR_INTERNAL_ERROR, 
                "Index Buffer: Out of memory", 
                "GLHardwareIndexBuffer::lock");
        }

        mIsLocked = true;
        return pBuffer;
    }
	//---------------------------------------------------------------------
	void GLHardwareIndexBuffer::unlock(void)
    {
        if(!glUnmapBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB ))
        {
            Except(Exception::ERR_INTERNAL_ERROR, 
                "Buffer data corrupted, please reload", 
                "GLHardwareIndexBuffer::unlock");
        }

        mIsLocked = false;
    }
	//---------------------------------------------------------------------
    void GLHardwareIndexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
        if(mUsage == HBU_STATIC)
        {
            glGetBufferSubDataARB(mBufferId, offset, length, pDest);
        }
        else
        {
            void* pSrc = this->lock(offset, length, 
                HardwareBuffer::HBL_READ_ONLY);
            memcpy(pDest, pSrc, length);
            this->unlock();
        }
    }
	//---------------------------------------------------------------------
    void GLHardwareIndexBuffer::writeData(size_t offset, size_t length, 
            const void* pSource,
			bool discardWholeBuffer)
    {
        glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, mBufferId );

        if(mUsage == HBU_STATIC)
        {
            glBufferDataARB( GL_ELEMENT_ARRAY_BUFFER_ARB, 
                mNumIndexes*3*sizeof(GL_FLOAT), pSource, GL_STATIC_DRAW_ARB);
        }
        else
        {
            void* pDst = this->lock(offset, length, 
                discardWholeBuffer ? HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NORMAL);
            memcpy(pDst, pSource, length);
            this->unlock();
        }
        
    }
	//---------------------------------------------------------------------

}
