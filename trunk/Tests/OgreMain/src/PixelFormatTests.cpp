/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2005 The OGRE Team
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
#include "PixelFormatTests.h"
#include "OgrePixelFormat.h"
#include <cstdlib>
// Register the suite
CPPUNIT_TEST_SUITE_REGISTRATION( PixelFormatTests );

void PixelFormatTests::setUp()
{
    size = 4096;
    randomData = new uint8[size];
    temp = new uint8[size];
    temp2 = new uint8[size];
    // Generate reproducable random data
    srand(0);
    for(unsigned int x=0; x<size; x++)
        randomData[x] = (uint8)rand();
}

void PixelFormatTests::tearDown()
{
    delete [] randomData;
    delete [] temp;
    delete [] temp2;
}


void PixelFormatTests::testIntegerPackUnpack()
{
    
}

void PixelFormatTests::testFloatPackUnpack()
{
    // Float32
    float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float r,g,b,a;
    PixelUtil::unpackColour(&r, &g, &b, &a, PF_FP_R32G32B32A32, data);
    CPPUNIT_ASSERT_EQUAL(r, 1.0f);
    CPPUNIT_ASSERT_EQUAL(g, 2.0f);
    CPPUNIT_ASSERT_EQUAL(b, 3.0f);
    CPPUNIT_ASSERT_EQUAL(a, 4.0f);
    
    // Float16
    setupBoxes(PF_A8B8G8R8, PF_FP_R16G16B16A16);
    dst2.format = PF_A8B8G8R8;
    unsigned int eob = src.width*4;
    
    PixelUtil::bulkPixelConversion(src, dst1);
    PixelUtil::bulkPixelConversion(dst1, dst2);
    
    // Locate errors
    std::stringstream s;
    int x;
    for(x=0; x<eob; x++) {
        if(temp2[x] != randomData[x]) 
            s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) randomData[x]
              << "!= " << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) temp2[x] << " ";
    }

    // src and dst2 should match    
    CPPUNIT_ASSERT_MESSAGE("PF_FP_R16G16B16A16<->PF_A8B8G8R8 conversion was not lossless "+s.str(), 
        memcmp(src.data, dst2.data, eob) == 0);
}

// Pure 32 bit float precision brute force pixel conversion; for comparision
void naiveBulkPixelConversion(const PixelBox &src, const PixelBox &dst)
{
    uint8 *srcptr = static_cast<uint8*>(src.data);
    uint8 *dstptr = static_cast<uint8*>(dst.data);
    unsigned int srcPixelSize = PixelUtil::getNumElemBytes(src.format);
    unsigned int dstPixelSize = PixelUtil::getNumElemBytes(dst.format);

    // Calculate pitches+skips in bytes
    int srcRowSkipBytes = src.getRowSkip()*srcPixelSize;
    int srcSliceSkipBytes = src.getSliceSkip()*srcPixelSize;

    int dstRowSkipBytes = dst.getRowSkip()*dstPixelSize;
    int dstSliceSkipBytes = dst.getSliceSkip()*dstPixelSize;

    float r,g,b,a;
    for(unsigned int z=0; z<src.depth; z++) 
    {
        for(unsigned int y=0; y<src.height; y++)
        {
            for(unsigned int x=0; x<src.width; x++)
            {
                PixelUtil::unpackColour(&r, &g, &b, &a, src.format, srcptr);
                PixelUtil::packColour(r, g, b, a, dst.format, dstptr);
                srcptr += srcPixelSize; 
                dstptr += dstPixelSize;
            }
            srcptr += srcRowSkipBytes;
            dstptr += dstRowSkipBytes;
        }
        srcptr += srcSliceSkipBytes;
        dstptr += dstSliceSkipBytes;
    }
}

void PixelFormatTests::setupBoxes(PixelFormat srcFormat, PixelFormat dstFormat)
{
    unsigned int width = (size-4) / PixelUtil::getNumElemBytes(srcFormat);
    unsigned int width2 = (size-4) / PixelUtil::getNumElemBytes(dstFormat);
    if(width > width2)
        width = width2;

    src.data = randomData;
    src.format = srcFormat;
    src.width = width;
    src.height = 1;
    src.depth = 1;
    src.setConsecutive();

    dst1.data = temp;
    dst1.format = dstFormat;
    dst1.width = width;
    dst1.height = 1;
    dst1.depth = 1;
    dst1.setConsecutive();

    dst2.data = temp2;
    dst2.format = dstFormat;
    dst2.width = width;
    dst2.height = 1;
    dst2.depth = 1;
    dst2.setConsecutive();
  
}

void PixelFormatTests::testCase(PixelFormat srcFormat, PixelFormat dstFormat)
{
    setupBoxes(srcFormat, dstFormat);
    // Check end of buffer
    unsigned int eob = dst1.width*PixelUtil::getNumElemBytes(dstFormat);
    temp[eob] = (unsigned char)0x56;
    temp[eob+1] = (unsigned char)0x23;

    //std::cerr << "["+PixelUtil::getFormatName(srcFormat)+"->"+PixelUtil::getFormatName(dstFormat)+"]" << " " << eob << std::endl;

    // Do pack/unpacking with both naive and optimized version
    PixelUtil::bulkPixelConversion(src, dst1);
    naiveBulkPixelConversion(src, dst2);
    
    CPPUNIT_ASSERT_EQUAL(temp[eob], (unsigned char)0x56);
    CPPUNIT_ASSERT_EQUAL(temp[eob+1], (unsigned char)0x23);

    std::stringstream s;
    int x;
    s << "src=";
    for(x=0; x<16; x++)
        s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) randomData[x];
    s << " dst=";
    for(x=0; x<16; x++)
        s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) temp[x];
    s << " dstRef=";
    for(x=0; x<16; x++)
        s << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) temp2[x];
    s << " ";
    
    // Compare result
    CPPUNIT_ASSERT_MESSAGE("Conversion mismatch ["+PixelUtil::getFormatName(srcFormat)+"->"+PixelUtil::getFormatName(dstFormat)+"] "+s.str(), 
        memcmp(dst1.data, dst2.data, eob) == 0);
}
 
void PixelFormatTests::testBulkConversion()
{
    // Self match
    testCase(PF_A8R8G8B8, PF_A8R8G8B8);
    // Optimized
    testCase(PF_A8R8G8B8, PF_A8B8G8R8);
    testCase(PF_A8B8G8R8, PF_A8R8G8B8);
    testCase(PF_B8G8R8A8, PF_A8B8G8R8);
    testCase(PF_A8B8G8R8, PF_B8G8R8A8);
    testCase(PF_A8B8G8R8, PF_L8);
    testCase(PF_L8, PF_A8B8G8R8);
    testCase(PF_A8R8G8B8, PF_L8);
    testCase(PF_L8, PF_A8R8G8B8);
    testCase(PF_B8G8R8A8, PF_L8);
    testCase(PF_L8, PF_B8G8R8A8);
    testCase(PF_L8, PF_L16);
    testCase(PF_L16, PF_L8);
    //CPPUNIT_ASSERT_MESSAGE("Conversion mismatch", false);
}

