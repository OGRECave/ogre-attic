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
#ifndef _Bitwise_H__
#define _Bitwise_H__

#include "OgrePrerequisites.h"

namespace Ogre {

    /** Class for manipulating bit patterns.
    */
    class Bitwise {
    public:
        /** Returns the number of bits a pattern must be shifted right by to
            remove right-hand zeroes.
        */
		template<typename T>
        static FORCEINLINE unsigned int getBitShift(T mask)
		{
			if (mask == 0)
				return 0;

			unsigned int result = 0;
			while ((mask & 1) == 0) {
				++result;
				mask >>= 1;
			}
			return result;
		}

        /** Takes a value with a given src bit mask, and produces another
            value with a desired bit mask.
            @remarks
                This routine is useful for colour conversion.
        */
		template<typename SrcT, typename DestT>
        static inline DestT convertBitPattern(SrcT srcValue, SrcT srcBitMask, DestT destBitMask)
		{
			// Mask off irrelevant source value bits (if any)
			srcValue = srcValue & srcBitMask;

			// Shift source down to bottom of DWORD
			const unsigned int srcBitShift = getBitShift(srcBitMask);
			srcValue >>= srcBitShift;

			// Get max value possible in source from srcMask
			const SrcT srcMax = srcBitMask >> srcBitShift;

			// Get max avaiable in dest
			const unsigned int destBitShift = getBitShift(destBitMask);
			const DestT destMax = destBitMask >> destBitShift;

			// Scale source value into destination, and shift back
			DestT destValue = (srcValue * destMax) / srcMax;
			return (destValue << destBitShift);
		}

        /**
         * Convert N bit colour channel value to P bits. It fills P bits with the
         * bit pattern repeated. (this is /((1<<n)-1) in fixed point)
         */
        static inline unsigned int fixedToFixed(uint32 value, unsigned int n, unsigned int p) 
        {
            if(n > p) 
            {
                // Less bits required than available; this is easy
                value >>= n-p;
            } 
            else if(n < p)
            {
                // More bits required than are there, do the fill
                // Use old fashioned division, probably better than a loop
                if(value == 0)
                        value = 0;
                else if(value == (1<<n)-1)
                        value = (1<<p)-1;
                else    value = value*(1<<p)/((1<<n)-1);
            }
            return value;    
        }

        /**
         * Convert floating point colour channel value between 0.0 and 1.0 (otherwise clamped) 
         * to integer of a certain number of bits. Works for any value of bits between 0 and 31.
         */
        static inline unsigned int floatToFixed(float value, unsigned int bits) 
        {
            if(value <= 0.0f) return 0;
            else if (value >= 1.0f) return (1<<bits)-1;
            else return (unsigned int)(value * (1<<bits));     
        }

        /**
         * Fixed point to float
         */
        static inline float fixedToFloat(unsigned value, unsigned int bits)
        {
            return (float)value/(float)((1<<bits)-1);
        }

        /**
         * Write a n*8 bits integer value to memory in native endian.
         */
        static inline void intWrite(void *dest, int n, unsigned int value) 
        {
            switch(n) {
                case 1:
                    ((uint8*)dest)[0] = (uint8)value;
                    break;
                case 2:
                    ((uint16*)dest)[0] = (uint16)value;
                    break;
                case 3:
#if OGRE_ENDIAN == ENDIAN_BIG      
                    ((uint8*)dest)[0] = (uint8)(value >> 16);
                    ((uint8*)dest)[1] = (uint8)(value >> 8);
                    ((uint8*)dest)[2] = (uint8)(value);
#else
                    ((uint8*)dest)[2] = (uint8)(value >> 16);
                    ((uint8*)dest)[1] = (uint8)(value >> 8);
                    ((uint8*)dest)[0] = (uint8)(value);
#endif
                    break;
                case 4:
                    ((uint32*)dest)[0] = (uint32)value;                
                    break;                
            }        
        }
    
        /**
         * Read a n*8 bits integer value to memory in native endian.
         */
        static inline unsigned int intRead(const void *src, int n) {
            switch(n) {
                case 1:
                    return ((uint8*)src)[0];
                case 2:
                    return ((uint16*)src)[0];
                case 3:
#if OGRE_ENDIAN == ENDIAN_BIG      
                    return ((uint32)((uint8*)src)[0]<<16)|
                            ((uint32)((uint8*)src)[1]<<8)|
                            ((uint32)((uint8*)src)[2]);
#else
                    return ((uint32)((uint8*)src)[0])|
                            ((uint32)((uint8*)src)[1]<<8)|
                            ((uint32)((uint8*)src)[2]<<16);
#endif
                case 4:
                    return ((uint32*)src)[0];
            } 
            return 0; // ?
        }


    };
}

#endif
