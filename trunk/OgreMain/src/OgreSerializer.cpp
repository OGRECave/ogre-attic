/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/

#include "OgreSerializer.h"
#include "OgreLogManager.h"
#include "OgreDataChunk.h"
#include "OgreException.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"


namespace Ogre {

    /// Chunk overhead = ID + size
    const unsigned long CHUNK_OVERHEAD_SIZE = sizeof(unsigned short) + sizeof(unsigned long);
    const unsigned short HEADER_CHUNK_ID = 0x1000;
    //---------------------------------------------------------------------
    Serializer::Serializer()
    {
        // Version number
        mVersion = "[Serializer_v1.00]";
    }
    //---------------------------------------------------------------------
    Serializer::~Serializer()
    {
    }
    //---------------------------------------------------------------------
    void Serializer::writeFileHeader(void)
    {
       
        unsigned short val = HEADER_CHUNK_ID;
        writeData(&val, sizeof(unsigned short), 1);

        writeString(mVersion);

    }
    //---------------------------------------------------------------------
    void Serializer::writeChunkHeader(unsigned short id, unsigned long size)
    {
        writeData(&id, sizeof(unsigned short), 1);
        writeData(&size, sizeof(unsigned long), 1);
    }
    //---------------------------------------------------------------------
    void Serializer::writeReals(const Real* pReal, unsigned short count)
    {
        writeData(pReal, sizeof(Real), count);
    }
    //---------------------------------------------------------------------
    void Serializer::writeShorts(const unsigned short* pShort, unsigned short count)
    {
        writeData(pShort, sizeof(unsigned short), count);
    }
    //---------------------------------------------------------------------
    void Serializer::writeLongs(const unsigned long* pLong, unsigned short count)
    {
        writeData(pLong, sizeof(unsigned long), count);
    }
    //---------------------------------------------------------------------
    void Serializer::writeData(const void* buf, size_t size, size_t count)
    {
        fwrite((void* const)buf, size, count, mpfFile);
    }
    //---------------------------------------------------------------------
    void Serializer::writeString(const String& string)
    {
        fputs(string.c_str(), mpfFile);
        // Write terminating newline char
        fputc('\n', mpfFile);
    }
    //---------------------------------------------------------------------
    void Serializer::readFileHeader(DataChunk& chunk)
    {
        unsigned short headerID;
        
        // Read header ID
        chunk.read(&headerID, sizeof(headerID));
        if (headerID == HEADER_CHUNK_ID)
        {
            // Read version
            String ver = readString(chunk);
            if (ver != mVersion)
            {
                Except(Exception::ERR_INTERNAL_ERROR, 
                    "Invalid file: version incompatible, file reports " + String(ver) +
                    " Serializer is version " + mVersion,
                    "Serializer::readFileHeader");
            }
        }
        else
        {
            Except(Exception::ERR_INTERNAL_ERROR, "Invalid file: no header", 
                "Serializer::readFileHeader");
        }

    }
    //---------------------------------------------------------------------
    unsigned short Serializer::readChunk(DataChunk& chunk)
    {
        unsigned short id;
        chunk.read(&id, sizeof(id));
        chunk.read(&mCurrentChunkLen, sizeof(mCurrentChunkLen));
        return id;
    }
    //---------------------------------------------------------------------
    void Serializer::readReals(DataChunk& chunk, Real* pDest, unsigned short count)
    {
        chunk.read(pDest, sizeof(Real) * count);
    }
    //---------------------------------------------------------------------
    void Serializer::readShorts(DataChunk& chunk, unsigned short* pDest, unsigned short count)
    {
        chunk.read(pDest, sizeof(unsigned short) * count);
    }
    //---------------------------------------------------------------------
    void Serializer::readLongs(DataChunk& chunk, unsigned long* pDest, unsigned short count) 
    {
        chunk.read(pDest, sizeof(unsigned long) * count);
    }
    //---------------------------------------------------------------------
    String Serializer::readString(DataChunk& chunk)
    {
        char str[255];
        int readcount;
        readcount = chunk.readUpTo(str, 255);
        str[readcount] = '\0';
        return str;

    }
    //---------------------------------------------------------------------
    void Serializer::writeObject(const Vector3& vec)
    {
        writeReals(&vec.x, 1);
        writeReals(&vec.y, 1);
        writeReals(&vec.z, 1);

    }
    //---------------------------------------------------------------------
    void Serializer::writeObject(const Quaternion& q)
    {
        writeReals(&q.x, 1);
        writeReals(&q.y, 1);
        writeReals(&q.z, 1);
        writeReals(&q.w, 1);
    }
    //---------------------------------------------------------------------
    void Serializer::readObject(DataChunk& chunk, Vector3* pDest)
    {
        readReals(chunk, &pDest->x, 1);
        readReals(chunk, &pDest->y, 1);
        readReals(chunk, &pDest->z, 1);
    }
    //---------------------------------------------------------------------
    void Serializer::readObject(DataChunk& chunk, Quaternion* pDest)
    {
        readReals(chunk, &pDest->x, 1);
        readReals(chunk, &pDest->y, 1);
        readReals(chunk, &pDest->z, 1);
        readReals(chunk, &pDest->w, 1);
    }
    //---------------------------------------------------------------------


}

