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
#ifndef OGREIMAGERESAMPLER_H
#define OGREIMAGERESAMPLER_H

#include <algorithm>

// this file is inlined into OgreImage.cpp!
// do not include anywhere else.
using namespace Ogre;

// define uint64 type
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
typedef unsigned __int64 uint64;
#else
typedef unsigned long long uint64;
#endif

// variable name hints:
// sx_48 = 16/48-bit fixed-point x-position in source
// stepx = difference between adjacent sx_48 values
// sx1 = lower-bound integer x-position in source
// sx2 = upper-bound integer x-position in source
// sxf = fractional weight beween sx1 and sx2
// x,y,z = location of output pixel in destination

template<unsigned int srcelemsize> struct NearestResampler {

	static void scale(const PixelBox& src, const PixelBox& dst) {

		unsigned int dstelemsize = srcelemsize;
		uchar* pdst = (uchar*)dst.data + dstelemsize*(dst.left +
			dst.top*dst.rowPitch + dst.front*dst.slicePitch);

		/* x,y,z use incrementing 16/48-bit fixed-point counters */
		uint64 stepx = ((uint64)src.getWidth() << 48) / dst.getWidth();
		uint64 stepy = ((uint64)src.getHeight() << 48) / dst.getHeight();
		uint64 stepz = ((uint64)src.getDepth() << 48) / dst.getDepth();

		uint64 sz_48 = ((uint64)src.front << 48) + (stepz >> 1) - 1;
		for (unsigned int z = dst.front; z < dst.back; z++, sz_48 += stepz) {
			unsigned int srczoff = (unsigned int)(sz_48 >> 48) * src.slicePitch;
			
			uint64 sy_48 = ((uint64)src.top << 48) + (stepy >> 1) - 1;
			for (unsigned int y = dst.top; y < dst.bottom; y++, sy_48 += stepy) {
				unsigned int srcyoff = (unsigned int)(sy_48 >> 48) * src.rowPitch;
			
				uint64 sx_48 = ((uint64)src.left << 48) + (stepx >> 1) - 1;
				for (unsigned int x = dst.left; x < dst.right; x++, sx_48 += stepx) {
					uchar* psrc = (uchar*)src.data +
						srcelemsize*((sx_48 >> 48) + srcyoff + srczoff);
                    memcpy(pdst, psrc, srcelemsize);
					pdst += dstelemsize;
				}
				pdst += dstelemsize*dst.getRowSkip();
			}
			pdst += dstelemsize*dst.getSliceSkip();
		}
	}

};


// default linear resampler: slower, but works for all src and dst formats.
// Does not need to be a template; this wastes memory, and as packColour/unpackColour are used
// it saves no cycle anyway.
//template<unsigned int srcelemsize> struct LinearResampler {
struct LinearResampler {
	static void scale(const PixelBox& src, const PixelBox& dst) {
		unsigned int srcelemsize = PixelUtil::getNumElemBytes(src.format);
		unsigned int dstelemsize = PixelUtil::getNumElemBytes(dst.format);
		uchar* pdst = (uchar*)dst.data + dstelemsize*(dst.left
			+ dst.top*dst.rowPitch + dst.front*dst.slicePitch);
		
		uint64 stepx = ((uint64)src.getWidth() << 48) / dst.getWidth();
		uint64 stepy = ((uint64)src.getHeight() << 48) / dst.getHeight();
		uint64 stepz = ((uint64)src.getDepth() << 48) / dst.getDepth();
		
		const float float_12bit = 1.0f / 0x1000;
		unsigned int temp;
		uchar *srcdata = (uchar*)src.data;
		
		uint64 sz_48 = ((uint64)src.front << 48) + (stepz >> 1) - 1;
		for (unsigned int z = dst.front; z < dst.back; z++, sz_48 += stepz) {
			temp = sz_48 >> 36;
			temp = (temp > 0x800)? temp - 0x800 : 0;
			unsigned int sz1 = temp >> 12; // slice 1
			unsigned int sz2 = (unsigned int)std::min(static_cast<unsigned int>(src.back-src.front-1), sz1+1); // slice 2
			float szf = (temp & 0xFFF) * float_12bit;

			uint64 sy_48 = ((uint64)src.top << 48) + (stepy >> 1) - 1;
			for (unsigned int y = dst.top; y < dst.bottom; y++, sy_48 += stepy) {
				temp = sy_48 >> 36;
				temp = (temp > 0x800)? temp - 0x800 : 0;
				unsigned int sy1 = temp >> 12; // row 1
				unsigned int sy2 = (unsigned int)std::min(static_cast<unsigned int>(src.bottom-src.top-1), sy1+1); // row 2
				float syf = (temp & 0xFFF) * float_12bit;
				
				uint64 sx_48 = ((uint64)src.left << 48) + (stepx >> 1) - 1;
				for (unsigned int x = dst.left; x < dst.right; x++, sx_48 += stepx) {
					temp = sx_48 >> 36;
					temp = (temp > 0x800)? temp - 0x800 : 0;
					unsigned int sx1 = temp >> 12; // column 1
					unsigned int sx2 = (unsigned int)std::min(static_cast<unsigned int>(src.right-src.left-1), sx1+1); // column 2
					float sxf = (temp & 0xFFF) * float_12bit;
				
					// 8 bits * 12 bits * 12 bits -> all 32 bits used in accum
					ColourValue x1y1z1, x2y1z1, x1y2z1, x2y2z1;
					ColourValue x1y1z2, x2y1z2, x1y2z2, x2y2z2;

					PixelUtil::unpackColour(&x1y1z1, src.format, srcdata +
						srcelemsize*(sx1 + sy1*src.rowPitch + sz1*src.slicePitch));
					PixelUtil::unpackColour(&x2y1z1, src.format, srcdata +
						srcelemsize*(sx2 + sy1*src.rowPitch + sz1*src.slicePitch));
					PixelUtil::unpackColour(&x1y2z1, src.format, srcdata +
						srcelemsize*(sx1 + sy2*src.rowPitch + sz1*src.slicePitch));
					PixelUtil::unpackColour(&x2y2z1, src.format, srcdata +
						srcelemsize*(sx2 + sy2*src.rowPitch + sz1*src.slicePitch));
					PixelUtil::unpackColour(&x1y1z2, src.format, srcdata +
						srcelemsize*(sx1 + sy1*src.rowPitch + sz2*src.slicePitch));
					PixelUtil::unpackColour(&x2y1z2, src.format, srcdata +
						srcelemsize*(sx2 + sy1*src.rowPitch + sz2*src.slicePitch));
					PixelUtil::unpackColour(&x1y2z2, src.format, srcdata +
						srcelemsize*(sx1 + sy2*src.rowPitch + sz2*src.slicePitch));
					PixelUtil::unpackColour(&x2y2z2, src.format, srcdata +
						srcelemsize*(sx2 + sy2*src.rowPitch + sz2*src.slicePitch));

					ColourValue accum =
						x1y1z1 * ((1.0f - sxf) * (1.0f - syf) * (1.0f - szf)) +
						x2y1z1 * (        sxf  * (1.0f - syf) * (1.0f - szf)) +
						x1y2z1 * ((1.0f - sxf) *         syf  * (1.0f - szf)) +
						x2y2z1 * (        sxf  *         syf  * (1.0f - szf)) +
						x1y1z2 * ((1.0f - sxf) * (1.0f - syf) *         szf ) +
						x2y1z2 * (        sxf  * (1.0f - syf) *         szf ) +
						x1y2z2 * ((1.0f - sxf) *         syf  *         szf ) +
						x2y2z2 * (        sxf  *         syf  *         szf );

					PixelUtil::packColour(accum, dst.format, pdst);

					pdst += dstelemsize;
				}
				pdst += dstelemsize*dst.getRowSkip();
			}
			pdst += dstelemsize*dst.getSliceSkip();
		}
	}
};

// optimized for 2D sampling on byte-oriented pixel formats
template<unsigned int srcelemsize> struct LinearResampler_Byte {
	static void scale(const PixelBox& src, const PixelBox& dst) {

		// only optimized for 2D
		if (src.getDepth() > 1 || dst.getDepth() > 1) {
			LinearResampler::scale(src, dst);
			return;
		}

		unsigned int dstelemsize = srcelemsize;
		uchar* pdst = (uchar*)dst.data + dstelemsize*(dst.left + dst.top*dst.rowPitch);

		uint64 stepx = ((uint64)src.getWidth() << 48) / dst.getWidth();
		uint64 stepy = ((uint64)src.getHeight() << 48) / dst.getHeight();
		unsigned int temp;
		
		uint64 sy_48 = ((uint64)src.top << 48) + (stepy >> 1) - 1;
		for (unsigned int y = dst.top; y < dst.bottom; y++, sy_48 += stepy) {
			temp = sy_48 >> 36;
			temp = (temp > 0x800)? temp - 0x800: 0;
			unsigned int syf = temp & 0xFFF;
			unsigned int sy1 = temp >> 12;
			unsigned int sy2 = (unsigned int)std::min(static_cast<unsigned int>(src.bottom-src.top-1), sy1+1);
			unsigned int syoff1 = sy1 * src.rowPitch;
			unsigned int syoff2 = sy2 * src.rowPitch;

			uint64 sx_48 = ((uint64)src.left << 48) + (stepx >> 1) - 1;
			for (unsigned int x = dst.left; x < dst.right; x++, sx_48 += stepx) {
				temp = sx_48 >> 36;
				temp = (temp > 0x800)? temp - 0x800 : 0;
				unsigned int sxf = temp & 0xFFF;
				unsigned int sx1 = temp >> 12;
				unsigned int sx2 = (unsigned int)std::min(static_cast<unsigned int>(src.right-src.left-1), sx1+1);
				unsigned int sxfsyf = sxf*syf;
				for (unsigned int k = 0; k < srcelemsize; k++) {
					unsigned int accum =
						((uchar*)src.data)[(sx1 + syoff1)*srcelemsize+k]*(0x1000000-(sxf<<12)-(syf<<12)+sxfsyf) +
						((uchar*)src.data)[(sx2 + syoff1)*srcelemsize+k]*((sxf<<12)-sxfsyf) +
						((uchar*)src.data)[(sx1 + syoff2)*srcelemsize+k]*((syf<<12)-sxfsyf) +
						((uchar*)src.data)[(sx2 + syoff2)*srcelemsize+k]*sxfsyf;
						// 8 bits * 12 bits * 12 bits -> result is 8/24 fixed-point
						// (maximum is 0xFF000000; rounding will not cause overflow)
					pdst[k] = (accum + 0x800000) >> 24;
				}

				pdst += dstelemsize;
			}
			pdst += dstelemsize*dst.getRowSkip();
		}
	}
};

#endif
