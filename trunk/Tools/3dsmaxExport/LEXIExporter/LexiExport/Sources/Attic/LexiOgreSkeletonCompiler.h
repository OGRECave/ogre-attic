/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
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

#include "LexiIntermediateAPI.h"


class COgreSkeletonCompiler
{


public:
	COgreSkeletonCompiler( Ogre::String name, Ogre::MeshPtr ogreMesh );
	virtual ~COgreSkeletonCompiler();
	bool WriteOgreSkeleton( const Ogre::String& sFilename, bool bXMLexport=false );
	void CreateAnimations( void );

protected:

private:

	void CreateSkeleton( CIntermediateBone* pIBone = NULL );

	CIntermediateSkeleton* m_pISkel;
	Ogre::SkeletonPtr m_pSkel;
	Ogre::MeshPtr m_pOgreMesh;

};