/*
===============================================================================
This source file is part of the Ogre-Maya Utilities.
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
#ifndef _OGREMAYA_MESH_MLL_H_
#define _OGREMAYA_MESH_MLL_H_

#include "OgreMayaCommon.h"

#include <maya/MPxFileTranslator.h>
class MDagPath;
class MFnDagNode;

#define	OgreMayaMeshPluginName		"OgreMesh"
#define	OgreMayaMeshPluginFileExt	"mesh"

namespace OgreMaya
{

	//	===========================================================================
	/** \class		MeshPlugin
		\author		John Van Vliet, Fifty1 Software Inc.
		\version	1.0
		\date		June 2003

		Implements the required methods for a Maya plug-in.
		Based on polyExporter from the Maya 4.5 devkit.
	*/	
	//	===========================================================================
	class MeshPlugin : public MPxFileTranslator {

	public:

		//  =====
		//  ===== Constructors and destructors
		//  =====
		
		/// Standard constructor (does nothing)
		MeshPlugin()          {};
		
		/// Destructor (does nothing)
		virtual ~MeshPlugin() {};

		//  =====
		//  ===== Maya plug-in requirements
		//  =====
		
		/// Indicate to Maya that this plugin implements writer().
		bool         haveWriteMethod() const  {return true;}

		/// Indicate to Maya that this plugin does not implement reader().
		bool         haveReadMethod() const   {return false;}

		/// Indicate to Maya that the translator can open and import files
		/// (false if it can only import files).
		bool         canBeOpened() const      {return true;}

		/// Returns the export file type extension (called by Maya)
		MString      defaultExtension() const {return OgreMayaMeshPluginFileExt;};

		/// Required by Maya to allocate an instance of the derived class
		static void* creator();
		
		/// Required to register the plugin with Maya
		MStatus      initializePlugin(MObject obj);
		
		/// Required to unregister the plugin with Maya
		MStatus      uninitializePlugin(MObject obj);

		/// Write a .mesh file containing all or selected meshes
		MStatus      writer(const MFileObject& file,
							const MString& optionsString,
							MPxFileTranslator::FileAccessMode mode);
	};

} // namespace OgreMaya

#endif // #ifndef _OGREMAYA_MESH_MLL_H_
