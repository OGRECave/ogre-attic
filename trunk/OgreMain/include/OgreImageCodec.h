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
#ifndef _ImageCodec_H__
#define _ImageCodec_H__

#include "OgreCodec.h"
#include "OgreCommon.h"

namespace Ogre {

    /** Codec specialized in images.
        @remarks
            The users implementing subclasses of ImageCodec are required to return
            a valid pointer to a ImageData class from the decode(...) function.
    */
    class _OgreExport ImageCodec : public Codec
    {
    public:
        /** Codec return class for images. Has imformation about the size and the
            pixel format of the image. */
        class ImageData : public Codec::CodecData
        {
        public:
            uint height;
            uint width;
            uint size;
            ushort num_mipmaps;
            uint flags;

            PixelFormat format;

        public:
            String dataType() const
            {
                return "ImageData";
            }
        };

    public:
        virtual void code( const DataChunk& input, DataChunk* output, ... ) const = 0;
        virtual CodecData * decode( const DataChunk& input, DataChunk* output, ... ) const = 0;
        /** Encodes data to a file.
        @param input Chunk containing data to write
        @param outFileName Filename to output to (extension implies type)
        @param pData ImageData pointer
        */
        virtual void codeToFile( const DataChunk& input, const String& outFileName, CodecData* pData) const = 0;

        virtual String getType() const = 0;
    };

} // namespace

#endif
