/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright � 2000-2001 Steven J. Streeting
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
#ifndef _ImageCodec_H__
#define _ImageCodec_H__

#include "OgreCodec.h"
#include "OgreImage.h"

BEGIN_OGRE_NAMESPACE

/** Codec specialized in images.
    @remarks
        The users implementing subclasses of ImageCodec are required to return
        a valid pointer to a ImageData class from the decode(...) function.
*/
class _OgreExport ImageCodec : public Codec
{
public:
    /** Codec return class for images. Has imformation about the size and the
        pixel format of the image.
    */
    class ImageData : public Codec::CodecData
    {
    public:
        UInt32 ulHeight;
        UInt32 ulWidth;

        Image::PixelFormat eFormat;

    public:
        String dataType()
        {
            return "ImageData";
        }
    };

public:
    virtual void code( const DataChunk& input, DataChunk* output, ... ) const = 0;
    virtual CodecData * decode( const DataChunk& input, DataChunk* output, ... ) const = 0;

    virtual String getType() const = 0;
};

END_OGRE_NAMESPACE

#endif
