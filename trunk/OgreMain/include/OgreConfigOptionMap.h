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

#ifndef _ConfigOptionMap_H__
#define _ConfigOptionMap_H__

#include "OgrePrerequisites.h"

#include "OgreStringVector.h"
#include "OgreString.h"


namespace Ogre {
    /** Packages the details of a configuration option.
        @remarks
            Used for RenderSystem::getConfigOptions. If immutable is true, this
            option must be disabled for modifying.
    */
    typedef struct _ConfigOption
    {
        String name;
        String currentValue;
        StringVector possibleValues;
        bool immutable;
    } ConfigOption;

    typedef std::map<
        String,
        ConfigOption> ConfigOptionMap;

//    /** A wrapper template for the std::map class for config options.
//        <p>
//        This is required because the VC++ STL implementation fails badly when
//        STL classes are passed directly over a DLL boundary. This class
//        ensures that all implementation is kept within a single DLL and hence
//        functions correctly.
//        </p>
//    */
//    class _OgreExport ConfigOptionMap : public _ConfigOptionMapBase
//    {
//    public:
//        /** Class for comfortably iterating trough the map.
//            <p>
//            Adds an overhead of <code>2 * sizeof( iterator )</code>, but
//            also adds the <code>bool end()</code> method that specifies
//            wether we've reached the end of the container.
//            </p>
//            @remark This is also required for correct alocation/de-allocation
//                of memory across multiple DLLs.
//        */
//        class _OgreExport iterator
//        {
//        private:
//            _ConfigOptionMapBase::iterator mIter;
//            _ConfigOptionMapBase::iterator mEnd;
//
//            /** Hidden default constructor. Should never be called.
//            */
//            iterator() {}
//
//        public:
//            /** Usual constructor - should only called by
//                <code>ConfigOptionMap::begin()</code>.
//            */
//            iterator(
//                _ConfigOptionMapBase::iterator startAt,
//                _ConfigOptionMapBase::iterator end ) :
//                mIter( startAt ), mEnd( end )
//            {
//            }
//
//            /** Overloaded to mimic the STL iterator.
//            */
//            iterator& operator++()
//            {
//                mIter++;
//                return *this;
//            }
//
//            /** Returns the key.
//            */
//            const String& getKey()
//            {
//                return mIter->first;
//            }
//
//            /** Returns the value.
//            */
//            ConfigOption& getValue()
//            {
//                return mIter->second;
//            }
//
//            /** Checks wether we're at the end of the parent container.
//            */
//            bool end()
//            {
//                return mIter == mEnd;
//            }
//        };
//
//    public:
//        /** Default constructor.
//        */
//        ConfigOptionMap() {}
//
//        /** Default destructor.
//        */
//        ~ConfigOptionMap() {}
//
//        /** Returns an iterator to the beginning of the sequence.
//        */
//        void insert(const String& key, const ConfigOption& value)
//        {
//            _ConfigOptionMapBase::insert(
//                _ConfigOptionMapBase::value_type( key, value ) );
//        }
//
//        /** Finds an object based on the key.
//        */
//        iterator find(const String& key)
//        {
//            return iterator(
//                _ConfigOptionMapBase::find( key),
//                _ConfigOptionMapBase::end() );
//        }
//
//        /** Returns an iterator to the beginning of the sequence.
//        */
//        iterator begin(void)
//        {
//            return iterator(
//                _ConfigOptionMapBase::begin(),
//                _ConfigOptionMapBase::end() );
//        }
//    };

}

#endif
