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
#ifndef _ArchiveEx_H__
#define _ArchiveEx_H__

#include "OgrePrerequisites.h"

#include "OgreResource.h"

namespace Ogre {

#ifdef __BORLANDC__
    using namespace std;
#endif

    /** Archive-handling class.
        @remarks
            This class is supposed to be subclassed in order to create
            specialised archive codecs that should reside in .DLL's/.so's.
        @author
            Adrian Cearnau (cearny@cearny.ro)
        @since
            31st of January 2002
    */
    class _OgreExport ArchiveEx : public Resource {
    public:
        /** Information about a file/directory within the archive will be
            returned using a FileInfo struct.
            @see
                ArchiveEx
        */
        struct FileInfo {
            /// The file's full name
            char szFilename[260];

            /// Compressed size
            int iCompSize;
            /// Uncompressed size
            int iUncompSize;

            /// Last time it was modified
            time_t iLastMod;
        };

    public:
        /** Default constructor.
            @note
                <br>Should never get called.
        */
        ArchiveEx() {}

        /** Usual constructor - used by ArchiveManager.
            @warning
                Never call directly.
        */
        ArchiveEx( const String& name ) { mName = name; }

        /** Default destructor.
        */
        virtual ~ArchiveEx();

        /** Loads the archive.
            @remarks
                This initializes all the internal data of the class.
            @warning
                Do not call this function directly, it is ment to be used
                only by the ArchiveManager class.
        */
        virtual void load()=0;

        /** Unloads the archive.
            @warning
                Do not call this function directly, it is ment to be used
                only by the ArchiveManager class.
        */
        virtual void unload()=0;

        virtual bool fileOpen( const String& strFile, FILE** ppFile ) const=0;

        /** Reads the contents of a file within the archive and copies it in the
            passed DataChunk.
            @param
                strFile The name of the file to read from the archive.
            @param
                ppChunk A pointer to the addres of a DataChunk class that
                will hold the file's contens.
            @returns
                If the function succeeds, <b>true</b> is returned.
            @par
                If the function fails, <b>false</b> is returned.
        */
        virtual bool fileRead( const String& strFile, DataChunk** ppChunk ) const=0;

        /**
            Creates a new file in the archive and copies in it the contents
            of the file associated with the passed FILE stream.
            @param pFile
                A valid FILE pointer that holds the file stream information.
            @param strPath
                The location inside the archive where the new file will be
                created.
            @param bOverwrite
                If set to true, a file with the same name already exists
                inside the archive and appending or deleting is possible,
                the old file is deleted and a new one containing the new data
                is created.
            @returns
                If the function succeeds, <b>true</b> is returned.
            @par
                If the function fails, <b>false</b> is returned.
        */
        virtual bool fileSave( FILE* pFile, const String& strPath, bool bOverwrite = false )=0;

        /**
            Creates a new file in the archive and copies in it the contents
            of the passed DataChunk.
            @param refChunk
                A DataChunk class that contains the data to be written to the
                new file.
            @param strPath
                The location inside the archive where the new file will be
                created.
            @param bOverwrite
                If set to true, a file with the same name already exists
                inside the archive and appending or deleting is possible,
                the old file is deleted and a new one containing the new data
                is created.
            @returns
                If the function succeeds, <b>true</b> is returned.
            @par
                If the function fails, <b>false</b> is returned.
        */
        virtual bool fileWrite( const DataChunk& refChunk, const String& strPath, bool bOverwrite = false )=0;

        /** Deletes a file from the archive.
            @param
                strFile The name of the file te be deleted.
            @returns
                If the function succeeds, <b>true</b> is returned.
            @par
                If the function fails, <b>false</b> is returned.
        */
        virtual bool fileDele( const String& strFile )=0;

        /**
            Moves a file within the archive.
            @param strSrc
                The source file.
            @param strDest
                The location within the archive where to move the file.
            @param bOverwrite
                If set to true, a file with the same name already exists at
                the destination and appending or deleting is possible, the
                old file is deleted and a new one containing the new data is
                created.
            @returns
                If the function succeeds, <b>true</b> is returned.
            @par
                If the function fails, <b>false</b> is returned.
        */
        virtual bool fileMove( const String& strSrc, const String& strDest, bool bOverwrite )=0;

        /** Retrieves information about a file within the archive.
            @param
                strFile The name of the file to retrieve information for.
            @param ppInfo
                Pointer to the memory address of a FileInfo struct that will
                be filled with the requested information.
            @returns
                If the function succeeds, <b>true</b> is returned.
            @par
                If the function fails, <b>false</b> is returned.
        */
        virtual bool fileInfo( const String& strFile, FileInfo** ppInfo ) const=0;

        /** Copies a file within the archive.
            @param strSrc
                The source file.
            @param strDest
                The destination path.
            @param bOverwrite
                If set to true, a file with the same name already exists at
                the destination and appending or deleting is possible, the
                old file is deleted and a new one containing the new data is
                created.
            @returns
                If the function succeeds, <b>true</b> is returned.
            @par
                If the function fails, <b>false</b> is returned.
        */
        virtual bool fileCopy( const String& strSrc, const String& strDest, bool bOverwrite )=0;

        /** Tests wheter a file within the archive can be succesfully read.
            @param strFile
                The name of the file to be tested.
            @returns
                If the file can be read, <b>true</b> is returned.
            @par
                If the file cannot be read, <b>false</b> is returned.
        */
        virtual bool fileTest( const String& strFile ) const=0;

        /** Finds the names of all the files within a directory inside the
            archive.
            @param strDir
                The directory's path.
            @returns
                A StringVector containing all the files.
        */
        virtual std::vector<String> dirGetFiles( const String& strDir ) const=0;

        /** Finds the names of all the subdirectories within a directory
            inside the archive.
            @param strDir
                The directory's path.
            @returns
                A StringVector containing the names of all the
                subdirectories.
        */
        virtual std::vector<String> dirGetSubs( const String& strDir ) const=0;

        /** Deletes a directory from the archive.
            @param strDir
                The path to the directory to delete.
            @param bRecursive
                If set to true, all the subdirectories of the given directory
                will also be deleted.
            @returns
                If the function succeeds, <b>true</b> is returned.
            @par
                If the function fails, <b>false</b> is returned.
            @note
                <br>If the directory contains other subdirectories, and the
                <code>bRecursive</code> parameter is set to false, only the
                files in the current directory are deleted and the
                directory will not be removed.
        */
        virtual bool dirDele( const String& strDir, bool bRecursive )=0;
        virtual bool dirMove( const String& strSrc, const String& strDest, bool bOverwrite )=0;

        virtual bool dirInfo( const String& strDir, FileInfo** ppInfo ) const=0;
        virtual bool dirCopy( const String& strSrc, const String& strDest, bool bOverwrite )=0;

        virtual bool dirTest( const String& strDir ) const=0;

        virtual std::vector<String> getAllNamesLike( const String& strStartPath, const String& strPattern, bool bRecursive=true )=0;

        /** This function gets called by the resource manager to know
            if it is okay to cahche all the files in the achive.
            @remarks
                Allowing file caching may not always be a very good idea.
                For example, caching all the files on a distant web server
                would work very slowly.
            @since 29.04.2002
        */
        virtual bool _allowFileCaching() const =0;
    };
}

#endif
