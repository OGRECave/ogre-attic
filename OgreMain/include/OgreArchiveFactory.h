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
#ifndef _ArchiveFactory_H__
#define _ArchiveFactory_H__

#include "OgrePrerequisites.h"

#include "OgreFactoryObj.h"

namespace Ogre {

    /** Abstract factory class, archive codec plugins can register concrete
        subclasses of this.
        @remarks
            All access to 'archives' (collections of files, compressed or
            just folders, maybe even remote) is managed via the abstract
            ArchiveEx class. Plugins are expected to provide the
            implementation for the actual codec itself, but because a
            subclass of ArchiveEx has to be created for every archive, a
            factory class is required to create the appropriate subclass.
        @par
            So archive plugins create a subclass of ArchiveEx AND a subclass
            of ArchiveFactory which creates instances of the ArchiveEx
            subclass. See the 'Zip' and 'FileSystem' plugins for examples.
            Each ArchiveEx and ArchiveFactory subclass pair deal with a
            single archive type (identified by a string).
    */
    class _OgreExport ArchiveFactory : public FactoryObj< ArchiveEx >
    {
    public:
        virtual ArchiveEx *createObj(const String& name ) = 0;
        virtual String getType() = 0;
    };

} // namespace

#endif
