/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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
#ifndef __DataStream_H__
#define __DataStream_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreSharedPtr.h"

namespace Ogre {

	/** General purpose class used for encapsulating the reading of data.
	@remarks
		This class performs basically the same tasks as std::basic_istream, 
		except that it does not have any formatting capabilities, and is
		designed to be subclassed to receive data from multiple sources,
		including libraries which have no compatiblity with the STL's
		stream interfaces. As such, this is an abstraction of a set of 
		wrapper classes which pretend to be standard stream classes but 
		can actually be implemented quite differently. 
	@par
		Generally, if a plugin or application provides an ArchiveFactory, 
		it should also provide a DataStream subclass which will be used
		to stream data out of that ArchiveEx implementation, unless it can 
		use one of the common implementations included.
	@note
		Ogre makes no guarantees about thread safety, for performance reasons.
		If you wish to access stream data asynchronously then you should
		organise your own mutexes to avoid race conditions. 
	*/
	class _OgreExport DataStream
	{
	protected:
		/// The name (e.g. resource name) that can be used to identify the source fot his data (optional)
		String mName;		
        /// Size of the data in the stream (may be 0 if size cannot be determined)
        size_t mSize;
	public:
		/// Constructor for creating unnamed streams
		DataStream() {}
		/// Constructor for creating named streams
		DataStream(const String& name) : mName(name) {}
		/// Returns the name of the stream, if it has one.
		const String& getName(void) { return mName; }
		virtual ~DataStream() {}
		// Streaming operators
		virtual DataStream& operator>>(char& val);
		virtual DataStream& operator>>(unsigned char& val);
		virtual DataStream& operator>>(bool& val);
		virtual DataStream& operator>>(short& val);
		virtual DataStream& operator>>(unsigned short& val);
		virtual DataStream& operator>>(int& val);
		virtual DataStream& operator>>(unsigned int& val);
		virtual DataStream& operator>>(long& val);
		virtual DataStream& operator>>(unsigned long& val);
		virtual DataStream& operator>>(float& val);
		virtual DataStream& operator>>(double& val);
		virtual DataStream& operator>>(long double& val);
		virtual DataStream& operator>>(void*& val);
		/** Read the requisite number of bytes from the stream, 
			stopping at the end of the file.
		@param buf Reference to a buffer pointer
		@param count Number of bytes to read
		@returns The number of bytes read
		*/
		virtual size_t read(unsigned char*& buf, size_t count) = 0;
		/** Get a single line from the stream.
		@remarks
			The terminating character is not included in the data
			returned, and it is skipped over so the next read will occur
			after it.
		@param buf Reference to a buffer pointer
		@param maxCount The maximum length of data to be read
		@param delim The delimiter to stop at
		@returns The number of bytes read
		*/
		virtual size_t readLine(unsigned char*&buf, size_t maxCount, char delim = '\n') = 0;
		
	    /** Returns a String containing the next line of data, optionally 
		    trimmed for whitespace. 
	    @remarks
		    This is a convenience method for text streams only, allowing you to 
		    retrieve a String object containing the next line of data. The data
		    is read up to the next newline character and the result trimmed if
		    required.
	    @param 
		    trimAfter If true, the line is trimmed for whitespace (as in 
		    String.trim(true,true))
	    */
	    virtual String getLine( bool trimAfter = true );

		/** Skip a single line from the stream.
		@param delim The delimiter to stop at
		@returns The number of bytes skipped
		*/
		virtual size_t skipLine(char delim = '\n') = 0;

		/** Skip a defined number of bytes. */
		virtual void skip(size_t count) = 0;
	
	    /** Repositions the read point to a specified byte.
	    */
	    virtual void seek( size_t pos ) = 0;

		/** Returns true if the stream has reached the end.
	    */
	    virtual bool eof(void) const = 0;

		/** Returns the total size of the data to be read from the stream, 
			or 0 if this is indeterminate for this stream. 
		*/
        size_t size(void) const { return mSize; }

        /** Close the stream; this makes further operations invalid. */
        virtual void close(void) = 0;
		

	};

	/** Shared pointer to allow data streams to be passed around without
		worrying about deallocation
	*/
	typedef SharedPtr<DataStream> DataStreamPtr;

	/// List of DataStream items
	typedef std::list<DataStreamPtr> DataStreamList;
	/// Shared pointer to list of DataStream items
	typedef SharedPtr<DataStreamList> DataStreamListPtr;

	/** Common subclass of DataStream for handling data from chunks of memory.
	*/
	class _OgreExport MemoryDataStream : public DataStream
	{
	protected:
	    uchar* mData;
	    uchar* mPos;
	    uchar* mEnd;
	    size_t mSize;
		bool mFreeOnDestroy;			
	public:
		
		/** Wrap an existing memory chunk in a stream.
		@param pMem Pointer to the exising memory
		@param size The size of the memory chunk in bytes
		@param freeOnDestroy If true, the memory associated will be destroyed
			when the stream is destroyed.
		*/
		MemoryDataStream(void* pMem, size_t size, bool freeOnDestroy = false);
		
		/** Wrap an existing memory chunk in a named stream.
		@param name The name to give the stream
		@param pMem Pointer to the exising memory
		@param size The size of the memory chunk in bytes
		@param freeOnDestroy If true, the memory associated will be destroyed
			when the stream is destroyed.
		*/
		MemoryDataStream(const String& name, void* pMem, size_t size, 
				bool freeOnDestroy = false);

		/** Create a stream which pre-buffers the contents of another stream.
		@remarks
			This constructor can be used to intentionally read in the entire
			contents of another stream, copying them to the internal buffer
			and thus making them available in memory as a single unit.
		@param sourceStream Another DataStream which will provide the source
			of data
		@param freeOnDestroy If true, the memory associated will be destroyed
			when the stream is destroyed.
		*/
		MemoryDataStream(DataStream& sourceStream, 
				bool freeOnDestroy = true);
		
		/** Create a named stream which pre-buffers the contents of 
			another stream.
		@remarks
			This constructor can be used to intentionally read in the entire
			contents of another stream, copying them to the internal buffer
			and thus making them available in memory as a single unit.
		@param name The name to give the stream
		@param sourceStream Another DataStream which will provide the source
			of data
		@param freeOnDestroy If true, the memory associated will be destroyed
			when the stream is destroyed.
		*/
		MemoryDataStream(const String& name, const DataStream& sourceStream, 
				bool freeOnDestroy = true);

		/** Create a stream with a brand new empty memory chunk.
		@param size The size of the memory chunk to create in bytes
		@param freeOnDestroy If true, the memory associated will be destroyed
			when the stream is destroyed.
		*/
		MemoryDataStream(size_t size, bool freeOnDestroy = true);
		/** Create a named stream with a brand new empty memory chunk.
		@param name The name to give the stream
		@param size The size of the memory chunk to create in bytes
		@param freeOnDestroy If true, the memory associated will be destroyed
			when the stream is destroyed.
		*/
		MemoryDataStream(const String& name, size_t size, 
				bool freeOnDestroy = true);

		~MemoryDataStream();

		/** Get a pointer to the start of the memory block this stream holds. */
		uchar* getPtr(void) { return mData; }
		
		/** Get a pointer to the current position in the memory block this stream holds. */
		uchar* getCurrentPtr(void) { return mPos; }
		
		/** @copydoc DataStream::read
		*/
		size_t read(unsigned char*& buf, size_t count);
		/** @copydoc DataStream::readLine
		*/
		size_t readLine(unsigned char*&buf, size_t maxCount, char delim = '\n');
		
		/** @copydoc DataStream::skipLine
		*/
		size_t skipLine(char delim = '\n');

		/** @copydoc DataStream::skip
		*/
		void skip(size_t count);
	
		/** @copydoc DataStream::seek
		*/
	    void seek( size_t pos );

		/** @copydoc DataStream::eof
		*/
	    bool eof(void) const;

        /** @copydoc DataStream::close
        */
        void close(void);
	};

	/** Common subclass of DataStream for handling data from 
		std::basic_istream.
	*/
	class _OgreExport BasicIstreamDataStream : public DataStream
	{
	protected:
		std::basic_istream mStream;
	public:
		/// Construct stream from an STL stream
		BasicIstreamDataStream(std::basic_istream& istream);
		/// Construct named stream from an STL stream
		BasicIstreamDataStream(const String& name, std::basic_istream& istream);

		~BasicIstreamDataStream();

		/** @copydoc DataStream::read
		*/
		size_t read(unsigned char*& buf, size_t count);
		/** @copydoc DataStream::readLine
		*/
		size_t readLine(unsigned char*&buf, size_t maxCount, char delim = '\n');
		
		/** @copydoc DataStream::skipLine
		*/
		size_t skipLine(char delim = '\n');

		/** @copydoc DataStream::skip
		*/
		void skip(size_t count);
	
		/** @copydoc DataStream::seek
		*/
	    void seek( size_t pos );

		/** @copydoc DataStream::eof
		*/
	    bool eof(void) const;

        /** @copydoc DataStream::close
        */
        void close(void);
		
		
	};

	/** Common subclass of DataStream for handling data from C-style file 
		handles.
	*/
	class _OgreExport FileHandleDataStream : public DataStream
	{
	protected:
		FILE* mFileHandle;
	public:
		/// Create stream from a C file handle
		FileHandleDataStream(FILE*& handle);
		/// Create named stream from a C file handle
		FileHandleDataStream(const String& name, FILE*& handle);
	
		/** @copydoc DataStream::read
		*/
		size_t read(unsigned char*& buf, size_t count);
		/** @copydoc DataStream::readLine
		*/
		size_t readLine(unsigned char*&buf, size_t maxCount, char delim = '\n');
		
		/** @copydoc DataStream::skipLine
		*/
		size_t skipLine(char delim = '\n');

		/** @copydoc DataStream::skip
		*/
		void skip(size_t count);
	
		/** @copydoc DataStream::seek
		*/
	    void seek( size_t pos );

		/** @copydoc DataStream::eof
		*/
	    bool eof(void) const;

        /** @copydoc DataStream::close
        */
        void close(void);

	};
}
#endif

