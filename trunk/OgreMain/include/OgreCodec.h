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
#ifndef _Codec_H__
#define _Codec_H__

#include "OgreDataChunk.h"

namespace Ogre {

    /** Abstract class that defines a 'codec'.
        @remarks
            A codec class works like a two-way filter for data - data entered on
            one end (the decode end) gets processed and transformed into easily
            usable data while data passed the other way around codes it back.
        @par
            The codec concept is a pretty generic one - you can easily understand
            how it can be used for images, sounds, archives, even compressed data.
    */
    class _OgreExport Codec
    {
    protected:
        /** A map that contains all the registered codecs.
        */
        static std::map< String, Codec* > ms_mapCodecs;

    public:
        class CodecData 
        {
        public:
            virtual ~CodecData() {};

            /** Returns the type of the data.
            */
            virtual String dataType() { return "CodecData"; };
        };

    public:
        /** Registers a new codec in the database.
        */
        static void registerCodec( Codec *pCodec )
        {
            ms_mapCodecs[pCodec->getType()] = pCodec;
        }

        /** Returns a map of all the registered codecs.
        */
        static std::map< String, Codec* >& getColl()
        {
            return ms_mapCodecs;
        }

        /** Codes the data in the input chunk and saves the result in the output
            chunk.
            @note
                Has a variable number of arguments, which depend on the codec type.
        */
        virtual void code( const DataChunk& input, DataChunk* output, ... ) const = 0;

        /** Codes the data from the input chunk into the output chunk.
            @remarks
                The returned CodecData pointer is a pointer to a class that holds
                information about the decoded buffer. For an image, this would be 
                the size, the bitdepht, etc.
            @note
                Has a variable number of arguments, which depend on the codec type.
        */
        virtual CodecData * decode( const DataChunk& input, DataChunk* output, ... ) const = 0;

        /** Returns the type of the codec as a String
        */
        virtual String getType() const = 0;
    };

} // namespace

#endif
