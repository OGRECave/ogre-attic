/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Marty Rabens

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

#include "LexiStdAfx.h"
#include "LexiIntermediateAPI.h"

//

const Ogre::String& CIntermediateCamera::getMovableType(void) const
{
	static Ogre::String sTypeName = "CIntermediateCamera";
	return sTypeName;
}

const Ogre::AxisAlignedBox& CIntermediateCamera::getBoundingBox(void) const
{
	static Ogre::AxisAlignedBox aab;
	return aab;
}

Ogre::Real CIntermediateCamera::getBoundingRadius(void) const
{
	return 0.0f;
}

void CIntermediateCamera::_updateRenderQueue(Ogre::RenderQueue* queue)
{
}

///////////////////////////////////////////////////////////

CIntermediateCamera::CIntermediateCamera(unsigned int iNodeID) : MovableObject(GetNodeFromID(iNodeID)->GetName()), CIntermediateObject(iNodeID)
{
	REGISTER_MODULE("Intermediate Camera")

	m_sObjectClass = "CIntermediateCamera";
}

CIntermediateCamera::~CIntermediateCamera()
{
	UNREGISTER_MODULE
}

