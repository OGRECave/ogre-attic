/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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


#include "Ogre.h"
#include "OgreXMLMeshSerializer.h"
#include "OgreMeshSerializer.h"
#include "OgreXMLSkeletonSerializer.h"
#include "OgreSkeletonSerializer.h"
#include "OgreDataChunk.h"
#include "OgreXMLPrerequisites.h"
#include <iostream>
#include <sys/stat.h>

using namespace std;

void help(void)
{
    // Print help message
    cout << endl << "OgreXMLConvert: Converts data between XML and OGRE binary formats." << endl;
    cout << "Provided for OGRE by Steve Streeting 2002" << endl << endl;
    cout << "Usage: OgreXMLConvert sourcefile [destfile] " << endl;
    cout << "sourcefile = name of file to convert" << endl;
    cout << "destfile   = optional name of file to write to. If you don't" << endl;
    cout << "             specify this OGRE works it out through the extension " << endl;
    cout << "             and the XML contents if the source is XML. For example" << endl;
    cout << "             test.mesh becomes test.xml, test.xml becomes test.mesh " << endl;
    cout << "             if the XML document root is <mesh> etc."  << endl;

    cout << endl;
}


using namespace Ogre;

// Crappy globals
// NB some of these are not directly used, but are required to
//   instantiate the singletons used in the dlls
LogManager* logMgr;
Math* mth;
MaterialManager* matMgr;
SkeletonManager* skelMgr;
MeshSerializer* meshSerializer;
XMLMeshSerializer* xmlMeshSerializer;
SkeletonSerializer* skeletonSerializer;
XMLSkeletonSerializer* xmlSkeletonSerializer;


void meshToXML(String source, String dest)
{
    struct stat tagStat;

    SDDataChunk chunk;
    stat( source, &tagStat );
    chunk.allocate( tagStat.st_size );
    FILE* pFile = fopen( source.c_str(), "rb" );
    fread( (void*)chunk.getPtr(), tagStat.st_size, 1, pFile );
    fclose( pFile );

    Mesh mesh("conversion");

    meshSerializer->importMesh(chunk, &mesh);
   
    xmlMeshSerializer->exportMesh(&mesh, dest, true);

}

void XMLToBinary(String source)
{
    // TODO
    // Read root element and decide from there what type
    TiXmlDocument* doc = new TiXmlDocument(source);
    // Some double-parsing here but never mind
    doc->LoadFile();
    TiXmlElement* root = doc->RootElement();
    // Chop off the '.xml'
    String dest = source.substr(0, source.size() - 4);
    if (!stricmp(root->Value(), "mesh"))
    {
        delete doc;
        Mesh newMesh("conversion");
        xmlMeshSerializer->importMesh(source, &newMesh);
        meshSerializer->exportMesh(&newMesh, dest, true);
    }
    else if (!stricmp(root->Value(), "skeleton"))
    {
        delete doc;
        Skeleton newSkel("conversion");
        xmlSkeletonSerializer->importSkeleton(source, &newSkel);
        skeletonSerializer->exportSkeleton(&newSkel, dest);
    }


}

void skeletonToXML(String source, String dest)
{
    struct stat tagStat;

    SDDataChunk chunk;
    stat( source, &tagStat );
    chunk.allocate( tagStat.st_size );
    FILE* pFile = fopen( source.c_str(), "rb" );
    fread( (void*)chunk.getPtr(), tagStat.st_size, 1, pFile );
    fclose( pFile );

    Skeleton skel("conversion");

    skeletonSerializer->importSkeleton(chunk, &skel);
   
    xmlSkeletonSerializer->exportSkeleton(&skel, dest);

}

int main(int numargs, char** args)
{
    if (numargs < 2)
    {
        help();
        return -1;
    }

    logMgr = new LogManager();
    mth = new Math();
    matMgr = new MaterialManager();;
    skelMgr = new SkeletonManager();
    meshSerializer = new MeshSerializer();
    xmlMeshSerializer = new XMLMeshSerializer();
    skeletonSerializer = new SkeletonSerializer();
    xmlSkeletonSerializer = new XMLSkeletonSerializer();



    String source(args[1]);

    logMgr->createLog("XMLConverter.log");

    std::vector<String> parts = source.split(".");
    String& ext = parts.back();

    ext.toLowerCase();
    if (ext == "mesh")
    {
        meshToXML(source, source + ".xml");
    }
    else if (ext == "skeleton")
    {
        skeletonToXML(source, source + ".xml");
    }
    else if (ext == "xml")
    {
        XMLToBinary(source);
    }

    

    delete xmlSkeletonSerializer;
    delete skeletonSerializer;
    delete xmlMeshSerializer;
    delete meshSerializer;
    delete skelMgr;
    delete matMgr;
    delete mth;
    delete logMgr;

    return 0;

}

