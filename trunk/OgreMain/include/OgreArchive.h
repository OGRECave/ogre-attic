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
#ifndef _Archive_H__
#define _Archive_H__

#include "OgrePrerequisites.h"
#include "OgreDataStream.h"
#include "OgreSharedPtr.h"
#include "OgreStringVector.h"

namespace Ogre {


    /** Archive-handling class.
    @remarks
        An archive is a generic term for a container of files. This may be a
        filesystem folder, it may be a compressed archive, it may even be 
        a remote location shared on the web. This class is designed to be 
        subclassed to provide access to a range of file locations. 
    @par
        Instances of this class are never constructed or even handled by end-user
        applications. They are constructed by custom ArchiveFactory classes, 
        which plugins can register new instances of using ArchiveManager. 
        End-user applications will typically use ResourceManager or 
        ResourceGroupManager to manage resources at a higher level, rather than 
        reading files directly through this class. Doing it this way allows you
        to benefit from OGRE's automatic searching of multiple file locations 
        for the resources you are looking for.
    @par
        Note that this class provides read-only access to files at this time.
    */
    class _OgreExport Archive  
    {
    public:
        /** Information about a file/directory within the archive will be
        returned using a FileInfo struct.
        @see
        Archive
        */
        struct FileInfo {
            /// The file's fully qualified name
            String filename;
            /// Path name; separated by '/' and ending with '/'
            String path;
            /// Base filename
            String basename;
            /// Compressed size
            size_t compressedSize;
            /// Uncompressed size
            size_t uncompressedSize;
            /// Last time it was modified
            time_t lastModified;
        };

        typedef std::vector<FileInfo> FileInfoList;
        typedef SharedPtr<FileInfoList> FileInfoListPtr;

        /// File access modes
        enum AccessMode
        {
            /// Read-only access
            READ, 
            /// Write access, overwrite any exsiting file
            WRITE,
            /// Append to the end of an existing file
            APPEND
        };
        /// File format flags
        enum FileFormat
        {
            /// Text format, includes carriage-return and character conversions
            TEXT,
            /// Binary format, no conversions
            BINARY
        };
    public:
        /** Default constructor.
        @note
        Should never get called.
        */
        Archive() {}

        /** Usual constructor - used by ArchiveManager.
        @warning
            Never call directly.
        */
        Archive( const String& name ) { mName = name; }

        /** Default destructor.
        */
        virtual ~Archive();

        /** Loads the archive.
        @remarks
            This initializes all the internal data of the class.
        @warning
            Do not call this function directly, it is ment to be used
            only by the ArchiveManager class.
        */
        virtual void load() = 0;

        /** Unloads the archive.
        @warning
            Do not call this function directly, it is ment to be used
            only by the ArchiveManager class.
        */
        virtual void unload() = 0;

        /** Open a stream on a given file. 
        @note
            There is no equivalent 'close' method; the returned stream
            controls the lifecycle of this file operation.
        @param filename The name of the fully-qualified filename
        @param format Whether this file is to be opened as text or binary
        @param mode Access mode (read / write / append)
        @returns A shared pointer to a DataStream which can be used to 
            read / write the file
        */
        virtual DataStreamPtr open(const String& filename, FileFormat format,
            AccessMode mode = IN) const = 0;

        /** Remove a specific file from the Archive. 
        @param filename The filename to remove.
        */
        virtual void kill(const String& filename) = 0;

        /** List all file names in the archive.
        @note
            This method only returns filenames, you can also retrieve other
            information using listFileInfo.
        @param recursive Whether all paths of the archive are searched (if the 
            archive has a concept of that)
        @returns A list of filenames matching the criteria, all are fully qualified
        */
        virtual StringVectorPtr list(bool recursive = true ) = 0;
        
        /** List all files in the archive with accompanying information.
        @param recursive Whether all paths of the archive are searched (if the 
            archive has a concept of that)
        @returns A list of structures detailing quite a lot of information about
            all the files in the archive.
        */
        virtual FileInfoListPtr listFileInfo(bool recursive = true ) = 0;

        /** Find all file names matching a given pattern in this archive.
        @note
            This method only returns filenames, you can also retrieve other
            information using findFileInfo.
        @param pattern The pattern to search for; wildcards (*) are allowed
        @param recursive Whether all paths of the archive are searched (if the 
            archive has a concept of that)
        @returns A list of filenames matching the criteria, all are fully qualified
        */
        virtual StringVectorPtr find(const String& pattern, bool recursive = true ) = 0;

        /** Find all files matching a given pattern in this archive and get 
            some detailed information about them.
        @param pattern The pattern to search for; wildcards (*) are allowed
        @param recursive Whether all paths of the archive are searched (if the 
        archive has a concept of that)
        @returns A list of file information structures for all files matching 
            the criteria.
        */
        virtual FileInfoListPtr findFileInfo(const String& pattern, bool recursive = true ) = 0;
        
    };
}

#endif
