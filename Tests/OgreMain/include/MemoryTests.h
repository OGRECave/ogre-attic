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


#ifndef MEMORYTESTS_H
#define MEMORYTESTS_H

// CppUnit headers
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

// headers for the objects we are testing
#include <OgreAllocator.h>
#include <OgreAllocWrapper.h>
#include <OgreMemProfilerBase.h>
#include <OgreObjectTraits.h>

// allocation policys to test, more to come.
#include <OgreStdAllocPolicy.h>

#include <memory>
#include <cstddef>

class MemoryTests : public CPPUNIT_NS :: TestFixture
{
public:
    inline explicit MemoryTests()
    {}

    virtual ~MemoryTests()
    {}

    CPPUNIT_TEST_SUITE ( MemoryTests );
    // in later tests we use the profile path to collect data,
    // so test it works first.
    CPPUNIT_TEST ( testProfilePath );

    // test the rebind mechanism, turn an allocator<T> into and allocator<U>
    CPPUNIT_TEST ( testRebind );

    // test combatiblity with std::list
    CPPUNIT_TEST ( testStdList );

    // test combatiblity with std::vector
    CPPUNIT_TEST ( testStdVector );

    //TODO: add more container type tests

    CPPUNIT_TEST ( testDataIntegrity );

    // test the use of AllocWrapper as a base class
    CPPUNIT_TEST ( testAllocWrapperBase );
    CPPUNIT_TEST_SUITE_END ();

public:
    void setUp ();
    void tearDown ();

protected:
    void testProfilePath();
    void testRebind();
    void testStdList();
    void testStdVector();
    void testDataIntegrity();
    void testAllocWrapperBase();

private:
    static int mNumAllocations;
    static int mNumBytesAllocated;
    static int mNumDeallocations;
    static int mNumBytesDeallocated;

    // simple testing data struct
    struct Data
    {
        int one;
        int two;
    };

    // internal class to simulate the profiler and allow for the
    // collection of test data
    template<typename T>
    class PsudoProfiler
    {
    public:
        /// define our types
        typedef T                   value_type;
        typedef value_type*         pointer;
        typedef const value_type*  const_pointer;
        typedef value_type&         reference;
        typedef const value_type&  const_reference;
        typedef std::size_t        size_type;
        typedef std::ptrdiff_t      difference_type;

        /// convert PsudoProfiler<T> to PsudoProfiler<U>
        template<typename U>
        struct rebind
        {
            typedef class PsudoProfiler<U> other;
        };

    public:
        explicit inline PsudoProfiler()
        { }

        inline ~PsudoProfiler()
        { }

        /// copy ctor
        inline explicit PsudoProfiler( PsudoProfiler const& )
        { }

        /// converstion
        template <typename U>
        inline explicit PsudoProfiler( PsudoProfiler<U> const& )
        { }

        /// Note information about an allocation
        inline void note_allocation( size_type sz,
                                     typename std::allocator<void>::const_pointer ptr = 0 )
        {
            MemoryTests::mNumAllocations++;
            MemoryTests::mNumBytesAllocated += sz;
        }

        /// Note information about a deallocation
        inline void note_deallocation( pointer ptr, size_type sz )
        {
            MemoryTests::mNumDeallocations++;
            MemoryTests::mNumBytesDeallocated += sz;
        }
    };

    /// class used in AllocWrapper tests
class TestClass : public Ogre::AllocWrapper
                                <
                                    Ogre::Allocator
                                    <
                                        TestClass,
                                        Ogre::StdAllocPolicy<TestClass>,
                                        Ogre::ObjectTraits<TestClass>,
                                        PsudoProfiler<TestClass>
                                    >
                                >
    {
        int padding[10];
    };

};



#endif // MEMORYTESTS_H
