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
#ifndef _FactoryObj_H__
#define _FactoryObj_H__

#include "OgrePrerequisites.h"

namespace Ogre {

    /** Abstract factory class. Does nothing by itself, but derived classes can add
        functionality.
    */
    template< typename T > class FactoryObj
    {
    public:
        virtual ~FactoryObj() {};

        /** Returns the factory type.
            @return
                The factory type.
        */
        virtual String getType() = 0;

        /** 'Produces' a new object.
            @param
                nA First required parameter. It exists because the implementations
                of createObj may need to get a pointer to the optional parameters
                list.
            @return
                An object created by the factory. The type of the object depends on
                the factory.
        */
        virtual T* createObj( int nA, ... ) = 0;    
    };

} // namespace

#endif
