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
#include "OgreZip.h"

#include "OgreArchiveManager.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreZipArchiveFactory.h"
#include "OgreStringVector.h"
#include "OgreRoot.h"

#if OGRE_PLATFORM == PLATFORM_LINUX
static void strlwr(char* x)
{
    while (*x)
    {
        *x = tolower(*x);
        x++;
    }
}
#endif

namespace Ogre {


    //-----------------------------------------------------------------------
    ZipArchiveFactory* pZipArchFactory = NULL;
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    extern "C" void dllStartPlugin(void)
    {
        SET_TERM_HANDLER;

        // Register archive
        pZipArchFactory = new ZipArchiveFactory();
        ArchiveManager::getSingleton().addArchiveFactory(pZipArchFactory);
    }

    //-----------------------------------------------------------------------
    extern "C" void dllStopPlugin(void)
    {
        delete pZipArchFactory;
    }

    //-----------------------------------------------------------------------
    bool Zip::fileOpen( const String& strFile, FILE** ppFile ) const
    {
        unz_file_info tagUFI;
        FILE *pFile;

        if( unzLocateFile( mArchive, strFile.c_str(), 2 ) == UNZ_OK ) {
            //*ppFile = tmpfile();
            pFile = *ppFile;

            unzGetCurrentFileInfo( mArchive, &tagUFI, NULL, 0, NULL, 0, NULL, 0 );
            unsigned char* pBuffer = new unsigned char[tagUFI.uncompressed_size];
            unzOpenCurrentFile( mArchive );
            unzReadCurrentFile( mArchive, (void*)pBuffer, tagUFI.uncompressed_size );
            unzCloseCurrentFile( mArchive );
            fwrite( (void*) pBuffer, 1, tagUFI.uncompressed_size, pFile );
            delete[] pBuffer;

            fseek( pFile, 0, SEEK_SET );
            return true;
        }

        return false;
    }

    //-----------------------------------------------------------------------
    bool Zip::fileRead( const String& strFile, DataChunk** ppChunk ) const
    {
        DataChunk* pChunk = *ppChunk;
        unz_file_info tagUFI;

        if( unzLocateFile( mArchive, strFile.c_str(), 2 ) == UNZ_OK ) {
            unzGetCurrentFileInfo( mArchive, &tagUFI, NULL, 0, NULL, 0, NULL, 0 );
            pChunk->allocate(tagUFI.uncompressed_size);
            unzOpenCurrentFile( mArchive );
            unzReadCurrentFile( mArchive, (void*)pChunk->getPtr(), tagUFI.uncompressed_size );
            unzCloseCurrentFile( mArchive );

            return true;
        }

        return false;
    }

    //-----------------------------------------------------------------------
    bool Zip::fileSave( ::FILE* pFile, const String& strPath, bool bOverwrite /* = false */ )
    {
        return false;
    }

    //-----------------------------------------------------------------------
    bool Zip::fileWrite( const DataChunk& refChunk, const String& strPath, bool bOverwrite /* = false */ )
    {
        return false;
    }

    //-----------------------------------------------------------------------
    bool Zip::fileTest( const String& strFile ) const
    {
        if( unzLocateFile( mArchive, strFile.c_str(), 2 ) == UNZ_OK )
            return true;
        return false;
    }

    //-----------------------------------------------------------------------
    bool Zip::fileCopy( const String& strSrc, const String& strDest, bool bOverwrite )
    {
        return false;
    }

    //-----------------------------------------------------------------------
    bool Zip::fileMove( const String& strSrc, const String& strDest, bool bOverwrite )
    {
        return false;
    }

    //-----------------------------------------------------------------------
    bool Zip::fileDele( const String& strFile )
    {
        return false;
    }

    //-----------------------------------------------------------------------
    bool Zip::fileInfo( const String& strFile, FileInfo** ppInfo ) const
    {
        return true;
    }

    //-----------------------------------------------------------------------
    std::vector<String> Zip::dirGetFiles( const String& strDir ) const
    {
        return const_cast<Zip *>(this)->getAllNamesLike( strDir, "", false );
    }

    //-----------------------------------------------------------------------
    std::vector<String> Zip::dirGetSubs( const String& strDir ) const
    {
        return std::vector<String>();
    }

    //-----------------------------------------------------------------------
    bool Zip::dirDele( const String& strDir, bool bRecursive )
    {
        return false;
    };

    //-----------------------------------------------------------------------
    bool Zip::dirMove( const String& strSrc, const String& strDest, bool bOverwrite )
    {
        return false;
    };

    //-----------------------------------------------------------------------
    bool Zip::dirInfo( const String& strDir, FileInfo** ppInfo ) const
    {
        return false;
    };

    //-----------------------------------------------------------------------
    bool Zip::dirCopy( const String& strSrc, const String& strDest, bool bOverwrite )
    {
        return false;
    };

    //-----------------------------------------------------------------------
    bool Zip::dirTest( const String& strDir ) const
    {
        return false;
    };

    //-----------------------------------------------------------------------
    StringVector Zip::getAllNamesLike( const String& strStartPath, const String& strPattern, bool bRecursive /* = true */ )
    {
        StringVector retVec;
        unz_file_info info;
        char filename[260];
        char extraField[260];
        char comment[260];
        char szPattern[260];

        strncpy( szPattern, strPattern.c_str(), 259 );
        strlwr( szPattern );

        int iRes = unzGoToFirstFile(mArchive);
        while( iRes == UNZ_OK )
        {

            unzGetCurrentFileInfo( mArchive,
                         &info,
                         filename, 259,
                         extraField, 259,
                         comment, 259 );

            if( info.uncompressed_size > 0 )
            {
                strlwr( filename );
                if( strstr( filename, szPattern ) )
                {
                    retVec.push_back(String( filename) );
                }
            }

            iRes = unzGoToNextFile( mArchive );
        }

        return retVec;
    };

    //-----------------------------------------------------------------------
    void Zip::load() {
        struct stat tagStat;
        stat( mName.c_str(), &tagStat );

        if( ( tagStat.st_mode & S_IFDIR ) )
            Except( Exception::ERR_FILE_NOT_FOUND, "Zip archive " + mName + " not found.",
                "Zip::load" );
        else
            mArchive = unzOpen( mName.c_str() );

        LogManager::getSingleton().logMessage( "Zip Archive codec for " + mName + " created.");
    };

    //-----------------------------------------------------------------------
    void Zip::unload() {
        if( mArchive )
            unzClose( mArchive );

        LogManager::getSingleton().logMessage( "Zip Archive Codec for " + mName + " unloaded." );

        delete this;
    };

    //-----------------------------------------------------------------------
    Zip::Zip() {}

    //-----------------------------------------------------------------------
    Zip::Zip( const String& name )
    {
        mName = name;
    }

    //-----------------------------------------------------------------------
    Zip::~Zip() {}

}
