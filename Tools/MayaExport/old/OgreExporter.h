










#pragma once

#define _BOOL

#include <string>

#include <maya/MPxFileTranslator.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>

class MFnPlugin;

class MayaMesh
{
public:
	MayaMesh( const MDagPath &meshDagPath, const MDagPath &meshParentPath, const MObject &meshObject );

	MDagPath path( void ) const
	{
		return m_meshDagPath;
	}

	MObject object( void ) const
	{
		return m_meshObject;
	}

private:
	MDagPath m_meshDagPath;
	MDagPath m_meshParentPath;

	MObject  m_meshObject;
};

struct MayaBone
{
public:
	MayaBone( const MDagPath &boneDagPath, const MDagPath &boneParentPath, const MObject &meshObject );

private:
	MDagPath m_boneDagPath;
	MDagPath m_boneParentPath;

	MObject  m_boneObject;

	int      m_references;
};


class OgreExporter : public MPxFileTranslator
{
public:
	static bool RegisterPlugin  ( MFnPlugin &plugin );
	static bool DeregisterPlugin( MFnPlugin &plugin );

public:
	OgreExporter();

public:
	virtual MStatus writer( const MFileObject &file,
	                        const MString     &optionsString,
	                        FileAccessMode     mode);
	static void* creator();
	
	virtual bool    haveWriteMethod () const;
	virtual bool    haveReadMethod  () const;
	virtual MString defaultExtension() const;

	virtual MPxFileTranslator::MFileKind identifyFile( const MFileObject &file,
	                                                   const char * buffer,
	                                                   short              size ) const;

private:
	std::string m_filename;

};
