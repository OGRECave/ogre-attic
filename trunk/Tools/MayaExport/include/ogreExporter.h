
#ifndef OGRE_EXPORTER_H
#define OGRE_EXPORTER_H

#include <stdio.h>
#include <vector>
#include <math.h>
#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MItDag.h>
#include <maya/MFnCamera.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MFnTransform.h>
#include <maya/MAngle.h>
#include "transferMesh.h"


class OgreExporter : public MPxCommand
{
public:
	// public methods
	OgreExporter();
	virtual ~OgreExporter(){};
	static void* creator();
	MStatus doIt(const MArgList& args);
	bool isUndoable() const;
	MStatus translateNode(MDagPath& dagPath);
	MStatus writeAnim(MFnAnimCurve& anim);
	MStatus writeCamera(MFnCamera& camera);

private:
	// private members
	MStatus stat;
	ParamList params;
	TransferSkeleton skeleton;
};




/*********************************************************************************************
 *                                  INLINE Functions                                         *
 *********************************************************************************************/
// Standard constructor
inline OgreExporter::OgreExporter()
{
	MGlobal::displayInfo("Translating scene to OGRE format");
}

// Routine for creating the plug-in
inline void* OgreExporter::creator()
{
	return new OgreExporter();
}

// It tells that this command is not undoable
inline bool OgreExporter::isUndoable() const
{
	MGlobal::displayInfo("Command is not undoable");
	return false;
}

// Routine for registering the command within Maya
MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, "OgreExporter", "6.0", "Any");
	status = plugin.registerCommand( "ogreExport", OgreExporter::creator );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}

// Routine for unregistering the command within Maya
MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );
	status = plugin.deregisterCommand( "ogreExport" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}

	return status;
}
#endif