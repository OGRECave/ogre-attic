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

#include <zzip/zzip.h>
#include "OgrePrerequisites.h"

#include "OgreArchive.h"
#include "OgreArchiveFactory.h"

namespace Ogre {

    /** Specialisation of the Archive class to allow reading of files from a zip
        format source archive.
    @remarks
        This archive format supports all archives compressed in the standard
        zip format, including iD pk3 files.
    */
    class ZipArchive : public Archive 
    {
    protected:
        /// Handle to root zip file
        ZZIP_DIR* mZzipDir;
        /// Handle any errors from zzip
        void checkZzipError(const zzip_error_t& zzipError, const String& operation) const;
    public:
        ZipArchive(const String& name, const String& archType );
        ~ZipArchive();
        /// @copydoc Archive::load
        void load();
        /// @copydoc Archive::unload
        void unload();

        /// @copydoc Archive::open
        DataStreamPtr open(const String& filename) const;

        /// @copydoc Archive::kill
        void kill(const String& filename);

        /// @copydoc Archive::list
        StringVectorPtr list(bool recursive = true );

        /// @copydoc Archive::listFileInfo
        FileInfoListPtr listFileInfo(bool recursive = true );

        /// @copydoc Archive::find
        StringVectorPtr find(const String& pattern, bool recursive = true, 
            bool caseSensitive = true);

        /// @copydoc Archive::findFileInfo
        FileInfoListPtr findFileInfo(const String& pattern, bool recursive = true,
            bool caseSensitive = true);

    };

    /** Specialisation of ArchiveFactory for Zip files. */
    class ZipArchiveFactory : public ArchiveFactory
    {
    public:
        virtual ~ZipArchiveFactory() {}
        /// @copydoc FactoryObj::getType
        String getType(void) const { return "Zip"; }       
        /// @copydoc FactoryObj::createInstance
        Archive *createInstance( const String& name ) 
        {
            return new ZipArchive(name, "Zip");
        }
        /// @copydoc FactoryObj::destroyInstance
        void destroyInstance( Archive* arch) { delete arch; }
    };

    /** Specialisation of DataStream to handle streaming data from zip archives. */
    class ZipDataStream : public DataStream
    {
    protected:
        ZZIP_FILE* mZzipFile;
		size_t mUncompressedSize;
    public:
        /// Unnamed constructor
        ZipDataStream(ZZIP_FILE* zzipFile, size_t uncompressedSize);
        /// Constructor for creating named streams
        ZipDataStream(const String& name, ZZIP_FILE* zzipFile, size_t uncompressedSize);
        /// @copydoc DataStream::read
        size_t read(void* buf, size_t count);
        /// @copydoc DataStream::read
        size_t readLine(unsigned char* buf, size_t maxCount, const String& delim = "\n");
        /// @copydoc DataStream::skipLine
        size_t skipLine(const String& delim = "\n");
        /// @copydoc DataStream::skip
        void skip(size_t count);
        /// @copydoc DataStream::seek
        void seek( size_t pos );
        /// @copydoc DataStream::eof
        bool eof(void) const;
        /// @copydoc DataStream::close
        void close(void);


    };


}

#endif
