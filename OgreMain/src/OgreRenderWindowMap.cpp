/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
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
#include "OgreRenderWindowMap.h"

#include "OgreException.h"

namespace Ogre {

#ifdef __OBSOLETE__DO__NOT__DEFINE__THIS__
    RenderWindowMap::RenderWindowMap()
    {
    }

    RenderWindowMap::~RenderWindowMap()
    {
    }

    void RenderWindowMap::insert(const String& key, RenderWindow* value)
    {
        mImpl.insert(MapImpl::value_type(key, value));
    }

    void/*RenderWindowMap::iterator*/ RenderWindowMap::erase(iterator e)
    {
        /*MapImpl::iterator newe = */mImpl.erase( e.mIter );
        //return RenderWindowMap::iterator( newe, mImpl.end());
    }

    size_t RenderWindowMap::erase(const String& key)
    {
        return mImpl.erase(key);
    }

    RenderWindowMap::iterator RenderWindowMap::find(const String& key)
    {
        MapImpl::iterator i = mImpl.find(key);

        return iterator(i, mImpl.end());

    }

    RenderWindowMap::iterator RenderWindowMap::begin(void)
    {
        return iterator(mImpl.begin(), mImpl.end());
    }

    RenderWindowMap::iterator::iterator(MapImpl::iterator startAt, MapImpl::iterator end)
    {
        mIter = startAt;
        mEnd = end;
    }

    bool RenderWindowMap::iterator::end()
    {
        return (mIter == mEnd);
    }

    RenderWindowMap::iterator& RenderWindowMap::iterator::operator++()
    {
        mIter++;
        return *this;
    }

    const String& RenderWindowMap::iterator::getKey()
    {
        return mIter->first;
    }

    RenderWindow* RenderWindowMap::iterator::getValue()
    {
        return mIter->second;
    }
#endif


}
