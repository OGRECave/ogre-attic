/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreDefaultHardwareBufferManager.h"
#include <iostream>
#include <sys/stat.h>

using namespace std;
using namespace Ogre;

struct XmlOptions
{
    String source;
    String dest;
    String sourceExt;
    String destExt;
    bool interactiveMode;
    unsigned short numLods;
    Real lodDist;
    Real lodPercent;
    size_t lodFixed;
    bool usePercent;
};

void help(void)
{
    // Print help message
    cout << endl << "OgreXMLConvert: Converts data between XML and OGRE binary formats." << endl;
    cout << "Provided for OGRE by Steve Streeting 2002" << endl << endl;
    cout << "Usage: OgreXMLConverter [-i] [-l lodlevels] [-d loddist] " << endl;
    cout << "                [[-p lodpercent][-f lodnumtris]] sourcefile [destfile] " << endl;
    cout << "-i             = interactive mode - prompt for options" << endl;
    cout << "(The next 4 options are only applicable when converting XML to Mesh)" << endl;
    cout << "-l lodlevels   = number of LOD levels" << endl;
    cout << "-d loddist     = distance increment to reduce LOD" << endl;
    cout << "-p lodpercent  = Percentage triangle reduction amount per LOD" << endl;
    cout << "-f lodnumtris  = Fixed vertex reduction per LOD" << endl;
    cout << "sourcefile     = name of file to convert" << endl;
    cout << "destfile       = optional name of file to write to. If you don't" << endl;
    cout << "                 specify this OGRE works it out through the extension " << endl;
    cout << "                 and the XML contents if the source is XML. For example" << endl;
    cout << "                 test.mesh becomes test.xml, test.xml becomes test.mesh " << endl;
    cout << "                 if the XML document root is <mesh> etc."  << endl;

    cout << endl;
}


XmlOptions parseArgs(int numArgs, char **args)
{
    XmlOptions opts;

    opts.interactiveMode = false;
    opts.lodDist = 500;
    opts.lodFixed = 0;
    opts.lodPercent = 20;
    opts.numLods = 0;
    opts.usePercent = true;

    // ignore program name
    char* source = 0;
    char* dest = 0;
    for (int i = 1; i < numArgs; ++i)
    {
        if (!strncmp(args[i], "-i", 2))
        {
            opts.interactiveMode = true;
            continue;
        }
        else if (!strncmp(args[i], "-l", 2))
        {
            if (strlen(args[i]) > 2)
            {
                // No space
                opts.numLods = StringConverter::parseInt(String(args[i]+2));
            }
            else
            {
                // Space
                ++i;
                opts.numLods = StringConverter::parseInt(String(args[i]));
            }
        }
        else if (!strncmp(args[i], "-d", 2))
        {
            if (strlen(args[i]) > 2)
            {
                // No space
                opts.lodDist = StringConverter::parseReal(String(args[i]+2));
            }
            else
            {
                // Space
                ++i;
                opts.lodDist = StringConverter::parseReal(String(args[i]));
            }
        }
        else if (!strncmp(args[i], "-p", 2))
        {
            if (strlen(args[i]) > 2)
            {
                // No space
                opts.lodPercent = StringConverter::parseReal(String(args[i]+2));
            }
            else
            {
                // Space
                ++i;
                opts.lodPercent = StringConverter::parseReal(String(args[i]));
            }
            opts.usePercent = true;
        }
        else if (!strncmp(args[i], "-f", 2))
        {
            if (strlen(args[i]) > 2)
            {
                // No space
                opts.lodFixed = StringConverter::parseInt(String(args[i]+2));
            }
            else
            {
                // Space
                ++i;
                opts.lodFixed = StringConverter::parseInt(String(args[i]));
            }
            opts.usePercent = false;
        }
        else if (!strncmp(args[i], "-", 1))
        {
            cout << "Invalid option " << args[i] << endl;
            help();
            exit(1);
        }
        else
        {
            // Anything else
            if (!source)
                source = args[i];
            else
                dest = args[i];
        }
    }

    if (!source)
    {
        cout << "Missing source file - abort. " << endl;
        help();
        exit(1);
    }
    // Work out what kind of conversion this is
    opts.source = source;
    std::vector<String> srcparts = opts.source.split(".");
    String& ext = srcparts.back();
    opts.sourceExt = ext.toLowerCase();

    if (!dest)
    {
        if (opts.sourceExt == "xml")
        {
            // dest is source minus .xml
            opts.dest = opts.source.substr(0, opts.source.size() - 4);
        }
        else
        {
            // dest is source + .xml
            opts.dest = opts.source;
            opts.dest.append(".xml");
        }

    }
    else
    {
        opts.dest = dest;
    }
    std::vector<String> dstparts = opts.dest.split(".");
    ext = dstparts.back();
    opts.destExt = ext.toLowerCase();

    cout << endl;
    cout << "-- OPTIONS --" << endl;
    cout << "source file      = " << opts.source << endl;
    cout << "destination file = " << opts.dest << endl;
    cout << "interactive mode = " << StringConverter::toString(opts.interactiveMode) << endl;
    if (opts.numLods == 0)
    {
        cout << "lod levels       = none (or use existing)" << endl;
    }
    else
    {
        cout << "lod levels       = " << opts.numLods << endl;
        cout << "lod distance     = " << opts.lodDist << endl;
        if (opts.usePercent)
        {
            cout << "lod reduction    = " << opts.lodPercent << "%" << endl;
        }
        else
        {
            cout << "lod reduction    = " << opts.lodFixed << " verts" << endl;
        }
    }
    cout << "-- END OPTIONS --" << endl;
    cout << endl;


    return opts;
}

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
DefaultHardwareBufferManager *bufferManager;


void meshToXML(XmlOptions opts)
{
    struct stat tagStat;

    SDDataChunk chunk;
    stat( opts.source, &tagStat );
    chunk.allocate( tagStat.st_size );
    FILE* pFile = fopen( opts.source.c_str(), "rb" );
    if (!pFile)
    {
        cout << "Unable to open file " << opts.source << " - fatal error." << endl;
        exit (1);
    }
    fread( (void*)chunk.getPtr(), tagStat.st_size, 1, pFile );
    fclose( pFile );

    Mesh mesh("conversion");

    meshSerializer->importMesh(chunk, &mesh);
   
    xmlMeshSerializer->exportMesh(&mesh, opts.dest);

}

void XMLToBinary(XmlOptions opts)
{
    // Read root element and decide from there what type
    String response;
    TiXmlDocument* doc = new TiXmlDocument(opts.source);
    // Some double-parsing here but never mind
    if (!doc->LoadFile())
    {
        cout << "Unable to open file " << opts.source << " - fatal error." << endl;
        exit (1);
    }
    TiXmlElement* root = doc->RootElement();
    if (!stricmp(root->Value(), "mesh"))
    {
        delete doc;
        Mesh newMesh("conversion");
        xmlMeshSerializer->importMesh(opts.source, &newMesh);

        // Prompt for LOD generation?
        bool genLod = false;
        bool askLodDtls = false;
        if (!opts.interactiveMode) // derive from params if in batch mode
        {
            askLodDtls = false;
            if (opts.numLods == 0)
            {
                genLod = false;
            }
            else
            {
                genLod = true;
            }
        }
        else if(opts.numLods == 0) // otherwise only ask if not specified on command line
        {
            if (newMesh.getNumLodLevels() > 1)
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
                        newMesh.removeLodLevels();
                    }
                    else if (response.toLowerCase() == "r")
                    {
                        genLod = true;
                        askLodDtls = true;

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
                        askLodDtls = true;
                    }
                }
            }
        }

        if (genLod)
        {
            unsigned short numLod;
            ProgressiveMesh::VertexReductionQuota quota;
            Real reduction;
            Mesh::LodDistanceList distanceList;

            if (askLodDtls)
            {
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
                    cout << "\nWhat percentage of remaining vertices should be removed "
                        "\at each LOD (e.g. 50)?";
                }
                cin >> reduction;
                if (quota == ProgressiveMesh::VRQ_CONSTANT)
                {
                    // Percentage -> parametric
                    reduction = reduction * 0.01f;
                }

                cout << "\nEnter the distance for each LOD to come into effect.";

                Real distance;
                for (unsigned short iLod = 0; iLod < numLod; ++iLod)
                {
                    cout << "\nLOD Level " << (iLod+1) << ":";
                    cin >> distance;
                    distanceList.push_back(distance);
                }
            }
            else
            {
                numLod = opts.numLods;
                quota = opts.usePercent? 
                    ProgressiveMesh::VRQ_PROPORTIONAL : ProgressiveMesh::VRQ_CONSTANT;
                if (opts.usePercent)
                {
                    reduction = opts.lodPercent * 0.01f;
                }
                else
                {
                    reduction = opts.lodFixed;
                }
                Real currDist = 0;
                for (unsigned short iLod = 0; iLod < numLod; ++iLod)
                {
                    currDist += opts.lodDist;
                    distanceList.push_back(currDist);
                }

            }

            newMesh.generateLodLevels(distanceList, quota, reduction);
        }


        meshSerializer->exportMesh(&newMesh, opts.dest);
    }
    else if (!stricmp(root->Value(), "skeleton"))
    {
        delete doc;
        Skeleton newSkel("conversion");
        xmlSkeletonSerializer->importSkeleton(opts.source, &newSkel);
        skeletonSerializer->exportSkeleton(&newSkel, opts.dest);
    }


}

void skeletonToXML(XmlOptions opts)
{
    struct stat tagStat;

    SDDataChunk chunk;
    stat( opts.source, &tagStat );
    chunk.allocate( tagStat.st_size );
    FILE* pFile = fopen( opts.source.c_str(), "rb" );
	if (!pFile)
	{
		cout << "Unable to load file " << opts.source << endl;
		exit(1);
	}
    fread( (void*)chunk.getPtr(), tagStat.st_size, 1, pFile );
    fclose( pFile );

    Skeleton skel("conversion");

    skeletonSerializer->importSkeleton(chunk, &skel);
   
    xmlSkeletonSerializer->exportSkeleton(&skel, opts.dest);

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
    matMgr = new MaterialManager();
    matMgr->initialise();
    skelMgr = new SkeletonManager();
    meshSerializer = new MeshSerializer();
    xmlMeshSerializer = new XMLMeshSerializer();
    skeletonSerializer = new SkeletonSerializer();
    xmlSkeletonSerializer = new XMLSkeletonSerializer();
    bufferManager = new DefaultHardwareBufferManager(); // needed because we don't have a rendersystem



    logMgr->createLog("OgreXMLConverter.log");

    XmlOptions opts = parseArgs(numargs, args);

    if (opts.sourceExt == "mesh")
    {
        meshToXML(opts);
    }
    else if (opts.sourceExt == "skeleton")
    {
        skeletonToXML(opts);
    }
    else if (opts.sourceExt == "xml")
    {
        XMLToBinary(opts);
    }

    

    delete bufferManager;
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

