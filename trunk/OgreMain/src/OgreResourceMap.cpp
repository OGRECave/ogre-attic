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
#include "OgreStableHeaders.h"
#include "OgreResourceMap.h"

#include "OgreException.h"

namespace Ogre {

#ifdef __OBSOLETE__DO__NOT__DEFINE__THIS__
    ResourceMap::ResourceMap()
    {
    }

    ResourceMap::~ResourceMap()
    {
    }

    void ResourceMap::insert(const String& key, Resource* value)
    {
        mImpl.insert(MapImpl::value_type(key, value));
    }

    void/*ResourceMap::iterator*/ ResourceMap::erase(iterator e)
    {
       /*MapImpl::iterator newe = ;*/
       /*std::map<String, Resource*, std::less<String> > mappie = mImpl;*/
        mImpl.erase( e.mIter );
       //return iterator(mappie.erase( e.mIter )/**/, mImpl.end());
    }

    void ResourceMap::erase(const String& key)
    {
        mImpl.erase(key);
    }

    ResourceMap::iterator ResourceMap::find(const String& key)
    {
        MapImpl::iterator i = mImpl.find(key);

        return iterator(i, mImpl.end());

    }


    ResourceMap::iterator ResourceMap::begin(void)
    {
        return iterator(mImpl.begin(), mImpl.end());
    }

    ResourceMap::iterator::iterator(MapImpl::iterator startAt, MapImpl::iterator end)
    {
        mIter = startAt;
        mEnd = end;
    }

    bool ResourceMap::iterator::end()
    {
        return (mIter == mEnd);
    }

    ResourceMap::iterator& ResourceMap::iterator::operator++()
    {
        mIter++;
        return *this;
    }

    const String& ResourceMap::iterator::getKey()
    {
        return mIter->first;
    }

    Resource* ResourceMap::iterator::getValue()
    {
        return mIter->second;
    }

    void ResourceMap::clear()
    {
        mImpl.clear();
    }
#endif


}
