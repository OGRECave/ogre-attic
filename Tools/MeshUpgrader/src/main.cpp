/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
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
#include "OgreMeshSerializer.h"
#include "OgreSkeletonSerializer.h"
#include "OgreDataChunk.h"
#include "OgreDefaultHardwareBufferManager.h"

#include <iostream>
#include <sys/stat.h>

using namespace std;

void help(void)
{
    // Print help message
    cout << endl << "OgreMeshUpgrader: Upgrades .mesh files to the latest version." << endl;
    cout << "Provided for OGRE by Steve Streeting 2003" << endl << endl;
    cout << "Usage: OgreMeshUpgrader sourcefile [destfile] " << endl;
    cout << "sourcefile = name of file to convert" << endl;
    cout << "destfile   = optional name of file to write to. If you don't" << endl;
    cout << "             specify this OGRE overwrites the existing file." << endl;

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
SkeletonSerializer* skeletonSerializer;
DefaultHardwareBufferManager *bufferManager;


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
    skeletonSerializer = new SkeletonSerializer();
    bufferManager = new DefaultHardwareBufferManager(); // needed because we don't have a rendersystem


    String source(args[1]);

    logMgr->createLog("OgreMeshUpgrader.log");


    // Load the mesh
    struct stat tagStat;

    SDDataChunk chunk;
    stat( source, &tagStat );
    chunk.allocate( tagStat.st_size );
    FILE* pFile = fopen( source.c_str(), "rb" );
    fread( (void*)chunk.getPtr(), tagStat.st_size, 1, pFile );
    fclose( pFile );

    Mesh mesh("conversion");

    meshSerializer->importMesh(chunk, &mesh);

    // Write out the converted mesh
    String dest;
    if (numargs == 3)
    {
        dest = args[2];
    }
    else
    {
        dest = source;
    }


    // Prompt for LOD generation
    bool genLod = false;
    String response;
    if (mesh.getNumLodLevels() > 1)
    {
        std::cout << "\nXML already contains level-of detail information.\n"
            "Do you want to: (u)se it, (r)eplace it, or (d)rop it?";
        while (response == "")
        {
            cin >> response;
            if (response.toLowerCase() == "u")
            {
                // Do nothing
            }
            else if (response.toLowerCase() == "d")
            {
                mesh.removeLodLevels();
            }
            else if (response.toLowerCase() == "r")
            {
                genLod = true;
            }
            else
            {
                response = "";
            }
        }// while response == ""
    }
    else // no existing LOD
    {
        std::cout << "\nWould you like to generate LOD information? (y/n)";
        while (response == "")
        {
            cin >> response;
            if (response.toLowerCase() == "n")
            {
                // Do nothing
            }
            else if (response.toLowerCase() == "y")
            {
                genLod = true;
            }
        }
    }

    if (genLod)
    {
        unsigned short numLod;
        ProgressiveMesh::VertexReductionQuota quota;
        Real reduction;

        cout << "\nHow many extra LOD levels would you like to generate?";
        cin >> numLod;

        cout << "\nWhat unit of reduction would you like to use:"
            "\n(f)ixed or (p)roportional?";
        cin >> response;
        if (response.toLowerCase() == "f")
        {
            quota = ProgressiveMesh::VRQ_CONSTANT;
            cout << "\nHow many vertices should be removed at each LOD?";
        }
        else
        {
            quota = ProgressiveMesh::VRQ_PROPORTIONAL;
            cout << "\nWhat proportion of remaining vertices should be removed "
                "\at each LOD (e.g. 0.5)?";
        }
        cin >> reduction;

        cout << "\nEnter the distance for each LOD to come into effect.";

        Real distance;
        Mesh::LodDistanceList distanceList;
        for (unsigned short iLod = 0; iLod < numLod; ++iLod)
        {
            cout << "\nLOD Level " << (iLod+1) << ":";
            cin >> distance;
            distanceList.push_back(distance);
        }

        mesh.generateLodLevels(distanceList, quota, reduction);
    }




    meshSerializer->exportMesh(&mesh, dest);
    



    delete skeletonSerializer;
    delete meshSerializer;
    delete skelMgr;
    delete matMgr;
    delete mth;
    delete logMgr;

    return 0;

}

