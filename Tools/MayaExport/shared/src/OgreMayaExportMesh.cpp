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

#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>
#include <maya/MDagPath.h>

#include <iostream>
#include <fstream>

#include "OgreMayaExportMesh.h"

using namespace std;
namespace OgreMaya
{

	//	--------------------------------------------------------------------------
	/**	Allow Maya to create an instance of this object.
	*/	
	//	--------------------------------------------------------------------------
	void* MeshPlugin::creator() 
	{
		return new MeshPlugin();
	}




	//	--------------------------------------------------------------------------
	/**	Registers the commands, tools, devices, and so on, defined by the plug-in.
		(called by Maya).

		\param		obj
					MObject (node) for the plugin
		\return		MStatus::kSuccess if plugin initialized ok, 
					MStatus::kFailure otherwise
	*/	
	//	--------------------------------------------------------------------------
	MStatus initializePlugin(MObject obj)
	{
		MStatus status;
		MFnPlugin plugin(obj, "Fifty1 Software - Ogre Mesh Exporter", "1.0", "Any");

		// Register the translator with the system
		status =  plugin.registerFileTranslator("OgreMeshExporter",
												"",
												MeshPlugin::creator,
												"",
												"option1=1",
												false);		// true if reader() needs MEL commands
		if (!status) {
			status.perror("registerFileTranslator");
			return status;
		}

		return status;
	}



	//	--------------------------------------------------------------------------
	/**	De-registers the commands, tools, devices, and so on, defined by the plug-in.
		(called by Maya).

		\param		obj
					MObject (node) for the plugin
		\return		MStatus::kSuccess if plugin uninitialized ok, 
					MStatus::kFailure otherwise
	*/	
	//	--------------------------------------------------------------------------
	MStatus uninitializePlugin(MObject obj) 
	{
		MStatus   status;
		MFnPlugin plugin( obj );

		status =  plugin.deregisterFileTranslator("OgreMeshExporter");
		if (!status) {
			status.perror("deregisterFileTranslator");
			return status;
		}

		return status;
	}



	//	--------------------------------------------------------------------------
	/**	Saves a file of a type supported by this translator by traversing all or 
		selected objects (depending on mode) in the current Maya scene, and 
		writing a representation to the given file.

		\param		file
					Object containing the pathname of the output file
		\param		options
					String representation of options
		\param		mode
					Method used to write the file. Export and export active are
					valid (method will fail for any other values).

		\return		MStatus::kSuccess if file exported ok, 
					MStatus::kFailure otherwise
	*/	
	//	--------------------------------------------------------------------------
	MStatus MeshPlugin::writer(const MFileObject& file,
                               const MString& options,
                               MPxFileTranslator::FileAccessMode mode) 
	{
		const MString fileName = file.fullName();

		ofstream newFile(fileName.asChar(), ios::out);
		if (!newFile) {
			MGlobal::displayError(fileName + ": could not be opened for reading");
			return MS::kFailure;
		}
		newFile.setf(ios::unitbuf);

		//check which objects are to be exported, and invoke the corresponding
		//methods; only 'export all' and 'export selection' are allowed
		//
		if (MPxFileTranslator::kExportAccessMode == mode) {
	//		if (MStatus::kFailure == exportAll(newFile)) {
	//			return MStatus::kFailure;
	//		}
		} else if (MPxFileTranslator::kExportActiveAccessMode == mode) {
	//		if (MStatus::kFailure == exportSelection(newFile)) {
	//			return MStatus::kFailure;
	//		}
		} else {
			return MStatus::kFailure;
		}

		newFile.flush();
		newFile.close();

		MGlobal::displayInfo("Export to " + fileName + " successful!");
		return MS::kSuccess;
	}

} // namespace OgreMaya