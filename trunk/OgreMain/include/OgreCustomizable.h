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
#ifndef __Customizable_H__
#define __Customizable_H__

#include "OgrePrerequisites.h"

namespace Ogre {
    /** Class that provides functions to get/set custom parameters.
        @warning
            This class does some really type-insafe operations, so care 
            should be taken.
    */
    class Customizable
    {
    protected:
        struct DataHolder
        {
            void  *data;
            size_t size;

            DataHolder()
                : data( NULL ),
                  size( 0 )
            {
            }

            DataHolder( void *d, size_t s )
                : data( d ),
                  size( s )
            {
            }

            ~DataHolder()
            {
            }
        };

    protected:
        std::map< std::string, DataHolder* > m_hsProperties;

    protected:
        void setParamPtr( 
            const std::string& param, 
            void *data,
            size_t size )
        {
            DataHolder *pDH = new DataHolder( data, size );
            m_hsProperties[ param ] = pDH;
        }

    public:
        void setParam( const std::string& param, void *data, size_t size )
        {        
            if( !m_hsProperties[param] )
            {
                return;
            }
            
            memcpy( 
                m_hsProperties[param]->data,
                &data,
                m_hsProperties[param]->size ) ;
        }

        template< typename T >
        const T& getParam( const std::string& param ) const
        {
            return *(T*)(m_hsProperties[param])->data;
        }

        template< typename T >
        const T* const getParamPtr( const std::string& param )
        {
            return (T*)(m_hsProperties[param])->data;
        }
    };
}

#endif
