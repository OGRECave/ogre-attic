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

#ifndef __Serializer_H__
#define __Serializer_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"

namespace Ogre {

    /** Generic class for serialising data to / from binary stream-based files.
    @remarks
        This class provides a number of useful methods for exporting / importing data
        from stream-oriented binary files (e.g. .mesh and .skeleton).
    */
    class _OgreExport Serializer
    {
    public:
        Serializer();
        virtual ~Serializer();


    protected:

        unsigned long mCurrentstreamLen;
        FILE* mpfFile;
        String mVersion;

        // Internal methods
        virtual void writeFileHeader(void);
        virtual void writeChunkHeader(unsigned short id, unsigned long size);
        
        void writeReals(const Real* pReal, size_t count);
        void writeShorts(const unsigned short* pShort, size_t count);
        void writeInts(const unsigned int* pInt, size_t count); 
        void writeLongs(const unsigned long* pLong, size_t count); 
        void writeBools(const bool* pLong, size_t count);
        void writeObject(const Vector3& vec);
        void writeObject(const Quaternion& q);
        
        void writeString(const String& string);
        void writeData(const void* buf, size_t size, size_t count);
        
        virtual void readFileHeader(DataStream& stream);
        virtual unsigned short readChunk(DataStream& stream);
        
        void readBools(DataStream& stream, bool* pDest, size_t count);
        void readReals(DataStream& stream, Real* pDest, size_t count);
        void readShorts(DataStream& stream, unsigned short* pDest, size_t count);
        void readInts(DataStream& stream, unsigned int* pDest, size_t count);
        void readLongs(DataStream& stream, unsigned long* pDest, size_t count); 
        void readObject(DataStream& stream, Vector3* pDest);
        void readObject(DataStream& stream, Quaternion* pDest);

        String readString(DataStream& stream);
        String readString(DataStream& stream, size_t numChars);
        
        virtual void flipToLittleEndian(void* pData, size_t size, size_t count = 1);
        virtual void flipFromLittleEndian(void* pData, size_t size, size_t count = 1);
        
        virtual void flipEndian(void * pData, size_t size, size_t count);
        virtual void flipEndian(void * pData, size_t size);
    };

}


#endif
