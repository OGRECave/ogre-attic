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
#ifndef __HardwareBufferManager__
#define __HardwareBufferManager__

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreSingleton.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"


namespace Ogre {

    /** Abtract interface representing a 'licensee' of a hardware buffer copy.
    remarks
        Often it's useful to have temporary buffers which are used for working
        but are not necessarily needed permanently. However, creating and 
        destroying buffers is expensive, so we need a way to share these 
        working areas, especially those based on existing fixed buffers. 
        This class represents a licensee of one of those temporary buffers, 
        and must be implemented by any user of a temporary buffer if they 
        wish to be notified when the license is expired. 
    */
    class _OgreExport HardwareBufferLicensee
    {
    public:
        virtual ~HardwareBufferLicensee() { }
        /** This method is called when the buffer license is expired and is about
        to be returned to the shared pool. */
        virtual void licenseExpired(HardwareBuffer* buffer) = 0;
    };

    /** Structure for recording the use of temporary blend buffers */
    class _OgreExport TempBlendedBufferInfo : public HardwareBufferLicensee
    {
    public:
        // Pre-blended 
        HardwareVertexBufferSharedPtr srcPositionBuffer;
        HardwareVertexBufferSharedPtr srcNormalBuffer;
        // Post-blended 
        HardwareVertexBufferSharedPtr destPositionBuffer;
        HardwareVertexBufferSharedPtr destNormalBuffer;
        /// Both positions and normals are contained in the same buffer
        bool posNormalShareBuffer;
        unsigned short posBindIndex;
        unsigned short normBindIndex;

        /// Utility method, checks out temporary copies of src into dest
        void checkoutTempCopies(void);
        /// Utility method, binds dest copies into a given VertexData struct
        void bindTempCopies(VertexData* targetData, bool suppressHardwareUpload);
        /** Overridden member from HardwareBufferLicensee. */
        void licenseExpired(HardwareBuffer* buffer);
    };


    /** Abstract singleton class for managing hardware buffers, a concrete instance
    of this will be created by the RenderSystem. */
    class _OgreExport HardwareBufferManager : public Singleton<HardwareBufferManager>
    {
        friend class HardwareVertexBufferSharedPtr;
        friend class HardwareIndexBufferSharedPtr;
    protected:
        typedef std::list<VertexDeclaration*> VertexDeclarationList;
		typedef std::list<VertexBufferBinding*> VertexBufferBindingList;
        typedef std::set<HardwareVertexBuffer*> VertexBufferList;
        typedef std::set<HardwareIndexBuffer*> IndexBufferList;

        VertexDeclarationList mVertexDeclarations;
		VertexBufferBindingList mVertexBufferBindings;
        VertexBufferList mVertexBuffers;
        IndexBufferList mIndexBuffers;


		/// Destroy a hardware vertex buffer, do not call direct
		virtual void destroyVertexBuffer(HardwareVertexBuffer* buf) = 0;
		/// Destroy a hardware index buffer, do not call direct
		virtual void destroyIndexBuffer(HardwareIndexBuffer* buf) = 0;

        virtual void destroyAllDeclarations(void);
        virtual void destroyAllBindings(void);

    public:

        enum BufferLicenseType
        {
            /// Licensee will only release buffer when it says so
            BLT_MANUAL_RELEASE,
            /// Licensee can have license revoked
            BLT_AUTOMATIC_RELEASE
        };

    protected:
        /** Struct holding details of a license to use a temporary shared buffer. */
        class VertexBufferLicense
        {
        public:
            HardwareVertexBuffer* originalBufferPtr;
            BufferLicenseType licenseType;
            HardwareVertexBufferSharedPtr buffer;
            HardwareBufferLicensee* licensee;
            VertexBufferLicense(
                HardwareVertexBuffer* orig,
                BufferLicenseType ltype, 
                HardwareVertexBufferSharedPtr buf, 
                HardwareBufferLicensee* lic) 
                : originalBufferPtr(orig), licenseType(ltype), buffer(buf), licensee(lic) {}

        };

        /// List of free temporary vertex buffers
        typedef std::vector<HardwareVertexBufferSharedPtr> FreeTemporaryVertexBufferList;
        /// Map from original buffer to list of temporary buffers
        typedef std::map<HardwareVertexBuffer*, FreeTemporaryVertexBufferList*> FreeTemporaryVertexBufferMap;
        /// Map of current available temp buffers 
        FreeTemporaryVertexBufferMap mFreeTempVertexBufferMap;
        /// List of currently licensed temp buffers
        typedef std::vector<VertexBufferLicense> TemporaryVertexBufferLicenseList;
        /// List of currently licensed temp buffers
        TemporaryVertexBufferLicenseList mTempVertexBufferLicenses;

        /// Creates  a new buffer as a copy of the source, does not copy data
        HardwareVertexBufferSharedPtr makeBufferCopy(
            const HardwareVertexBufferSharedPtr& source, 
            HardwareBuffer::Usage usage, bool useShadowBuffer);

    public:
        HardwareBufferManager();
        virtual ~HardwareBufferManager();
		/** Create a hardware vertex buffer.
        @remarks
            This method creates a new vertex buffer; this will act as a source of geometry
            data for rendering objects. Note that because the meaning of the contents of
            the vertex buffer depends on the usage, this method does not specify a
            vertex format; the user of this buffer can actually insert whatever data 
            they wish, in any format. However, in order to use this with a RenderOperation,
            the data in this vertex buffer will have to be associated with a semantic element
            of the rendering pipeline, e.g. a position, or texture coordinates. This is done 
            using the VertexDeclaration class, which itself contains VertexElement structures
            referring to the source data.
        @remarks Note that because vertex buffers can be shared, they are reference
            counted so you do not need to worry about destroying themm this will be done
            automatically.
        @param vertexSize The size in bytes of each vertex in this buffer; you must calculate
            this based on the kind of data you expect to populate this buffer with.
        @param numVerts The number of vertices in this buffer.
        @param usage One or more members of the HardwareBuffer::Usage enumeration; you are
            strongly advised to use HBU_STATIC_WRITE_ONLY wherever possible, if you need to 
            update regularly, consider HBU_DYNAMIC_WRITE_ONLY and useShadowBuffer=true.
		@param useShadowBuffer If set to true, this buffer will be 'shadowed' by one stored in 
            system memory rather than GPU or AGP memory. You should set this flag if you intend 
            to read data back from the vertex buffer, because reading data from a buffer
			in the GPU or AGP memory is very expensive, and is in fact impossible if you
            specify HBU_WRITE_ONLY for the main buffer. If you use this option, all 
            reads and writes will be done to the shadow buffer, and the shadow buffer will
            be synchronised with the real buffer at an appropriate time.
        */
		virtual HardwareVertexBufferSharedPtr 
            createVertexBuffer(size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage, 
			bool useShadowBuffer = false) = 0;
		/** Create a hardware index buffer.
        @remarks Note that because buffers can be shared, they are reference
            counted so you do not need to worry about destroying themm this will be done
            automatically.
		@param itype The type in index, either 16- or 32-bit, depending on how many vertices
			you need to be able to address
		@param numIndexes The number of indexes in the buffer
        @param usage One or more members of the HardwareBuffer::Usage enumeration.
		@param useShadowBuffer If set to true, this buffer will be 'shadowed' by one stored in 
            system memory rather than GPU or AGP memory. You should set this flag if you intend 
            to read data back from the index buffer, because reading data from a buffer
			in the GPU or AGP memory is very expensive, and is in fact impossible if you
            specify HBU_WRITE_ONLY for the main buffer. If you use this option, all 
            reads and writes will be done to the shadow buffer, and the shadow buffer will
            be synchronised with the real buffer at an appropriate time.
        */
		virtual HardwareIndexBufferSharedPtr 
            createIndexBuffer(HardwareIndexBuffer::IndexType itype, size_t numIndexes, 
			HardwareBuffer::Usage usage, bool useShadowBuffer = false) = 0;
        /// Creates a vertex declaration, may be overridden by certain rendering APIs
        virtual VertexDeclaration* createVertexDeclaration(void);
        /// Destroys a vertex declaration, may be overridden by certain rendering APIs
        virtual void destroyVertexDeclaration(VertexDeclaration* decl);

		/** Creates a new VertexBufferBinding. */
		virtual VertexBufferBinding* createVertexBufferBinding(void);
		/** Destroys a VertexBufferBinding. */
		virtual void destroyVertexBufferBinding(VertexBufferBinding* binding);

        /** Allocates a copy of a given vertex buffer.
        @remarks
            This method allocates a temporary copy of an existing vertex buffer.
            This buffer is subsequently stored and can be made available for 
            other purposes later without incurring the cost of construction / 
            destruction.
        @param sourceBuffer The source buffer to use as a copy
        @param licenseType The type of license required on this buffer - automatic
            release causes this class to release licenses every frame so that 
            they can be reallocated anew.
        @param licensee Pointer back to the class requesting the copy, which must
            implement HardwareBufferLicense in order to be notified when the license
            expires.
        @param copyData If true, the current data is copied as well as the 
            structure of the buffer
        */
        virtual HardwareVertexBufferSharedPtr allocateVertexBufferCopy(
            const HardwareVertexBufferSharedPtr& sourceBuffer, 
            BufferLicenseType licenseType,
            HardwareBufferLicensee* licensee,
            bool copyData = false);
        /** Manually release a vertex buffer copy for others to subsequently use.
        @remarks
            Only required if the original call to allocateVertexBufferCopy
            included a licenseType of BLT_MANUAL_RELEASE. 
        @param bufferCopy The buffer copy. The caller is expected to delete
            or at least no longer use this reference, since another user may
            well begin to modify the contents of the buffer.
        */
        virtual void releaseVertexBufferCopy(
            const HardwareVertexBufferSharedPtr& bufferCopy); 

        /** Internal method for releasing all temporary buffers which have been 
           allocated using BLT_AUTOMATIC_RELEASE; is called by OGRE. */
        virtual void _releaseBufferCopies(void);
            
        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton
                implementation is in a .h file, which means it gets compiled
                into anybody who includes it. This is needed for the
                Singleton template to work, but we actually only want it
                compiled into the implementation of the class based on the
                Singleton, not all of them. If we don't change this, we get
                link errors when trying to use the Singleton-based class from
                an outside dll.
            @par
                This method just delegates to the template version anyway,
                but the implementation stays in this single compilation unit,
                preventing link errors.
        */
        static HardwareBufferManager& getSingleton(void);
    };

}

#endif

