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
#ifndef _DataChunk_H__
#define _DataChunk_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"

namespace Ogre {
    /** Wraps a chunk of memory, storing both size and a pointer to the data.
	    @remarks
		    This class simply wraps a chunk of memory. It provides extra info
		    about the size of the data contained within it, simple allocation
		    and free methods, and simple stream-like 'read' methods for
		    parsing through the memory chunk rather like a file.
	    @note
		    <br>By default the DataChunk object will <b>NOT</b> free any
		    memory it has allocated <b>EVEN WHEN DESTROYED</b>.
	    @par
		    This is for efficiency since you may just want to reuse the
		    memory as-is rather than copying it elsewhere, and it also allows
		    the DataChunk to be copied safely, i.e. when returned from
		    methods. At some point you should call DataChunk::free to release
		    the memory if you wish.
	    @par
		    If you need a DataChunk that frees the allocated memory on
		    destruction, use SDDataChunk instead.
	    @see
		    SDDataChunk
    */
    class _OgreExport DataChunk
    {
    protected:
	    uchar* mData;
	    uchar* mPos;
	    uchar* mEnd;
	    size_t mSize;

    public:
	    /** Default constructor.
	    */
	    DataChunk();

	    /** Wraps an existing, already-allocated, memory block.
	    */
	    DataChunk( void *pData, size_t size );

	    /** Default destructor.
		    @note
			    The destructor <b>DOES NOT FREE</b> memory.
	    */
	    virtual ~DataChunk() {}

	    /** Allocates the passed number of bytes.
	    */
	    uchar * allocate( size_t size, const uchar * ptr = NULL );

	    /** Frees all internally allocated memory.
	    */
	    DataChunk & clear();

	    /** Returns the size of the allocated chunk in bytes.
	    */
	    size_t getSize() const;	

	    /** Returns a pointer to the start of the memory.
	    */
	    uchar * getPtr();

	    /** Returns a const pointer to the start of the memory.
	    */
	    const uchar* getPtr() const;

        /** Returns a pointer to the current position in memory.
        */
        uchar * getCurrentPtr();

        /** Returns a const pointer to the current position in memory.
        */
        const uchar* getCurrentPtr() const;

        /** Reads memory from the main buffer into another, incrementing an
		    internal 'current' pointer to allow progressive reads.
		    @param
			    buffer Pointer to buffer to read into
		    @param
			    size Number of bytes to read
		    @returns
			    The number of bytes actually read
	    */
	    size_t read(void* buffer, size_t size);

	    /** Repositions the internal read pointer to a specified byte.
	    */
	    DataChunk & seek( size_t pos );

	    /** Moves the internal read pointer backwards or forwards by the number of bytes specified. 
	    @remarks The sign of the parameter determines the direction of the skip.
	    */
	    DataChunk & skip( long offset );

	    /** Reads data into the provided buffer until hitting the specified
		    character or reaching the upper limit provided.
		    @remarks
			    The terminating character is not included in the data
			    returned, and it is skipped over so the next read will occur
			    after it.
		    @param
			    buffer Pointer to buffer to read into
		    @param
			    size Size of the buffer i.e. max bytes to read
		    @param
			    delim List of delimiters to read up to (default newline)
		    @returns
			    The actual number of characters copied into the buffer.
	    */
	    size_t readUpTo(
		    void* buffer, 
		    size_t size, 
		    const char* delim = "\n" );

        /** Skips data into the provided buffer until hitting the specified
            character or reaching the end of the data.
            @remarks
                The terminating characters is not included in the data
                returned, and it is skipped over so the next read will occur
                after it.
            @param
                delim List of delimiters to skip up to.
            @returns
                The number of characters that were skipped.
        */
        size_t skipUpTo( const char* delim );

	    /** Returns true if the buffer pointer has reached the end of the
		    buffer.
	    */
	    bool isEOF() const;

	    /** Returns a String containing the next line of data, optionally 
		    trimmed for whitespace. 
	    @remarks
		    This is a convenience method for text chunks only, allowing you to 
		    retrieve a String object containing the next line of data. The data
		    is read up to the next newline character and the result trimmed if
		    required.
	    @param 
		    trimAfter If true, the line is trimmed for whitespace (as in 
		    String.trim(true,true))
	    */
	    String getLine( bool trimAfter = true );

        /** Get the entire contents of the chunk as a string. */
        String getAsString(void) const;
    };

} // namespace

#endif
