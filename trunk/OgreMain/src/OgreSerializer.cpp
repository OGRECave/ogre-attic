/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
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

#include "OgreSerializer.h"
#include "OgreLogManager.h"
#include "OgreDataStream.h"
#include "OgreException.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"


namespace Ogre {

    /// stream overhead = ID + size
    const unsigned long stream_OVERHEAD_SIZE = sizeof(unsigned short) + sizeof(unsigned long);
    const unsigned short HEADER_stream_ID = 0x1000;
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
        
        unsigned short val = HEADER_stream_ID;
        writeShorts(&val, 1);

        writeString(mVersion);

    }
    //---------------------------------------------------------------------
    void Serializer::writeChunkHeader(uint16 id, uint32 size)
    {
        writeShorts(&id, 1);
        writeInts(&size, 1);
    }
    //---------------------------------------------------------------------
    void Serializer::writeReals(const Real* const pReal, size_t count = 1)
    {
#	if OGRE_ENDIAN == ENDIAN_BIG
            Real * pRealToWrite = (Real *)malloc(sizeof(Real) * count);
            memcpy(pRealToWrite, pReal, sizeof(Real) * count);
            
            flipToLittleEndian(pRealToWrite, sizeof(Real), count);
            writeData(pRealToWrite, sizeof(Real), count);
            
            free(pRealToWrite);
# 	else
            writeData(pReal, sizeof(Real), count);
#	endif
    }
    //---------------------------------------------------------------------
    void Serializer::writeShorts(const uint16* const pShort, size_t count = 1)
    {
#	if OGRE_ENDIAN == ENDIAN_BIG
            unsigned short * pShortToWrite = (unsigned short *)malloc(sizeof(unsigned short) * count);
            memcpy(pShortToWrite, pShort, sizeof(unsigned short) * count);
            
            flipToLittleEndian(pShortToWrite, sizeof(unsigned short), count);
            writeData(pShortToWrite, sizeof(unsigned short), count);
            
            free(pShortToWrite);
# 	else
            writeData(pShort, sizeof(unsigned short), count);
#	endif
    }
    //---------------------------------------------------------------------
    void Serializer::writeInts(const uint32* const pInt, size_t count = 1)
    {
#	if OGRE_ENDIAN == ENDIAN_BIG
            unsigned int * pIntToWrite = (unsigned int *)malloc(sizeof(unsigned int) * count);
            memcpy(pIntToWrite, pInt, sizeof(unsigned int) * count);
            
            flipToLittleEndian(pIntToWrite, sizeof(unsigned int), count);
            writeData(pIntToWrite, sizeof(unsigned int), count);
            
            free(pIntToWrite);
# 	else
            writeData(pInt, sizeof(unsigned int), count);
#	endif
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void Serializer::writeBools(const bool* const pBool, size_t count = 1)
    {
    //no endian flipping for 1-byte bools
    //XXX Nasty Hack to convert to 1-byte bools
#	if OGRE_PLATFORM == PLATFORM_APPLE
        char * pCharToWrite = (char *)malloc(sizeof(char) * count);
        for(int i = 0; i < count; i++)
        {
            *(char *)(pCharToWrite + i) = *(bool *)(pBool + i);
        }
        
        writeData(pCharToWrite, sizeof(char), count);
        
        free(pCharToWrite);
#	else
        writeData(pBool, sizeof(bool), count);
#	endif

    }
    
    //---------------------------------------------------------------------
    void Serializer::writeData(const void* const buf, size_t size, size_t count)
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
    void Serializer::readFileHeader(DataStreamPtr& stream)
    {
        unsigned short headerID;
        
        // Read header ID
        readShorts(stream, &headerID, 1);
        
        if (headerID == HEADER_stream_ID)
        {
            // Read version
            String ver = readString(stream);
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
    unsigned short Serializer::readChunk(DataStreamPtr& stream)
    {
        unsigned short id;
        readShorts(stream, &id, 1);
        
        readInts(stream, &mCurrentstreamLen, 1);
        return id;
    }
    //---------------------------------------------------------------------
    void Serializer::readBools(DataStreamPtr& stream, bool* pDest, size_t count)
    {
        //XXX Nasty Hack to convert 1 byte bools to 4 byte bools
#	if OGRE_PLATFORM == PLATFORM_APPLE
        char * pTemp = (char *)malloc(1*count); // to hold 1-byte bools
        stream->read(pTemp, 1 * count);
        for(int i = 0; i < count; i++)
            *(bool *)(pDest + i) = *(char *)(pTemp + i);
            
        free (pTemp);
#	else
        stream->read(pDest, sizeof(bool) * count);
#	endif
        //no flipping on 1-byte datatypes
    }
    //---------------------------------------------------------------------
    void Serializer::readReals(DataStreamPtr& stream, Real* pDest, size_t count)
    {
        stream->read(pDest, sizeof(Real) * count);
        flipFromLittleEndian(pDest, sizeof(Real), count);
    }
    //---------------------------------------------------------------------
    void Serializer::readShorts(DataStreamPtr& stream, unsigned short* pDest, size_t count)
    {
        stream->read(pDest, sizeof(unsigned short) * count);
        flipFromLittleEndian(pDest, sizeof(unsigned short), count);
    }
    //---------------------------------------------------------------------
    void Serializer::readInts(DataStreamPtr& stream, unsigned int* pDest, size_t count)
    {
        stream->read(pDest, sizeof(unsigned int) * count);
        flipFromLittleEndian(pDest, sizeof(unsigned int), count);
    }
    //---------------------------------------------------------------------
    String Serializer::readString(DataStreamPtr& stream, size_t numChars)
    {
        assert (numChars <= 255);
        char str[255];
        stream->read(str, numChars);
        str[numChars] = '\0';
        return str;
    }
    //---------------------------------------------------------------------
    String Serializer::readString(DataStreamPtr& stream)
    {
        return stream->getLine(false);
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
    void Serializer::readObject(DataStreamPtr& stream, Vector3* pDest)
    {
        readReals(stream, &pDest->x, 1);
        readReals(stream, &pDest->y, 1);
        readReals(stream, &pDest->z, 1);
    }
    //---------------------------------------------------------------------
    void Serializer::readObject(DataStreamPtr& stream, Quaternion* pDest)
    {
        readReals(stream, &pDest->x, 1);
        readReals(stream, &pDest->y, 1);
        readReals(stream, &pDest->z, 1);
        readReals(stream, &pDest->w, 1);
    }
    //---------------------------------------------------------------------


    void Serializer::flipToLittleEndian(void* pData, size_t size, size_t count)
    {
#	if OGRE_ENDIAN == ENDIAN_BIG
        flipEndian(pData, size, count);
#	endif
    }
    
    void Serializer::flipFromLittleEndian(void* pData, size_t size, size_t count)
    {
#	if OGRE_ENDIAN == ENDIAN_BIG
        flipEndian(pData, size, count);
#	endif
    }
    
    void Serializer::flipEndian(void * pData, size_t size, size_t count)
    {
        for(unsigned int index = 0; index < count; index++)
        {
            flipEndian((void *)((int)pData + (index * size)), size);
        }
    }
    
    void Serializer::flipEndian(void * pData, size_t size)
    {
        char swapByte;
        for(unsigned int byteIndex = 0; byteIndex < size/2; byteIndex++)
        {
            swapByte = *(char *)((int)pData + byteIndex);
            *(char *)((int)pData + byteIndex) = *(char *)((int)pData + size - byteIndex - 1);
            *(char *)((int)pData + size - byteIndex - 1) = swapByte;
        }
    }
    
}

