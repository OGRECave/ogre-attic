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
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef __SharedPtr_H__
#define __SharedPtr_H__

#include "OgrePrerequisites.h"

namespace Ogre {

	/** Reference-counted shared pointer, used for objects where implicit destruction is 
        required. 
    @remarks
        This is a standard shared pointer implementation which uses a reference 
        count to work out when to delete the object. OGRE does not use this class
        very often, because it is usually more efficient to make the destruction
        of objects more intentional (in blocks, say). However in some cases you
        really cannot tell how many people are using an object, and this approach is
        worthwhile (e.g. ControllerValue)
    */
    template<class T> class SharedPtr {
	protected:
		T* pRep;
		unsigned int* pUseCount;
	public:
		/** Constructor, does not initialise the SharedPtr.
			@remarks
				<b>Dangerous!</b> You have to call bind() before using the SharedPtr.
		*/
		SharedPtr() : pRep(0), pUseCount(0) {}
		SharedPtr(T* rep) : pRep(rep), pUseCount(new unsigned int(1)) {}
		SharedPtr(const SharedPtr& r) : pRep(r.pRep), pUseCount(r.pUseCount) { 
			// Handle zero pointer gracefully to manage STL containers
			if(pUseCount)
			{
				++(*pUseCount); 
			}
		}
		SharedPtr& operator=(const SharedPtr& r) {
			if (pRep == r.pRep)
				return *this;
			if (pUseCount)
			{
				if (--(*pUseCount) == 0) {
					destroy();
				}
			}
			pRep = r.pRep;
			pUseCount = r.pUseCount;
			if (pUseCount)
			{
				++(*pUseCount);
			}
			return *this;
		}
		virtual ~SharedPtr() {
			if(pUseCount)
			{
				if (--(*pUseCount) == 0) {
					destroy();
				}
			}
		}

        virtual void destroy(void)
        {
			delete pRep;
			delete pUseCount;
        }


		inline T& operator*() const { assert(pRep); return *pRep; }
		inline T* operator->() const { assert(pRep); return pRep; }
		inline T* get() const { assert(pRep); return pRep; }

		/** Binds rep to the SharedPtr.
			@remarks
				Assumes that the SharedPtr is uninitialised!
		*/
		void bind(T* rep) {
			assert(!pRep && !pUseCount);
			pUseCount = new unsigned int(1);
			pRep = rep;
		}

		inline bool unique() const { assert(pUseCount); return *pUseCount == 1; }
		inline unsigned int useCount() const { assert(pUseCount); return *pUseCount; }

		inline T* getPointer() { assert(pRep); return pRep; }

		inline bool isNull(void) { return pRep == 0; }
	};

	template<class T, class U> inline bool operator==(SharedPtr<T> const& a, SharedPtr<U> const& b)
	{
		return a.get() == b.get();
	}

	template<class T, class U> inline bool operator!=(SharedPtr<T> const& a, SharedPtr<U> const& b)
	{
		return a.get() != b.get();
	}

}

#endif
