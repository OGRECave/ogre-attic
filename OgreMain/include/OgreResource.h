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
#ifndef _Resource_H__
#define _Resource_H__

#include "OgrePrerequisites.h"

#include "OgreString.h"

BEGIN_OGRE_NAMESPACE

/** Abstract class reprensenting a loadable resource (e.g. textures, sounds etc)
    @remarks
        Resources are generally passive constructs, handled through the
        ResourceManager abstract class for the appropriate subclass.
        The main thing is that Resources can be loaded or unloaded by the
        ResourceManager to stay within a defined memory budget. Therefore,
        all Resources must be able to load, unload (whilst retainin enough
        info about themselves to be reloaded later), and state how big
        they are.
    @par
        Subclasses must implement:
            1. A constructor, with at least a mandatory name param.
            This constructor must set mName and optionally mSize.
            2. The load() and unload() methods - mSize must be set after load()
            Each must check & update the mIsLoaded flag.
*/
class _OgreExport Resource {
protected:
    String mName;
    bool   mIsLoaded;
    time_t mLastAccess;
    size_t mSize;

public:
    /** Basic constructor. 
        @warn
            Subclasses must init mName and mSize!
    */
    Resource() 
        : mIsLoaded( false ), mSize( 0 )
    { 
    }

    /** Virtual destructor. Shouldn't need to be overloaded, as the resource
        deallocation code should reside in unload()
        @see
            Resource::unload()
    */
    virtual ~Resource() 
    { 
        if (mIsLoaded) 
            unload(); 
    }

    /** Loads the resource, if it is not already.
    */
    virtual void load() = 0;

    /** Unloads the resource, but retains data to recreate.
    */
    virtual void unload() {};

    /** Retrieves info about the size of the resource.
    */
    virtual size_t getSize(void) 
    { 
        return mSize; 
    }

    /** 'Touches' the resource to indicate it has been used.
    */
    void touch(void) 
    { 
        mLastAccess = time(NULL); 
    }

    /** Gets the last time the resource was 'touched'.
    */
    time_t getLastAccess(void) const 
    { 
        return mLastAccess; 
    }

    /** Gets resource name.
    */
    const String& getName(void) const 
    { 
        return mName; 
    }

    /** Returns true if the Resource has been loaded, false otherwise.
    */
    bool isLoaded(void) const 
    { 
        return mIsLoaded; 
    }
};

END_OGRE_NAMESPACE

#endif
