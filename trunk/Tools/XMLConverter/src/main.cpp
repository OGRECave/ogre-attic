/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/


#include "Ogre.h"
#include "OgreXMLMeshSerializer.h"
#include "OgreMeshSerializer.h"
#include "OgreDataChunk.h"
#include <iostream.h>
#include <sys/stat.h>

void help(void)
{
    // Print help message
    cout << endl << "OgreXMLConvert: Converts data between XML and OGRE binary formats." << endl;
    cout << "Provided for OGRE by Steve Streeting 2002" << endl << endl;
    cout << "Usage: OgreXMLConvert sourcefile [destfile] " << endl;
    cout << "sourcefile = name of file to convert" << endl;
    cout << "destfile   = optional name of file to write to. If you don't" << endl;
    cout << "            specify this OGRE works it out through the extension " << endl;
    cout << "             and the XML contents if the source is XML. For example" << endl;
    cout << "             test.mesh becomes test.xml, test.xml becomes test.mesh " << endl;
    cout << "             if the XML document root is <mesh> etc."  << endl;

    cout << endl;
}


using namespace Ogre;

// Dummy chunk


int main(int numargs, char** args)
{
    if (numargs < 2)
    {
        help();
        return -1;
    }

    char* source = args[1];



    LogManager logMgr;
    MaterialManager matMgr;
    MeshSerializer meshSerializer;
    XMLMeshSerializer xmlMeshSerializer;
    ArchiveManager archMgr;


    logMgr.createLog("XMLConverter.log");

    struct stat tagStat;

    DataChunk chunk;
    stat( source, &tagStat );
    chunk.allocate( tagStat.st_size );
    FILE* pFile = fopen( source, "rb" );
    fread( (void*)chunk.getPtr(), tagStat.st_size, 1, pFile );
    fclose( pFile );

    Mesh mesh("test");

    meshSerializer.importMesh(chunk, &mesh);

   
    xmlMeshSerializer.exportMesh(&mesh, "ogrehead.xml", true);

    

    return 0;

}