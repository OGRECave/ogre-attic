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
#include "OgreStableHeaders.h"

#include "OgreZip.h"

#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreStringVector.h"
#include "OgreRoot.h"


namespace Ogre {

    //-----------------------------------------------------------------------
    ZipArchive::ZipArchive(const String& name, const String& archType )
        : Archive(name, archType), mZzipDir(0)
    {
    }
    //-----------------------------------------------------------------------
    ZipArchive::~ZipArchive()
    {
        unload();
    }
    //-----------------------------------------------------------------------
    void ZipArchive::load()
    {
        if (!mZzipDir)
        {
            zzip_error_t zzipError;
            mZzipDir = zzip_dir_open(mName.c_str(), &zzipError);
            checkZzipError(zzipError, "opening archive");
        }
    }
    //-----------------------------------------------------------------------
    void ZipArchive::unload()
    {
        if (mZzipDir)
        {
            zzip_dir_close(mZzipDir);
            mZzipDir = 0;
        }
    
    }
    //-----------------------------------------------------------------------
	DataStreamPtr ZipArchive::open(const String& filename) const
    {
        // Format not used here (always binary)
        ZZIP_FILE* zzipFile = 
            zzip_file_open(mZzipDir, filename.c_str(), ZZIP_ONLYZIP);
        if (zzip_error(mZzipDir) != ZZIP_NO_ERROR)
		{
			// return null pointer
			return DataStreamPtr();
		}

		// Get uncompressed size too
		ZZIP_STAT zstat;
		zzip_dir_stat(mZzipDir, filename.c_str(), &zstat, 0);

        // Construct & return stream
        return DataStreamPtr(new ZipDataStream(filename, zzipFile, static_cast<size_t>(zstat.d_csize)));

    }
    //-----------------------------------------------------------------------
    void ZipArchive::kill(const String& filename)
    {
        // Not supported
        Except(Exception::ERR_CANNOT_WRITE_TO_FILE, 
            "Writing to zip archives is not currently supported", 
            "ZipArchive::kill");
    }
    //-----------------------------------------------------------------------
    StringVectorPtr ZipArchive::list(bool recursive)
    {
        ZZIP_DIRENT zzipEntry;
        StringVectorPtr ret = StringVectorPtr(new StringVector());

        while (zzip_dir_read(mZzipDir, &zzipEntry))
        {
            String name(zzipEntry.d_name);
			if (recursive || name.find_first_of("/") == String::npos)
            {
                ret->push_back(name);
            }
        }
        return ret;

    }
    //-----------------------------------------------------------------------
    Archive::FileInfoListPtr ZipArchive::listFileInfo(bool recursive)
    {
        ZZIP_DIRENT zzipEntry;
        FileInfoListPtr ret = FileInfoListPtr(new FileInfoList());

        while (zzip_dir_read(mZzipDir, &zzipEntry))
        {
            String name(zzipEntry.d_name);
            // Assume that the presence of '/' means file is in a subdir of the zip
			if (recursive || name.find_first_of("/") == String::npos)
            {
                FileInfo info;
                // Get basename / path
                StringUtil::splitFilename(name, info.basename, info.path);
                // Get sizes
                info.compressedSize = static_cast<size_t>(zzipEntry.d_csize);
                info.uncompressedSize = static_cast<size_t>(zzipEntry.st_size);

                ret->push_back(info);
            }
        }
        return ret;
    }
    //-----------------------------------------------------------------------
    StringVectorPtr ZipArchive::find(const String& pattern, bool recursive, 
        bool caseSensitive)
    {
        ZZIP_DIRENT zzipEntry;
        StringVectorPtr ret = StringVectorPtr(new StringVector());

        while (zzip_dir_read(mZzipDir, &zzipEntry))
        {
            String name(zzipEntry.d_name);
			if (recursive || name.find_first_of("/") == String::npos)
            {
                // Get basename / path
                String basename, path;
                StringUtil::splitFilename(name, basename, path);
                // Check name matches pattern
                if (StringUtil::match(basename, pattern, caseSensitive))
                {
                    ret->push_back(name);
                }
            }
        }
        return ret;
    }
    //-----------------------------------------------------------------------
	Archive::FileInfoListPtr ZipArchive::findFileInfo(const String& pattern, 
        bool recursive, bool caseSensitive)
    {
        ZZIP_DIRENT zzipEntry;
        FileInfoListPtr ret = FileInfoListPtr(new FileInfoList());

        while (zzip_dir_read(mZzipDir, &zzipEntry))
        {
            String name(zzipEntry.d_name);
            // Assume that the presence of '/' means file is in a subdir of the zip
			if (recursive || name.find_first_of("/") == String::npos)
            {
                FileInfo info;
                // Get basename / path
                StringUtil::splitFilename(name, info.basename, info.path);
                // Check name matches pattern
                if (StringUtil::match(info.basename, pattern, caseSensitive))
                {
                    // Get sizes
                    info.compressedSize = static_cast<size_t>(zzipEntry.d_csize);
                    info.uncompressedSize = static_cast<size_t>(zzipEntry.st_size);

                    ret->push_back(info);
                }
            }
        }
        return ret;
    }
    //-----------------------------------------------------------------------
    void ZipArchive::checkZzipError(const zzip_error_t& zzipError, const String& operation) const
    {
        if (zzipError != ZZIP_NO_ERROR)
        {
            String errorMsg;
            switch (zzipError)
            {
            case ZZIP_OUTOFMEM:
                errorMsg = "Out of memory.";
                break;            
            case ZZIP_DIR_OPEN:
            case ZZIP_DIR_STAT: 
            case ZZIP_DIR_SEEK:
            case ZZIP_DIR_READ:
                errorMsg = "Unable to read zip file.";
                break;            
            case ZZIP_UNSUPP_COMPR:
                errorMsg = "Unsupported compression format.";
                break;            
            case ZZIP_CORRUPTED:
                errorMsg = "Corrupted archive.";
                break;            
            default:
                errorMsg = "Unknown error.";
                break;            
            };

            Except(Exception::ERR_INTERNAL_ERROR, 
                mName + " - error whilst " + operation + ": " + errorMsg,
                "ZipArchive::checkZzipError");
        }
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
#define DECOMPRESS_CHUNK_SIZE 128
    //-----------------------------------------------------------------------
    ZipDataStream::ZipDataStream(ZZIP_FILE* zzipFile, size_t uncompressedSize)
        : mZzipFile(zzipFile), mUncompressedSize(uncompressedSize)
    {
    }
    //-----------------------------------------------------------------------
    ZipDataStream::ZipDataStream(const String& name, ZZIP_FILE* zzipFile, size_t uncompressedSize)
        :DataStream(name), mZzipFile(zzipFile), mUncompressedSize(uncompressedSize)
    {
    }
    //-----------------------------------------------------------------------
    size_t ZipDataStream::read(unsigned char*& buf, size_t count)
    {
        return zzip_file_read(mZzipFile, (char*)buf, count);
    }
    //-----------------------------------------------------------------------
    size_t ZipDataStream::readLine(unsigned char*&buf, size_t maxCount, const String& delim)
    {
        // read in chunks
		static char tmpChunk[DECOMPRESS_CHUNK_SIZE+1];
		size_t chunkSize = std::min(maxCount, (size_t)DECOMPRESS_CHUNK_SIZE);
		size_t totalCount = 0;
		size_t readCount; 
		while (chunkSize && (readCount = zzip_file_read(mZzipFile, tmpChunk, chunkSize)))
		{
			// Terminate
			tmpChunk[chunkSize] = '\0';
			// Find first delimiter
			size_t pos = strcspn(tmpChunk, delim.c_str());

			if (pos > 0)
			{
				// Are we genuinely copying?
				if (buf)
				{
					memcpy(buf, (const void*)tmpChunk, pos);
				}
				totalCount += pos;
			}

			if (pos < chunkSize)
			{
				// found terminator, break out
				break;
			}
			// Adjust chunkSize for next time
			chunkSize = std::min(maxCount-totalCount, (size_t)DECOMPRESS_CHUNK_SIZE);
			
		}
		return totalCount;
    }
    //-----------------------------------------------------------------------
    size_t ZipDataStream::skipLine(const String& delim)
    {
		// Re-use readLine, but don't copy data
		unsigned char* nullBuf = 0;
        readLine(nullBuf, 1024, delim);
    }
    //-----------------------------------------------------------------------
    void ZipDataStream::skip(size_t count)
    {
        zzip_seek(mZzipFile, count, SEEK_CUR);
    }
    //-----------------------------------------------------------------------
    void ZipDataStream::seek( size_t pos )
    {
		zzip_seek(mZzipFile, pos, SEEK_SET);
    }
    //-----------------------------------------------------------------------
    bool ZipDataStream::eof(void) const
    {
        return (zzip_tell(mZzipFile) > mUncompressedSize);
    }
    //-----------------------------------------------------------------------
    void ZipDataStream::close(void)
    {
        zzip_file_close(mZzipFile);
    }
    //-----------------------------------------------------------------------


}
