/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreStableHeaders.h"

#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreImage.h"
#include "OgreException.h"

#include "OgreEXRCodec.h"

#include "O_IStream.h"

#include <cmath>
#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>

using namespace Imath;
using namespace Imf;

namespace Ogre {

void writeEXRHalf(OStream *ost, const float *pixels,
	      int width, int height, int components) 
{
	//assert(components==3 || components==4);
	// TODO: throw std::exception if invalid number of components

	Header header (width, height);
	header.channels().insert ("R", Channel (HALF));
	header.channels().insert ("G", Channel (HALF));
	header.channels().insert ("B", Channel (HALF));
	if(components==4)
		header.channels().insert ("A", Channel (HALF));

	// Convert data to half
	half data[width*height*components];
	std::copy(pixels, pixels+(width*height*components), data);
	
	// And save it
	OutputFile file (*ost, header);
	FrameBuffer frameBuffer;

	frameBuffer.insert("R",				// name
			    Slice (HALF,		// type
				   ((char *) data)+0,	// base
				   2 * components,		// xStride
				   2 * components * width));	// yStride
	frameBuffer.insert("G",				// name
			    Slice (HALF,		// type
				   ((char *) data)+2,	// base
				   2 * components,		// xStride
				   2 * components * width));	// yStride
	frameBuffer.insert("B",				// name
			    Slice (HALF,		// type
				   ((char *) data)+4,	// base
				   2 * components,		// xStride
				   2 * components * width));	// yStride
	if(components==4) {
		frameBuffer.insert("A",					// name
				    Slice (HALF,			// type
					   ((char *) data)+6,		// base
					   2 * components,		// xStride
					   2 * components * width));	// yStride
	}

	file.setFrameBuffer(frameBuffer);
	file.writePixels(height);
}


EXRCodec::EXRCodec() 
{
    LogManager::getSingleton().logMessage("EXRCodec initialised");
}
EXRCodec::~EXRCodec() 
{
    LogManager::getSingleton().logMessage("EXRCodec deinitialised");
}

void EXRCodec::code( const DataChunk& input, DataChunk* output, ... ) const 
{

}
    
EXRCodec::CodecData * EXRCodec::decode( const DataChunk& input, DataChunk* output, ... ) const 
{
    ImageData * ret_data = new ImageData;

    try {
        // Make a mutable clone of input to be able to change file pointer
        DataChunk myIn(const_cast<uchar*>(input.getPtr()), input.getSize());
    
        // Now we can simulate an OpenEXR file with that
        O_IStream str(myIn, "SomeChunk.exr");
        InputFile file(str);
    
        Box2i dw = file.header().dataWindow();
        int width  = dw.max.x - dw.min.x + 1;
        int height = dw.max.y - dw.min.y + 1;
        int components = 3;
    
        // Alpha channel present?
        const ChannelList &channels = file.header().channels();
        if(channels.findChannel("A"))
            components = 4;
        
        // Allocate memory
        output->allocate(width*height*components*4);
    
        // Construct frame buffer
        uchar *pixels = output->getPtr();
        FrameBuffer frameBuffer;
        frameBuffer.insert("R",             // name
                    Slice (FLOAT,       // type
                       ((char *) pixels)+0, // base
                       4 * components,      // xStride
                    4 * components * width));    // yStride
        frameBuffer.insert("G",             // name
                    Slice (FLOAT,       // type
                       ((char *) pixels)+4, // base
                       4 * components,      // xStride
                    4 * components * width));    // yStride
        frameBuffer.insert("B",             // name
                    Slice (FLOAT,       // type
                       ((char *) pixels)+8, // base
                       4 * components,      // xStride
                    4 * components * width));    // yStride
        if(components==4) {
            frameBuffer.insert("A",                 // name
                        Slice (FLOAT,           // type
                           ((char *) pixels)+12,        // base
                           4 * components,      // xStride
                        4 * components * width));    // yStride
        }
      
        file.setFrameBuffer (frameBuffer);
        file.readPixels (dw.min.y, dw.max.y);
    
        ret_data->format = components==3 ? PF_FP_R32G32B32 : PF_FP_R32G32B32A32;
        ret_data->width = width;
        ret_data->height = height;
        ret_data->depth = 1;
        ret_data->size = width*height*components*4;
        ret_data->num_mipmaps = 1;
        ret_data->flags = 0;
    } catch (const std::exception &exc) {
        delete ret_data;
        throw(Exception(Exception::ERR_INTERNAL_ERROR,
            "OpenEXR Error",
            exc.what()));
    }
    
    return ret_data;
}

void EXRCodec::codeToFile( const DataChunk& input, const String& outFileName, CodecData* pData) const 
{

}


String EXRCodec::getType() const 
{
    return "exr";
}



}
