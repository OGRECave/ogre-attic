/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreDataChunk.h"
#include "OgreException.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    DataChunk::DataChunk()
        : mData( NULL ), mSize( 0 )
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

        mData = new uchar[size];
        mSize = size;
        mPos = mData;
        mEnd = mData + size;

        if( ptr )
            memcpy( mData, ptr, mSize );

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
    ulong DataChunk::read(void* buffer, ulong size)
    {
        ulong cnt = size;
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
    ulong DataChunk::readUpTo( void* buffer, size_t size, const char *delim )
    {
        size_t pos = strcspn((const char*)mPos, delim);
        if (pos > size)
            pos = size;

        if (pos > 0)
        {
            memcpy(buffer, (const void*)mPos, pos);
        }
        mPos += pos + 1;

        return static_cast< ulong >( pos );
    }
    //-----------------------------------------------------------------------
    bool DataChunk::isEOF(void)
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
        int count;

        count = readUpTo(buf, 511);
        buf[count] = '\0';
        String ret = buf;
        if (trimAfter)
            ret.trim();
        return ret;

    }

}
