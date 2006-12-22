/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Lasse Tassing

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

#ifndef __DD_BASE__
#define __DD_BASE__

// Dynamic Data types
enum EDDType
{
	DD_UNKNOWN=0,
	DD_OBJECT,
	DD_OBJLIST,
	DD_INT,
	DD_INTLIST,
	DD_FLOAT,
	DD_FLOATLIST,
	DD_STRING,
	DD_STRINGLIST,
	DD_BINARY,
	DD_VEC3,
	DD_VEC3LIST,
	DD_BOOL,
	DD_VEC2,
	DD_VEC4,
	DD_VEC4LIST,
	DD_MATRIX,
	DD_MATRIXLIST
};

// Dynamic Data Baseclass
class CDDBase : public CRefCount
{
public:
	virtual ~CDDBase(void) {};

	// Retrieve type of object
	virtual EDDType	GetType(void) const =0;

	// Create a clone of the object instance
	virtual CDDBase* Clone(void) const =0;
};

#endif
