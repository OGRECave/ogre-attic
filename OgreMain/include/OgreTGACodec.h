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
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef _TGACodec_H__
#define _TGACodec_H__

#include "OgreImageCodec.h"

namespace Ogre {

    /** ImageCodec specialized in Traga images.
    */
    class _OgreExport TGACodec : public ImageCodec
    {
    protected:

    // We're mapping onto raw file data, so ensure members are packed with no gaps 
    //    by using uchars all the way (more portable than #pragma pack
    typedef struct {
	    uchar id_length;
	    uchar color_map_type;
	    uchar image_type;
	    uchar first_entry_index[2];
	    uchar color_map_length[2];
	    uchar color_map_entry_size;
	    uchar x_origin[2];
	    uchar y_origin[2];
	    uchar image_width[2];
	    uchar image_height[2];
	    uchar pixel_depth;
	    uchar image_descriptor;
    } TgaHeader;

    public:
        void code( const DataChunk& input, DataChunk* output, ... ) const;
        CodecData * decode( const DataChunk& input, DataChunk* output, ... ) const;

        String getType() const { return "tga"; }
    };

}

#endif
