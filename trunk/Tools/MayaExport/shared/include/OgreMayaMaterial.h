/*
===============================================================================
This source file is part of the Ogre-Maya Tools.
Distributed as part of Ogre (Object-oriented Graphics Rendering Engine).
Copyright © 2003 Fifty1 Software Inc. 

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free 
Software Foundation; either version 2 of the License, or (at your option) any 
later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more 
details.

You should have received a copy of the GNU Lesser General Public License along 
with this program; if not, write to the Free Software Foundation, Inc., 
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
===============================================================================
*/
#ifndef _OGREMAYA_MAT_H_
#define _OGREMAYA_MAT_H_

#include <maya/MFnMesh.h>


namespace OgreMaya {

	//	===========================================================================
	/** \struct		MatOptions
		Information required by MatGenerator to convert the materials in a Maya 
		scene into an Ogre .material file.
	*/	
	//	===========================================================================
	struct MatOptions
	{
		bool bValid;
		string sOgreFile;
		string sMaterialPrefix;

		MatOptions() {
			bValid         = false;
			sOgreFile      = "";
		}
	};


	//	===========================================================================
	/** \class		MatGenerator
		\author		John Van Vliet, Fifty1 Software Inc.
		\version	1.0
		\date		June 2003

		Generates an Ogre material file from a Maya scene.
	*/	
	//	===========================================================================
	class MatGenerator : public OptionParser
	{
	public:

		/// Utility function for other classes
		/// \return		Name of the material attached to the given mesh
		static MString getMaterialName(MFnMesh &fnMesh);

		/// Standard constructor.
		MatGenerator();
		
		/// Destructor.
		virtual ~MatGenerator();

		/// Export the complete Maya scene (called by OgreMaya.mll or OgreMaya.exe).
		bool exportAll();

		/// Export selected parts of the Maya scene (called by OgreMaya.mll).
		bool exportSelection();

	protected:
		MatOptions	     mMatOptions;
		MaterialManager *mpOgreMatMgr;
		std::vector<Material*> mOgreMatPtrs;

		/// Required for OptionParser interface.
		bool _validateOptions();

		bool _extractMaterials();
		void _makeMaterials(MObject &ShaderSet);
		Material *_makePhongMaterial(MObject &ShaderNode);
		Material *_makeBlinnMaterial(MObject &ShaderNode);
		Material *_makeLambertMaterial(MObject &ShaderNode);

	};

} // namespace OgreMaya

#endif
