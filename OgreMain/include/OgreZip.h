/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __Zip_H__
#define __Zip_H__

#include "OgrePrerequisites.h"

#include "OgreArchiveEx.h"
#include "unzip.h"

namespace Ogre {

    class Zip : public ArchiveEx {
    public:
        Zip();
        Zip(const String& name );
        ~Zip();

        void load();
        void unload();

        bool fileOpen( const String& strFile, FILE** ppFile ) const;
        bool fileRead( const String& strFile, DataChunk** ppChunk ) const;

        bool fileSave( FILE* pFile, const String& strPath, bool bOverwrite = false );
        bool fileWrite( const DataChunk& refChunk, const String& strPath, bool bOverwrite = false );

        bool fileDele( const String& strFile );
        bool fileMove( const String& strSrc, const String& strDest, bool bOverwrite );

        bool fileInfo( const String& strFile, FileInfo** ppInfo ) const;
        bool fileCopy( const String& strSrc, const String& strDest, bool bOverwrite );

        bool fileTest( const String& strFile ) const;

        std::vector<String> dirGetFiles( const String& strDir ) const;
        std::vector<String> dirGetSubs( const String& strDir ) const;

        bool dirDele( const String& strDir, bool bRecursive );
        bool dirMove( const String& strSrc, const String& strDest, bool bOverwrite );

        bool dirInfo( const String& strDir, FileInfo** ppInfo ) const;
        bool dirCopy( const String& strSrc, const String& strDest, bool bOverwrite );

        bool dirTest( const String& strDir ) const;

        std::vector<String> getAllNamesLike( const String& strStartPath, const String& strPattern, bool bRecursive=true );

        bool _allowFileCaching() const { return true; }

    private:
        unzFile mArchive;

    };

}

#endif
