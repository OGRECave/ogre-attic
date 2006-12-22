/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Bo Krohn

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

#ifndef __REFCOUNT__
#define __REFCOUNT__

// Reference count base - derive from this class to 
class CRefCount
{
public:
	CRefCount(void) { m_iRefCount=1; }
	virtual ~CRefCount(void) {}

	// Called when refcount reaches zero
	virtual void OnFinalRelease(void) { delete this; }

	// Increment reference count
//	void AddRef(void) const { ++(((CRefCount*)this)->m_iRefCount); }
	void AddRef(void) const 
	{ 
		InterlockedIncrement(&((CRefCount*)this)->m_iRefCount); 
	}

	// Relase object (decrease reference count)
//	void Release(void) const { if(--(((CRefCount*)this)->m_iRefCount)<=0) 
//											((CRefCount*)this)->OnFinalRelease(); }
	virtual void Release(void) const 
	{
		if(InterlockedDecrement(&((CRefCount*)this)->m_iRefCount)<=0)
			((CRefCount*)this)->OnFinalRelease();
	}
								

	// Retrive current reference count
	int	GetRefCount(void) const { return m_iRefCount; }

protected:
//	signed int	m_iRefCount;
	volatile long m_iRefCount;
};

#endif
