/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
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
#include "OgreStableHeaders.h"
#include "OgreDataChunk.h"
#include "OgreException.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    DataChunk::DataChunk()
        : mData(NULL), mPos(NULL), mEnd(NULL), mSize(0)
    {
    }

    DataChunk::DataChunk( void *pData, size_t size )
    {
        mData = reinterpret_cast< uchar* >( pData );
        mEnd = mData + size;
        mPos = mData;
        mSize = size;
    }

    //-----------------------------------------------------------------------
    uchar* DataChunk::allocate( size_t size, const uchar * ptr )
    {
        assert (size > 0);

        if( mData )
            delete [] mData;
        mSize = size;
        // Always allocate 1 additional byte, which we set to zero
        // This is to ensure that string chunks are always null-terminated
        mData = new uchar[mSize + 1];
        mPos = mData;
        mEnd = mData + mSize;

        if( ptr )
            memcpy( mData, ptr, size );

        // Add null terminator to the end (just after where chunk reports that it ends)
        mData[mSize] = '\0';

        return mData;
    }

    //-----------------------------------------------------------------------
    DataChunk & DataChunk::clear(void)
    {
        if (mData)
        {
            delete [] mData;
            mData = 0;
            mSize = 0;
        }

        return *this;
    }
    //-----------------------------------------------------------------------
    size_t DataChunk::getSize(void) const
    {
        return mSize;
    }

    //-----------------------------------------------------------------------
    uchar * DataChunk::getPtr(void)
    {
        return mData;
    }

    //-----------------------------------------------------------------------
    const uchar * DataChunk::getPtr() const
    {
        return mData;
    }
    //-----------------------------------------------------------------------
    uchar * DataChunk::getCurrentPtr(void)
    {
        return mPos;
    }

    //-----------------------------------------------------------------------
    const uchar * DataChunk::getCurrentPtr() const
    {
        return mPos;
    }

    //-----------------------------------------------------------------------
    size_t DataChunk::read(void* buffer, size_t size)
    {
        size_t cnt = size;
        // Read over end of memory?
        if (mPos + size > mEnd)
            cnt = mEnd - mPos;
        if (cnt == 0)
            return 0;

        memcpy(buffer, (const void*)mPos, cnt);
        mPos += cnt;
        return cnt;
    }

    //-----------------------------------------------------------------------
    DataChunk & DataChunk::seek( size_t pos )
    {
        if( pos <= mSize )
            mPos = mData + pos;

        return *this;
    }
    //-----------------------------------------------------------------------
    DataChunk & DataChunk::skip( long offset )
    {
        size_t newpos = (size_t)( ( mPos - mData ) + offset );
        assert( mData <= mData + newpos && mData + newpos <= mEnd );        

        mPos = mData + newpos;

        return *this;
    }
    //-----------------------------------------------------------------------
    size_t DataChunk::readUpTo( void* buffer, size_t size, const char *delim )
    {
        size_t pos = strcspn((const char*)mPos, delim);
        if (pos > size)
            pos = size;

        // Make sure pos can never go past the end of the data 
        if(mPos + pos > mEnd) pos = mEnd - mPos; 

        if (pos > 0)
        {
            memcpy(buffer, (const void*)mPos, pos);
        }
        mPos += pos + 1;

        return pos;
    }
    //-----------------------------------------------------------------------
    size_t DataChunk::skipUpTo( const char *delim )
    {

        size_t pos = strcspn( (const char*)mPos, delim );

        // Make sure pos can never go past the end of the data 
        if(mPos + pos > mEnd) pos = mEnd - mPos; 

        mPos += pos + 1;

        return pos;
    }
    //-----------------------------------------------------------------------
    bool DataChunk::isEOF(void) const
    {
        if (mPos >= mEnd)
            return true;
        else
            return false;

    }
    //-----------------------------------------------------------------------
    String DataChunk::getLine(bool trimAfter)
    {
        static char buf[512]; // prevent continuous allocation / deallocation - not thread safe!

        size_t count = readUpTo(buf, 511);
        buf[count] = '\0';
        String ret = buf;
        if (trimAfter)
            StringUtil::trim(ret);
        return ret;

    }
    //-----------------------------------------------------------------------
    String DataChunk::getAsString(void) const
    {
        String s;
		// Insert n characters since we can't expect mData to be null-terminated
		s.insert(0, (const char*)mData, mSize);
		return s;
    }

}
