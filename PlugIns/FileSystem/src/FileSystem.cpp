// FileSystem.cpp : Defines the entry point for the DLL application.
//

#include "FileSystem.h"
#include "FileSystemFactory.h"
#include "OgreLogManager.h"
#include "OgreArchiveManager.h"
#include "OgreException.h"
#include "OgreStringVector.h"
#include "OgreRoot.h"



#include <sys/types.h>
#include <sys/stat.h>

#if OGRE_PLATFORM == PLATFORM_LINUX || OGRE_PLATFORM == PLATFORM_APPLE
#   include "SearchOps.h"
#   include <sys/param.h>
#   define MAX_PATH MAXPATHLEN
#endif

#if OGRE_PLATFORM == PLATFORM_WIN32
#   include <windows.h>
#   include <direct.h>
#   include <io.h>
#endif

/*#ifdef _INC_STDIO
#endif*/

namespace Ogre {
    //-----------------------------------------------------------------------
    FileSystemFactory* pFSFactory = NULL;
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    extern "C" void dllStartPlugin(void)
    {
        SET_TERM_HANDLER;

        pFSFactory = new FileSystemFactory();
        ArchiveManager::getSingleton().addArchiveFactory(pFSFactory);
    }

    //-----------------------------------------------------------------------
    extern "C" void dllStopPlugin(void)
    {
        delete pFSFactory;
    }

    //-----------------------------------------------------------------------
    bool FileSystem::fileOpen( const String& strFile, FILE **ppFile ) const
    {
        setPath();

        FILE *pFile;
        pFile = fopen( strFile.c_str(), "r+b" );
        *ppFile = pFile;

        retunset(true);
    }

    //-----------------------------------------------------------------------
    bool FileSystem::fileRead( const String& strFile, DataChunk **ppChunk ) const
    {
        setPath();

        struct stat tagStat;
        DataChunk *pChunk = *ppChunk;

        int ret = stat( strFile.c_str(), &tagStat );
        assert(ret == 0 && "Problem getting file size" );

        pChunk->allocate( tagStat.st_size );

        FILE* pFile = fopen( strFile.c_str(), "rb" );

        fread( (void*)pChunk->getPtr(), tagStat.st_size, 1, pFile );
        fclose( pFile );

        retunset(true);
    }

    //-----------------------------------------------------------------------
    bool FileSystem::fileSave( FILE *pFile, const String& strPath, bool bOverwrite /* = false */ )
    {
        setPath();

        FILE *pArchFile = fopen( strPath.c_str(), "r" );
        if( !pArchFile || ( pFile && bOverwrite ) ) {
            if( pArchFile )
                freopen( strPath.c_str(), "wb", pArchFile );
            else
                pArchFile = fopen( strPath.c_str(), "wb" );

            long lPos = ftell( pFile );
            fseek( pFile, 0, SEEK_END );
            long lSize = ftell( pFile );
            fseek( pFile, 0, SEEK_SET );

            unsigned char *szBuffer = new unsigned char[lSize];
            fread( (void*)szBuffer, lSize, 1, pFile );
            fwrite( (const void*)szBuffer, lSize, 1, pArchFile );
            delete[] szBuffer;

            fseek( pFile, lPos, SEEK_SET );
            fclose( pArchFile );

            retunset(true);
        }
        retunset(false);
    }

    //-----------------------------------------------------------------------
    bool FileSystem::fileWrite( const DataChunk& refChunk, const String& strPath, bool bOverwrite /* = false */ )
    {
        setPath();

        FILE* pFile = fopen( strPath.c_str(), "r" );
        if( !pFile || ( pFile && bOverwrite ) ) {
            if( pFile )
                freopen( strPath.c_str(), "wb", pFile );
            else
                pFile = fopen( strPath.c_str(), "wb" );

            fwrite( (const void*)refChunk.getPtr(), refChunk.getSize(), 1, pFile );
            fclose(pFile);

            retunset(true);
        }

        retunset(false);
    }

    //-----------------------------------------------------------------------
    bool FileSystem::fileTest( const String& strFile ) const
    {
        setPath();

        char szPath[MAX_PATH+1];
        getcwd( szPath, MAX_PATH);

        struct stat tagStat;
        if(!stat( strFile.c_str(), &tagStat ))
            retunset(true);
        retunset(false);
    }

    //-----------------------------------------------------------------------
    bool FileSystem::fileCopy( const String& strSrc, const String& strDest, bool bOverwrite )
    {
        setPath();

        FILE* pSrcFile, *pDestFile;
        struct stat tagStat;
        int iCh;

        if( strSrc != strDest )
        {
            pDestFile = fopen(strDest.c_str(), "r" );

            if( pDestFile == NULL )
                pDestFile = fopen( strDest.c_str(), "wb" );
            else if( pDestFile != NULL && bOverwrite == true ) {
                fclose( pDestFile );
                pDestFile = fopen( strDest.c_str(), "wb" );
            }

            if( pDestFile == 0 )
                retunset(false);

            pSrcFile = fopen( strSrc.c_str(), "rb" );
            if( !pSrcFile )
                retunset(false);

            stat( strSrc.c_str(), &tagStat );

            for( long lI=0; lI<tagStat.st_size; lI++ ) {
                iCh = fgetc( pSrcFile );
                fputc( iCh, pDestFile );
            }

            fclose( pDestFile );
            fclose( pSrcFile );

            retunset(true);
        }

        retunset(false);
    }

    //-----------------------------------------------------------------------
    bool FileSystem::fileMove( const String& strSrc, const String& strDest, bool bOverwrite )
    {
        setPath();

        if( fileCopy( strSrc, strDest, bOverwrite ) ) {
            if( unlink( strSrc.c_str() ) ) {
                retunset(true);
            }
            else {
                retunset(false);
            }
        } else
            retunset(false);
    }

    //-----------------------------------------------------------------------
    bool FileSystem::fileDele( const String& strFile )
    {
        setPath();

        FILE* pFile = fopen( strFile.c_str(), "r" );

        if( !pFile )
            retunset(false);
        fclose( pFile );

        unlink( strFile.c_str() );

        retunset(true);
    }

    //-----------------------------------------------------------------------
    bool FileSystem::fileInfo( const String& strFile, FileInfo** ppInfo ) const
    {
        setPath();
        FileInfo* pInfo = *ppInfo;;

        struct stat tagStat;
        if( !stat( strFile.c_str(), &tagStat ) ) {
            pInfo = NULL;
            retunset(false);
        }

        pInfo->iCompSize = pInfo->iUncompSize = tagStat.st_size;
        pInfo->iLastMod = tagStat.st_mtime;

        strcpy( pInfo->szFilename, strFile.c_str() );

        retunset(true);
    }

    //-----------------------------------------------------------------------
    void FileSystem::setPath() const
    {
        getcwd( mszTempPath, OGRE_MAX_PATH );
        chdir( mstrBasePath.c_str() );
    }

    //-----------------------------------------------------------------------
    void FileSystem::unsetPath() const
    {
            chdir( mszTempPath );
    }

    //-----------------------------------------------------------------------
    std::vector<String> FileSystem::dirGetFiles( const String& strDir ) const
    {
        std::vector<String> vec;

        setPath();
        if( chdir( strDir.c_str() ) == -1 )
            Except(Exception::ERR_FILE_NOT_FOUND, "Cannot open requested directory", "FileSystem::dirGetFiles");

        long lHandle;
        struct _finddata_t tagData;

        if( ( lHandle = _findfirst( "*.*", &tagData ) ) != -1 ) {
            _findnext( lHandle, &tagData );

            // okay, we skipped . and .., get to the good stuff
            while( _findnext( lHandle, &tagData ) == 0 )
                if( !(tagData.attrib & _A_SUBDIR) ) {
                    String strTemp = tagData.name;
                    vec.push_back( strTemp );
                }

            _findclose(lHandle);
        }

        retunset(vec);
    };

    //-----------------------------------------------------------------------
    std::vector<String> FileSystem::dirGetSubs( const String& strDir ) const
    {
        std::vector<String> vec;

        setPath();
        if( chdir( strDir.c_str() ) == -1 )
            Except(Exception::ERR_FILE_NOT_FOUND, "Cannot open requested directory", "FileSystem::dirGetFiles");

        long lHandle;
        struct _finddata_t tagData;

        if( ( lHandle = _findfirst( "*.*", &tagData ) ) != -1 ) {
            _findnext( lHandle, &tagData );

            // okay, we skipped . and .., get to the good stuff
            while( _findnext( lHandle, &tagData ) == 0 )
                if( tagData.attrib & _A_SUBDIR ) {
                    String strTemp = tagData.name;
                    vec.push_back( strTemp );
                }

            _findclose(lHandle);
        }

        retunset(vec);
    };

    //-----------------------------------------------------------------------
    bool FileSystem::dirDele( const String& strDir, bool bRecursive )
    {
        setPath();
        if( chdir( strDir.c_str() ) == -1 )
            retunset(true);

        recursDeleDir( bRecursive );
        if( rmdir( strDir.c_str() ) == -1 )
            retunset(false);

        retunset(true);
    };

    //-----------------------------------------------------------------------
    bool FileSystem::dirMove( const String& strSrc, const String& strDest, bool bOverwrite )
    {
        return true;
    };

    //-----------------------------------------------------------------------
    bool FileSystem::dirInfo( const String& strDir, FileInfo** ppInfo ) const
    {
        return true;
    };

    //-----------------------------------------------------------------------
    bool FileSystem::dirCopy( const String& strSrc, const String& strDest, bool bOverwrite )
    {
        return true;
    };

    //-----------------------------------------------------------------------
    bool FileSystem::dirTest( const String& strDir ) const
    {
        return true;
    };

    //-----------------------------------------------------------------------
    std::vector<String> FileSystem::getAllNamesLike( const String& strStartPath, const String& strPattern, bool bRecursive /* = true */ )
    {
        std::vector<String> retList;

        long lHandle, res;
        struct _finddata_t tagData;

        setPath();
        chdir(strStartPath.c_str());

        lHandle = _findfirst(("*" + strPattern).c_str(), &tagData);
        res = 0;
        while (lHandle != -1 && res != -1)
        {
            retList.push_back(tagData.name);
            res = _findnext( lHandle, &tagData );
        }
        if(lHandle != -1)
            _findclose(lHandle);

        retunset(retList);
    };

    //-----------------------------------------------------------------------
    void FileSystem::recursDeleDir( bool bRecursive )
    {
        long lHandle;
        struct _finddata_t tagData;

        if( ( lHandle = _findfirst( "*.*", &tagData ) ) != -1 ) {
            _findnext( lHandle, &tagData );

            while( _findnext( lHandle, &tagData ) == 0 )
                if( !( tagData.attrib & _A_SUBDIR ) )
                    unlink( tagData.name );
                else if( bRecursive ) {
                    chdir( tagData.name );
                    recursDeleDir( true );
                    rmdir( tagData.name );
                }
            _findclose(lHandle);
        }

        chdir( ".." );
    }

    //-----------------------------------------------------------------------
    void FileSystem::load() {

        /* SJS: Dropped the 'stat' check
        For starters must check the result from stat - if non-zero the struct is unreliable (garbage)
        After running a number of tests stat never seemed to work for folders anyway, the result was always
        -1 and the S_IFDIR check was purely random. Should work but doesn't?

        struct stat tagStat;
        int statResult;

        stat( mName.c_str(), &tagStat );

        if( !( tagStat.st_mode & S_IFDIR ) )
            throw Exception(Exception::ERR_ITEM_NOT_FOUND, "Cannot find folder " + mName, "FileSystem::load");
        else
            mstrBasePath = mName;
        */

        /* cearny: Simplest solution is always the best. If I can't chdir to the given path, then it musn't
        be a directory */
        char szCurrPath[MAX_PATH];
        getcwd( szCurrPath, MAX_PATH );
        if( chdir( mName.c_str() ) )
            Except( Exception::ERR_ITEM_NOT_FOUND, "Cannot find folder " + mName, "FileSystem::load" );
        chdir( szCurrPath );

        mstrBasePath = mName;

        LogManager::getSingleton().logMessage( "FileSystem Archive Codec for " + mName + " created.");
        mIsLoaded = true;
    };

    //-----------------------------------------------------------------------
    void FileSystem::unload() {
        LogManager::getSingleton().logMessage( "FileSystem Archive Codec for " + mName + " unloaded.");
        delete this;
    };

    //-----------------------------------------------------------------------
    FileSystem::FileSystem() {}

    //-----------------------------------------------------------------------
    FileSystem::FileSystem( const String& name )
    {
        mName = name;
    }

    //-----------------------------------------------------------------------
    FileSystem::~FileSystem() {}
}
