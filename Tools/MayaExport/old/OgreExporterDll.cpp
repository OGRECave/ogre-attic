









#define NT_PLUGIN

#define MNoVersionString

#include <maya/MObject.h>
#include <maya/MFnPlugin.h>
//#include <maya/MGlobal.h>

#include "OgreExporter.h"


namespace
{
	const char* const csg_pAuthorString  = "(C) Copyright Marc Hernandez";
	const char* const csg_pVersionString = "0.1";

} // namespace {}

MStatus __declspec( dllexport ) initializePlugin(MObject Object)
{
    MStatus status;
    MFnPlugin Plugin( Object, csg_pAuthorString, csg_pVersionString, "Any", &status );
	if (!status)
	{
        status.perror("MFnPlugin");
		return status;
	}

	// Register the exporters
	{
		OgreExporter::RegisterPlugin(Plugin);
	}

	return status;
}

MStatus __declspec( dllexport ) uninitializePlugin( MObject Object )
{
    MStatus status;
    MFnPlugin Plugin( Object, csg_pAuthorString, csg_pVersionString, "Any", &status );
	if (!status)
	{
        status.perror("MFnPlugin");
		return status;
	}

	// Exporters
	{
		OgreExporter::DeregisterPlugin( Plugin );
	}

    return status ;
}
