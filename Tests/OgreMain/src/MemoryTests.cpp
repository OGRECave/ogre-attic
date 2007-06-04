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


#include "MemoryTests.h"

// containers to test - more to come
#include <list>
#include <vector>

CPPUNIT_TEST_SUITE_REGISTRATION ( MemoryTests );

// statics
int MemoryTests::mNumAllocations = 0;
int MemoryTests::mNumBytesAllocated = 0;
int MemoryTests::mNumDeallocations = 0;
int MemoryTests::mNumBytesDeallocated = 0;

void MemoryTests::setUp()
{
  mNumAllocations=0;
  mNumBytesAllocated=0;
  mNumDeallocations=0;
  mNumBytesDeallocated=0;
}

void MemoryTests::tearDown()
{}

void MemoryTests::testProfilePath()
{
  // setup allocator with all default policies and a psudo-profiller
  Ogre::Allocator
  <
    int,
    Ogre::StdAllocPolicy<int>,
    Ogre::ObjectTraits<int>,
    PsudoProfiler<int>
  > aloc;

  int* tmp;
  for(int i=0;i<10;++i)
  {
    tmp = aloc.allocate(1);
    aloc.deallocate(tmp,1);
  }

  // check that the number and size of [de]allocation match the expected values
  CPPUNIT_ASSERT(mNumAllocations==10);
  CPPUNIT_ASSERT(mNumBytesAllocated==(sizeof(int)*10));
  CPPUNIT_ASSERT(mNumDeallocations==10);
  CPPUNIT_ASSERT(mNumBytesDeallocated==(sizeof(int)*10));
}

void MemoryTests::testRebind()
{
  // setup allocator with all default policies and a psudo-profiller
  typedef Ogre::Allocator
  <
    int,
    Ogre::StdAllocPolicy<int>,
    Ogre::ObjectTraits<int>,
    PsudoProfiler<int>
  > AlocTypeT;

  typedef Ogre::Allocator
  <
    float,
    Ogre::StdAllocPolicy<float>,
    Ogre::ObjectTraits<float>,
    PsudoProfiler<float>
  > AlocTypeU;

  AlocTypeT aloc1;
  AlocTypeU aloc2;
  AlocTypeT::rebind<float>::other aloc3;

  //NOTE: if this compiles then the test is passed
  aloc2 = aloc3;
}

void MemoryTests::testStdList()
{
  // container type
  typedef std::list
  <
    int,
    Ogre::Allocator
    <
      int,
      Ogre::StdAllocPolicy<int>,
      Ogre::ObjectTraits<int>,
      PsudoProfiler<int>
    >
  > TestCont;

  {
    TestCont t;
    for(int i=0;i<10;++i)
      t.push_back(i);
  }

  // check that the number of [de]allocation match the expected values
  CPPUNIT_ASSERT(mNumAllocations==10);
  CPPUNIT_ASSERT(mNumDeallocations==10);

  // NOTE: std::list uses rebind<node<int> > so the allocation size can not
  // be tested directly, we dont know the stl "node" implementation details.
  CPPUNIT_ASSERT(mNumBytesAllocated==mNumBytesDeallocated);
}

void MemoryTests::testStdVector()
{
  // setup allocator with all default policies and a psudo-profiller
  typedef Ogre::Allocator<
    int,
    Ogre::StdAllocPolicy<int>,
    Ogre::ObjectTraits<int>,
    PsudoProfiler<int>
  > aloc_type;

  // container type
  typedef std::vector<int,aloc_type > TestCont;

  {
    TestCont t;
    t.reserve(10); // vector grabs the memory in one block
  } // vector goes out of scope and releases memory here

  // check that the number and size of [de]allocation match the expected values
  CPPUNIT_ASSERT(mNumAllocations==1);
  CPPUNIT_ASSERT(mNumBytesAllocated==(sizeof(int)*10));
  CPPUNIT_ASSERT(mNumDeallocations==1);
  CPPUNIT_ASSERT(mNumBytesDeallocated==(sizeof(int)*10));
}

void MemoryTests::testDataIntegrity()
{
  // setup allocator with all default policies and a psudo-profiller
  typedef Ogre::Allocator<
    Data,
    Ogre::StdAllocPolicy<Data>,
    Ogre::ObjectTraits<Data>,
    PsudoProfiler<Data>
  > aloc_type;

  // container type
  typedef std::vector<Data,aloc_type > TestCont;

  {
    TestCont t;
    // populate the container with data
    for(int i=0;i<10;++i)
    {
      Data d;
      d.one = 0;
      d.two = i;
      t.push_back(d);
    }

    // confirme that the data remains valid after several [de]allocations
    for(int j=0;j<10;++j)
    {
      CPPUNIT_ASSERT(t[j].one==0);
      CPPUNIT_ASSERT(t[j].two==j);
    }
  }
}

void MemoryTests::testAllocWrapperBase()
{
  TestClass* t;
  t = new TestClass();
  delete t;

  CPPUNIT_ASSERT(mNumAllocations==1);
  CPPUNIT_ASSERT(mNumBytesAllocated==sizeof(TestClass));
  CPPUNIT_ASSERT(mNumDeallocations==1);
  CPPUNIT_ASSERT(mNumBytesDeallocated==sizeof(TestClass));
}



