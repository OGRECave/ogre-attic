/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef OGREOGREMEMORYREGION_H
#define OGREOGREMEMORYREGION_H

#include "OgreMemoryBin.h"
#include "OgrePlatform.h"
#include "OgreStdHeaders.h"


namespace Ogre
{

  /**
   @author
  */

  class MemoryRegion
  {
    public:
        static const unsigned long POOL_SIZE = 32768; // (32k = 2^15)
        static const unsigned long NUM_BINS = 13; // (sufficent for 32k, POWER-3)

    private:
        char mPool[POOL_SIZE];    // this is our memory pool 
        MemoryBin mBin[NUM_BINS]; // bins for managing blocks
        MemoryBin m24ByteBin;
        uint32 mIndex;

        // helper method, distribute excess memory
        void distributeCore(MemBlock block);

    public:
      explicit inline MemoryRegion(){}
      inline ~MemoryRegion(){}

      void setup(uint32 index);

      /**
       * allocate memory from the free store. This will expand the 
       * process virtual address space and "touch" the resulting 
       * memory, the OS should map some form of phisical storage as
       * a result. Normal paging rules still apply.
       */
      void* allocMem ( size_t size ) throw ( std::bad_alloc );

      /**
       * return memory to be re-used, note, this may not release the
       * memory back to the system imediatly. The manager caches 
       * some memory to be re-used later, its kinder to the OS memory
       * mapping stuff.
       */
      void purgeMem ( MemCtrl* ptr ) throw ( std::bad_alloc );

      /**
       * test to see if this region can satisfy a reguested alloc size
       * @param size the desired allocation size
       * @return true if it can 
       */
      bool canSatisfy(uint32 size);

      void dumpInternals();

  };

}

#endif
