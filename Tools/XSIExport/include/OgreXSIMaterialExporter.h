/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#ifndef __XSIMATERIALEXPORTER_H__
#define __XSIMATERIALEXPORTER_H__

#include "OgreXSIHelper.h"
#include "OgreBlendMode.h"
#include "OgreMaterialSerializer.h"

namespace Ogre {

	class XsiMaterialExporter
	{
	public:
		XsiMaterialExporter();
		virtual ~XsiMaterialExporter();

		/** Export a set of XSI materials to a material script.
		@param materials List of materials to export
		@param filename Name of the script file to create
		@param copyTextures Whether to copy any textures used into the same
			folder as the material script.
		*/
		void exportMaterials(MaterialMap& materials, const String& filename, 
			bool copyTextures);
	protected:	
		MaterialSerializer mMatSerializer;

		void exportMaterial(MaterialEntry* matEntry, bool copyTextures);

		void populatePass(Pass* pass, XSI::Shader& xsishader, 
			bool copyTextures);

		SceneBlendFactor convertSceneBlend(short xsiVal);
		
	};
}

#endif


