
#include "OgreExporter.h"

#include "Ogre.h"

//#include <crtdbg.h>

#include <zlib.h>

/*
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MFnDependencyNode.h>
#include <maya/M3dView.h>
#include <maya/MItDependencyGraph.h>
*/

#include <maya/MFnMesh.h>
#include <maya/MItDag.h>
#include <maya/MFnPlugin.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <maya/MMatrix.h>

#include <vector>
#include <functional>
#include <algorithm>
#include "Windows.h"

const char* const c_pPlugName = "OgreGeometry";


MayaMesh::MayaMesh( const MDagPath &meshDagPath, const MDagPath &meshParentPath, const MObject &meshObject ) :
m_meshDagPath   ( meshDagPath    ),
m_meshParentPath( meshParentPath ),
m_meshObject    ( meshObject     )
{
	
}


MayaBone::MayaBone( const MDagPath &boneDagPath, const MDagPath &boneParentPath, const MObject &boneObject ) :
m_boneDagPath   ( boneDagPath    ),
m_boneParentPath( boneParentPath ),
m_boneObject    ( boneObject     ),
m_references    ( 0              )
{
	
}






OgreExporter::OgreExporter()
{

}

void* OgreExporter::creator()
{
	return new OgreExporter;
}

MDagPath GetParentDagPath(const MDagPath& path)
{
	MDagPath pathToParent(path);
	while (pathToParent.pop() == MS::kSuccess)
	{
		if (pathToParent.node().hasFn(MFn::kTransform))
			return pathToParent;
	}

	return pathToParent;
}



void collectStuff( std::vector< MayaMesh > * const pMeshes, std::vector< MayaBone > * const pBones )
{
	assert(pMeshes);
	assert(pBones);

	assert( pMeshes->size() == 0 );

	//pMeshes->resize(0);
	
	//pBones->resize(0);

	MItDag dag_itr( MItDag::kDepthFirst, MFn::kInvalid );
	while( dag_itr.isDone() == false )
	{
		MDagPath dagPath;
		dag_itr.getPath( dagPath );

		const MFnDagNode dagNode( dagPath );
		if (dagNode.isIntermediateObject()) 
		{
			dag_itr.next();
			continue;
		}

		if ( dagPath.hasFn(MFn::kMesh) &&
		     !dagPath.hasFn(MFn::kTransform) )
		{
			pMeshes->push_back( MayaMesh( dagPath, GetParentDagPath( dagPath ), dag_itr.item() ) );

		}
		else if (dag_itr.item().apiType() == MFn::kJoint ||
				 dag_itr.item().apiType() == MFn::kTransform)
		{
			MDagPath boneDagPath;

			dag_itr.getPath( boneDagPath );

			pBones->push_back( MayaBone( boneDagPath, GetParentDagPath( boneDagPath ), dag_itr.item() ) );
		}

		dag_itr.next();
	}
}

class IndirectVert
{
public:

	IndirectVert( void ) :
	m_newVertIndex( -1 ),
	m_posIndex    ( -1 ),
	m_normalIndex ( -1 ),
	m_uv0Index    ( -1 ),
	m_colorIndex  ( -1 )
	{
        calcHash();
	}


	IndirectVert( const int newVertIndex, const int posIndex, const int normalIndex, const int uv0Index, const int colorIndex ) :
	m_newVertIndex( newVertIndex ),
	m_posIndex    ( posIndex     ),
	m_normalIndex ( normalIndex  ),
	m_uv0Index    ( uv0Index     ),
	m_colorIndex  ( colorIndex   )
	{
        calcHash();
	}

    void calcHash(void)
    {
        unsigned char* buf = new unsigned char[sizeof(int) * 4];
        unsigned char* pBuf = buf;
        memcpy(pBuf, &m_posIndex, sizeof(int));
        pBuf += sizeof(int);
        memcpy(pBuf, &m_normalIndex, sizeof(int));
        pBuf += sizeof(int);
        memcpy(pBuf, &m_uv0Index, sizeof(int));
        pBuf += sizeof(int);
        memcpy(pBuf, &m_colorIndex, sizeof(int));

        // Use zlib crc32
        unsigned long crc = crc32(0L, Z_NULL, 0);
        m_hash = crc32(crc, buf, sizeof(int) * 4);
    }

	const unsigned long hash( void ) const
	{
		return m_hash;
	}

	bool operator ==( const IndirectVert &other ) const
	{
		return m_posIndex    == other.m_posIndex    && 
		       m_normalIndex == other.m_normalIndex &&
			   m_uv0Index    == other.m_uv0Index    &&
			   m_colorIndex  == other.m_colorIndex;
	}

	bool operator !=( const IndirectVert &other ) const
	{
		return !(m_hash == other.m_hash);
	}

	bool operator <( const IndirectVert &other ) const
	{
		return m_hash < other.m_hash;
	}

	//Ease of access.  Privitise later.
		//Does not participate in hashing
		int m_newVertIndex; 

		//Participate in the hashing.
		int m_posIndex;
		int m_normalIndex;
		int m_uv0Index;
		int m_colorIndex;

private:

	unsigned long m_hash;

};

typedef std::hash_set< IndirectVert > IndVertHS;

struct std::hash<IndirectVert>
{
	size_t operator ()( const IndirectVert &vert ) const
	{
		return vert.hash();
	}
};

/*
void uniqueifyVerts
( 
	std::vector< IndirectVert > * const vertList, 
	const std::vector< int >           &posIndices,
	const std::vector< int >           &normalIndices,
	const std::vector< int >           &uv0Indices,
	const std::vector< int >           &colorIndices
)
{
	assert( posIndices.size() == normalIndices.size() == uv0Indices.size() == colorIndices.size() );

	std::hash_set< IndirectVert > m_vertHash;

	const int numVerts = posIndices.size();

	for( int i=0; i<numVerts; ++i )
	{
		const IndirectVert vert( posIndices[ i ], normalIndices[ i ], uv0Indices[ i ], colorIndices[ i ] );

		if( m_vertHash.find( vert )
	}	
}
*/


MStatus exportMesh( const MayaMesh &mesh, Ogre::Mesh * const ogreMesh )
{
	MStatus stat = MS::kSuccess;
	const MSpace::Space space = MSpace::kWorld;

	MFnMesh fnMesh( mesh.path(), &stat );
	if ( MS::kSuccess != stat) 
	{
		fprintf(stderr,"Failure in MFnMesh initialization.\n");
		return MS::kFailure;
	}

	const MMatrix matrix = mesh.path().inclusiveMatrix( &stat );

	const float det44 = matrix.det4x4();

	bool reverseWinding = false;

	if( det44 < 0.0f )
	{
		reverseWinding = true;
	}

	MItMeshVertex vtxIter( mesh.object(), &stat );
	if ( MS::kSuccess != stat) 
	{
		fprintf(stderr,"Failure in MItMeshVertex initialization.\n");
		return MS::kFailure;
	}

	Ogre::SubMesh * const ogreSubMesh = ogreMesh->createSubMesh();

	//Default ugly texture for the moment.
	ogreSubMesh->setMaterialName( "Examples/Test" );

	// Set up mesh geometry
	// Always 1 texture layer, 2D coords
	ogreSubMesh->geometry.numTexCoords             = 1;
	ogreSubMesh->geometry.numTexCoordDimensions[0] = 2;
	ogreSubMesh->geometry.hasNormals               = true;
	ogreSubMesh->geometry.hasColours               = false;
	ogreSubMesh->geometry.vertexStride             = 0;
	ogreSubMesh->geometry.texCoordStride[0]        = 0;
	ogreSubMesh->geometry.normalStride             = 0;
	ogreSubMesh->useSharedVertices                 = false;
	ogreSubMesh->useTriStrips                      = false;

	/*
    // Fill in vertex table
	std::vector< MPoint > points;

	for ( ; !vtxIter.isDone(); vtxIter.next() ) 
	{
		const MPoint pt      = vtxIter.position( space, &stat );
		const MPoint ptWorld = vtxIter.position( MSpace::kWorld , &stat );
		const MPoint ptLocal = vtxIter.position( MSpace::kObject, &stat );

		points.push_back( pt );
	}
	*/

	MFloatPointArray points;
	fnMesh.getPoints( points, space );



	MFloatVectorArray norms;
	fnMesh.getNormals( norms, space );
    //const int normsLength = norms.length();

    // Write out the uv table
	MFloatArray uArray, vArray;
	fnMesh.getUVs( uArray, vArray );
    //const int uvLength = uArray.length();

	assert( uArray.length() == vArray.length() );

	ogreSubMesh->numFaces = fnMesh.numPolygons( &stat );

	MItMeshPolygon polyIter( mesh.object(), &stat );
	if ( MS::kSuccess != stat) 
	{
		fprintf( stderr, "Failure in MItMeshPolygon initialization.\n" );
		return stat;
	}

	std::vector< int > faceIndices;

	std::vector< IndirectVert > vertList;

	std::hash_set< IndirectVert > vertHash;


	int curVert = 0;

	for ( ; !polyIter.isDone(); polyIter.next() ) 
	{
		const int vertCount = polyIter.polygonVertexCount( &stat );

		int start = 0;
		int end   = 3;
		int delta = 1;

		if( reverseWinding )
		{
			start = 2;
			end   = -1;
			delta = -1;
		}

		for( int i = start; i != end; i+=delta )
		{
			const int posIndex = polyIter.vertexIndex( i, &stat );

			const int normalIndex = polyIter.normalIndex( i, &stat );

				  int uv0Index;

			polyIter.getUVIndex( i, *&uv0Index );

			const IndirectVert potentialNewVert( vertList.size(), posIndex, normalIndex, uv0Index, -1 );

			const IndVertHS::iterator it = vertHash.find( potentialNewVert );

			if( it != vertHash.end() )
			{
				const IndirectVert vert = *it;

				faceIndices.push_back( vert.m_newVertIndex );
			}
			else
			{
				vertHash.insert( potentialNewVert );
				vertList.push_back( potentialNewVert );
				faceIndices.push_back( potentialNewVert.m_newVertIndex );
			}
		}
	}

	ogreSubMesh->geometry.numVertices   = vertList.size();
	ogreSubMesh->geometry.pVertices     = new Ogre::Real[ogreSubMesh->geometry.numVertices * 3];

	ogreSubMesh->geometry.pNormals      = new Ogre::Real[ogreSubMesh->geometry.numVertices * 3];
	ogreSubMesh->geometry.pTexCoords[0] = new Ogre::Real[ogreSubMesh->geometry.numVertices * 2];

	for( int i=0; i<vertList.size(); ++i )
	{
		const IndirectVert &vert = vertList[i];

		{
			const int posIndex = vert.m_posIndex;

			assert( posIndex >= 0 );
			assert( posIndex <  points.length() );

			const MFloatPoint &pt = points[ posIndex ];

			const int posBaseIndex = i * 3;

			ogreSubMesh->geometry.pVertices[ posBaseIndex + 0 ] = pt.x;
			ogreSubMesh->geometry.pVertices[ posBaseIndex + 1 ] = pt.y;
			ogreSubMesh->geometry.pVertices[ posBaseIndex + 2 ] = pt.z;
		}

		{
			const int normalIndex = vert.m_normalIndex;

			assert( normalIndex >= 0 );
			assert( normalIndex <  norms.length() );

		    const MFloatVector &normal = norms[normalIndex];

			const int normBaseIndex = i * 3;

			ogreSubMesh->geometry.pNormals[ normBaseIndex + 0 ] = normal.x;
			ogreSubMesh->geometry.pNormals[ normBaseIndex + 1 ] = normal.y;
			ogreSubMesh->geometry.pNormals[ normBaseIndex + 2 ] = normal.z;
		}

		{
			const int uv0Index = vert.m_uv0Index;

			assert( uv0Index >= 0 );
			assert( uv0Index <  uArray.length() );

			const int uv0BaseIndex = i * 2;

			ogreSubMesh->geometry.pTexCoords[0][ uv0BaseIndex + 0 ] = uArray[ uv0Index ];
			ogreSubMesh->geometry.pTexCoords[0][ uv0BaseIndex + 1 ] = vArray[ uv0Index ];
		}

	}

 	ogreSubMesh->faceVertexIndices = new unsigned short[ faceIndices.size() ];

	for(     i=0; i<faceIndices.size(); ++i )
	{
		ogreSubMesh->faceVertexIndices[ i ] = faceIndices[ i ];
	}

	/*
    if (pSkel)
        delete pSkel;
	*/


	return stat;
}

	/* Old triagulation.
	for ( ; !polyIter.isDone(); polyIter.next() ) 
	{
		const int vertCount = polyIter.polygonVertexCount( &stat );

		if( vertCount != 3 )
		{
			int dummy = 0;
		}

//MStatus MItMeshPolygon:: getTriangles ( MPointArray & points, MIntArray & vertexList, MSpace::Space space ) const 

		MPointArray ptArray;
		MIntArray   vertIndices;

		polyIter.getTriangles( *&ptArray, *&vertIndices, space );

		assert( vertIndices.length() % 3 == 0 );

		for( int baseVertIndex=0; baseVertIndex<vertIndices.length(); baseVertIndex+=3 )
		{
			for( int offset = 0; offset < 3; ++offset )
			{
				const int vertIndex = vertIndices[ baseVertIndex + offset ];

				int polyLocalIndex = -1;

				for( int i=0; i<vertCount; ++i )
				{
					if( polyIter.vertexIndex( i, &stat ) == vertIndex )
					{
						polyLocalIndex = 0;
						break;
					}
				}

				assert( polyLocalIndex != -1 );

				const int posIndex = vertIndex; //polyIter.vertexIndex( vertIndex, &stat );

				const int normalIndex = polyIter.normalIndex( polyLocalIndex, &stat );

					  int uv0Index;

				polyIter.getUVIndex( polyLocalIndex, *&uv0Index );

				const IndirectVert potentialNewVert( vertList.size(), posIndex, normalIndex, uv0Index, -1 );

				const IndVertHS::iterator it = vertHash.find( potentialNewVert );

				if( it != vertHash.end() )
				{
					const IndirectVert vert = *it;

					faceIndices.push_back( vert.m_newVertIndex );
				}
				else
				{
					vertHash.insert( potentialNewVert );
					vertList.push_back( potentialNewVert );
					faceIndices.push_back( potentialNewVert.m_newVertIndex );
				}
			}
		}
	}
	*/


/*
MStatus exportMesh( const MayaMesh &mesh, Ogre::Mesh * const ogreMesh )
{
	MStatus stat = MS::kSuccess;
	const MSpace::Space space = MSpace::kWorld;

	MFnMesh fnMesh( mesh.path(), &stat );
	if ( MS::kSuccess != stat) 
	{
		fprintf(stderr,"Failure in MFnMesh initialization.\n");
		return MS::kFailure;
	}

	MItMeshVertex vtxIter( mesh.object(), &stat );
	if ( MS::kSuccess != stat) 
	{
		fprintf(stderr,"Failure in MItMeshVertex initialization.\n");
		return MS::kFailure;
	}

	Ogre::SubMesh * const ogreSubMesh = ogreMesh->createSubMesh();

	//Default ugly texture for the moment.
	ogreSubMesh->setMaterialName("BaseWhite");

	/*
	int matIdx = msMesh_GetMaterialIndex(pMesh);

	if (matIdx == -1)
	{
		// No material, use blank
		ogreSubMesh->setMaterialName("BaseWhite");
		logMgr.logMessage("No Material, using default 'BaseWhite'.");
	}
	else
	{

		msMaterial *pMat = msModel_GetMaterialAt(pModel, matIdx);
		ogreSubMesh->setMaterialName(pMat->szName);
		logMgr.logMessage("SubMesh Material Done.");
	}
	* /

	// Set up mesh geometry
	// Always 1 texture layer, 2D coords
	ogreSubMesh->geometry.numTexCoords             = 1;
	ogreSubMesh->geometry.numTexCoordDimensions[0] = 2;
	ogreSubMesh->geometry.hasNormals               = true;
	ogreSubMesh->geometry.hasColours               = false;
	ogreSubMesh->geometry.vertexStride             = 0;
	ogreSubMesh->geometry.texCoordStride[0]        = 0;
	ogreSubMesh->geometry.normalStride             = 0;
	ogreSubMesh->useSharedVertices                 = false;
	ogreSubMesh->useTriStrips                      = false;


    // Fill in vertex table
	std::vector< MPoint > points;

	for ( ; !vtxIter.isDone(); vtxIter.next() ) 
	{

		points.push_back( vtxIter.position( space ) );

		/* Need to look this up to see what it is.
		if (ptgroups && groups) 
		{
			int compIdx = vtxIter.index();
		    outputSets( mdagPath, compIdx, true );
		}
		fprintf(fp,"v %f %f %f\n",p.x,p.y,p.z);
		v++;
		* /
	}

	ogreSubMesh->geometry.numVertices = points.size();
	ogreSubMesh->geometry.pVertices = new Ogre::Real[ogreSubMesh->geometry.numVertices * 3];

	ogreSubMesh->geometry.pNormals      = new Ogre::Real[ogreSubMesh->geometry.numVertices * 3];
	ogreSubMesh->geometry.pTexCoords[0] = new Ogre::Real[ogreSubMesh->geometry.numVertices * 2];

	/*
	for( unsigned i=0; i<points.size(); ++i )
	{
		const int index = i * 3;
		ogreSubMesh->geometry.pVertices[ index + 0 ] = points[i].x;
		ogreSubMesh->geometry.pVertices[ index + 1 ] = points[i].y;
		ogreSubMesh->geometry.pVertices[ index + 2 ] = points[i].z;

	}
	* /


    // Write out the normal table
	MFloatVectorArray norms;
	fnMesh.getNormals( norms, space );
    const int normsLength = norms.length();

	assert( normsLength >= points.size() );

	/*
	for ( i=0; i<points.size(); ++i ) 
	{
	    MFloatVector tmpf = norms[i];

		const int vertIdx = i * 3;

        ogreSubMesh->geometry.pNormals[ vertIdx + 0 ] = tmpf.x;
        ogreSubMesh->geometry.pNormals[ vertIdx + 1 ] = tmpf.y;
        ogreSubMesh->geometry.pNormals[ vertIdx + 2 ] = tmpf.z;
	}
	* /

    // Write out the uv table
	MFloatArray uArray, vArray;
	fnMesh.getUVs( uArray, vArray );
    const int uvLength = uArray.length();

	assert( uvLength >= points.size() );

	/*
	for ( i=0; i<points.size(); i++ ) 
	{
		const int index = i * 2;

		//fprintf(fp,"vt %f %f\n",uArray[x],vArray[x]);
		ogreSubMesh->geometry.pTexCoords[0][ index + 0 ] = uArray[i];
		ogreSubMesh->geometry.pTexCoords[0][ index + 1 ] = vArray[i];
	}
	* /


	ogreSubMesh->numFaces = fnMesh.numPolygons( &stat );

	MItMeshPolygon polyIter( mesh.object(), &stat );
	if ( MS::kSuccess != stat) 
	{
		fprintf(stderr,"Failure in MItMeshPolygon initialization.\n");
		return MS::kFailure;
	}

	std::vector< int > faceIndices;

	std::vector< IndirectVert > vertList;

	std::hash_set< IndirectVert > m_vertHash;

	int curVert = 0;

	for ( ; !polyIter.isDone(); polyIter.next() ) 
	{
		for( int vert = 0; vert < 3; ++vert )
		{
			//faceIndices.push_back( polyIter.vertexIndex( vert, &stat ) );

			const int posIndex = polyIter.vertexIndex( vert, &stat )

			const IndirectVert vert( posIndex, normalIndices[ i ], uv0Indices[ i ], colorIndices[ i ] );


		}
	}

 	ogreSubMesh->faceVertexIndices = new unsigned short[ faceIndices.size() ];

	for( i=0; i<faceIndices.size(); ++i )
	{
		ogreSubMesh->faceVertexIndices[ i ] = faceIndices[ i ];
	}

	/*
    if (pSkel)
        delete pSkel;
	* /


	return stat;
}
*/

MStatus OgreExporter::writer( const MFileObject &file, const MString &optionsString, FileAccessMode mode )
{
	/*
	int tmpDbgFlag  = _CRTDBG_ALLOC_MEM_DF;
	tmpDbgFlag |= _CRTDBG_CHECK_ALWAYS_DF;
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpDbgFlag);
	*/


    MString mname = file.fullName();
    m_filename = mname.asChar();

    Ogre::ResourceGroupManager resGrpMagr;
    Ogre::MaterialManager matMgr;
    Ogre::LogManager      logMgr;

    logMgr.createLog("MayaOgreExport.log");

    logMgr.logMessage("OGRE Maya Exporter Log");
    logMgr.logMessage("---- ---- -------- ---");

	std::vector< MayaMesh > meshes;
	std::vector< MayaBone > bones;

	collectStuff( &meshes, &bones );

	if( meshes.size() > 0 )
	{
		//Create the Ogre meshes and submeshes.  
		//TODO: find the name of the mesh.
		Ogre::Mesh * const ogreMesh = new Ogre::Mesh( m_filename.c_str() );

		for( unsigned i=0; i<meshes.size(); ++i )
		{
			exportMesh( meshes[i], ogreMesh );
		}

		Ogre::MeshSerializer serializer;

		serializer.exportMesh( ogreMesh, m_filename.c_str(), false );

		delete ogreMesh;
	}


	return MStatus::kSuccess;
}

bool OgreExporter::haveWriteMethod() const
{
	return true;
}

bool OgreExporter::haveReadMethod() const
{
	return false;
}

MString OgreExporter::defaultExtension() const
{
	return MString("mesh");
}

MPxFileTranslator::MFileKind OgreExporter::identifyFile( const MFileObject &file, const char * buffer, short size ) const
{
	//We do not load .mesh files yet.
	return kNotMyFileType;
}


bool OgreExporter::RegisterPlugin( MFnPlugin &fnPlugin )
{
	MStatus status = fnPlugin.registerFileTranslator( c_pPlugName,
	                                                  NULL,
	                                                  OgreExporter::creator,
	                                                  NULL,
	                                                  NULL,
	                                                  false );

	if (!status)
	{
        status.perror("Registering .mesh exporter.");
		return status;
	}

	return MStatus::kSuccess;
}

bool OgreExporter::DeregisterPlugin( MFnPlugin &fnPlugin )
{
    MStatus status = fnPlugin.deregisterFileTranslator( c_pPlugName );

	if (!status)
	{
		status.perror("Deregistering .mesh exporter.");
		return false;
    }

	return true;
}

