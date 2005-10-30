/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreDefaultHardwareBufferManager.h"

#include <iostream>
#include <sys/stat.h>

using namespace std;

void help(void)
{
    // Print help message
    cout << endl << "OgreMeshUpgrader: Upgrades .mesh files to the latest version." << endl;
    cout << "Provided for OGRE by Steve Streeting 2004" << endl << endl;
    cout << "Usage: OgreMeshUpgrader [-e] sourcefile [destfile] " << endl;
    cout << "-e         = DON'T generate edge lists (for stencil shadows)" << endl;
    cout << "-t         = Generate tangents (for normal mapping)" << endl;
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
ResourceGroupManager* rgm;
MeshManager* meshMgr;

String describeSemantic(VertexElementSemantic sem)
{
	switch (sem)
	{
	case VES_POSITION:
		return "Positions";
	case VES_NORMAL:
		return "Normals";
	case VES_BLEND_WEIGHTS:
		return "Blend Weights";
	case VES_BLEND_INDICES:
		return "Blend Indices";
	case VES_DIFFUSE:
		return "Diffuse";
	case VES_SPECULAR:
		return "Specular";
	case VES_TEXTURE_COORDINATES:
		return "Texture coordinates";
	case VES_BINORMAL:
		return "Binormals";
	case VES_TANGENT:
		return "Tangents";
	};
    return "";
}
void displayVertexBuffers(VertexDeclaration::VertexElementList& elemList)
{
	// Iterate per buffer
	unsigned short currentBuffer = 999;
	unsigned short elemNum = 0;
	VertexDeclaration::VertexElementList::iterator i, iend;
	iend = elemList.end();
	for (i = elemList.begin(); i != iend; ++i)
	{
		if (i->getSource() != currentBuffer)
		{
			currentBuffer = i->getSource();
			cout << "> Buffer " << currentBuffer << ":" << endl;
		}
		cout << "   - Element " << elemNum++ << ": " << describeSemantic(i->getSemantic());
		if (i->getSemantic() == VES_TEXTURE_COORDINATES)
		{
			cout << " (index " << i->getIndex() << ")"; 
		}
		cout << endl;

	}
}
// Sort routine for VertexElement
bool vertexElementLess(const VertexElement& e1, const VertexElement& e2)
{
	// Sort by source first
	if (e1.getSource() < e2.getSource())
	{
		return true;
	}
	else if (e1.getSource() == e2.getSource())
	{
		// Use ordering of semantics to sort
		if (e1.getSemantic() < e2.getSemantic())
		{
			return true;
		}
		else if (e1.getSemantic() == e2.getSemantic())
		{
			// Use index to sort
			if (e1.getIndex() < e2.getIndex())
			{
				return true;
			}
		}
	}
	return false;
}
void copyElems(VertexDeclaration* decl, VertexDeclaration::VertexElementList* elemList)
{
	
	elemList->clear();
	const VertexDeclaration::VertexElementList& origElems = decl->getElements();
    VertexDeclaration::VertexElementList::const_iterator i, iend;
	iend = origElems.end();
	for (i = origElems.begin(); i != iend; ++i)
	{
		elemList->push_back(*i);
	}
	elemList->sort(VertexDeclaration::vertexElementLess);
}
// Utility function to allow the user to modify the layout of vertex buffers.
void reorganiseVertexBuffers(const String& desc, Mesh& mesh, VertexData* vertexData)
{
	cout << endl << desc << ":- " << endl;
	// Copy elements into a list
	VertexDeclaration::VertexElementList elemList;
	copyElems(vertexData->vertexDeclaration, &elemList);

	bool finish = false;
	bool anyChanges = false;
	while (!finish)
	{
		displayVertexBuffers(elemList);
		cout << endl;

		cout << "Options: (a)utomatic" << endl;
        cout << "         (m)ove element" << endl;
		cout << "         (d)elete element" << endl;
		cout << "         (r)eset" << endl;
		cout << "         (f)inish" << endl;
		String response = "";
		while (response.empty())
		{
			cin >> response;
			StringUtil::toLowerCase(response);

			if (response == "m")
			{
				String moveResp;
				cout << "Which element do you want to move (type number): ";
				cin >> moveResp;
				if (!moveResp.empty())
				{
					int eindex = StringConverter::parseInt(moveResp);
					VertexDeclaration::VertexElementList::iterator movei = elemList.begin();
					std::advance(movei, eindex);
					cout << endl << "Move element " << eindex << "(" + describeSemantic(movei->getSemantic()) << ") to which buffer: ";
					cin >> moveResp;
					if (!moveResp.empty())
					{
						int bindex = StringConverter::parseInt(moveResp);
						// Move (note offset will be wrong)
						*movei = VertexElement(bindex, 0, movei->getType(),
							movei->getSemantic(), movei->getIndex());
						elemList.sort(vertexElementLess);
                        anyChanges = true;
								
					}
				}
			}
            else if (response == "a")
            {
                // Automatic
                VertexDeclaration* newDcl = 
                    vertexData->vertexDeclaration->getAutoOrganisedDeclaration(
                        mesh.hasSkeleton(), mesh.hasVertexAnimation());
                copyElems(newDcl, &elemList);
                HardwareBufferManager::getSingleton().destroyVertexDeclaration(newDcl);
                anyChanges = true;

            }
			else if (response == "d")
			{
				String moveResp;
				cout << "Which element do you want to delete (type number): ";
				cin >> moveResp;
				if (!moveResp.empty())
				{
					int eindex = StringConverter::parseInt(moveResp);
					VertexDeclaration::VertexElementList::iterator movei = elemList.begin();
					std::advance(movei, eindex);
                    cout << std::endl << "Delete element " << eindex << "(" + describeSemantic(movei->getSemantic()) << ")?: ";
					cin >> moveResp;
					StringUtil::toLowerCase(moveResp);
					if (moveResp == "y")
					{
						elemList.erase(movei);
                        anyChanges = true;
					}
				}
			}
			else if (response == "r")
			{
				// reset
				copyElems(vertexData->vertexDeclaration, &elemList);
				anyChanges = false;
			}
			else if (response == "f")
			{
				// finish
				finish = true;
			}
			else
			{
				response == "";
			}
			
		}
	}

	if (anyChanges)
	{
		String response;
		while (response.empty())
		{
			displayVertexBuffers(elemList);
			cout << "Really reorganise the vertex buffers this way?";
			cin >> response;
			StringUtil::toLowerCase(response);
			if (response == "y")
			{
				VertexDeclaration* newDecl = HardwareBufferManager::getSingleton().createVertexDeclaration();
				VertexDeclaration::VertexElementList::iterator i, iend;
				iend = elemList.end();
				unsigned short currentBuffer = 999;
				size_t offset;
				for (i = elemList.begin(); i != iend; ++i)
				{
					// Calc offsets since reorg changes them
					if (i->getSource() != currentBuffer)
					{
						offset = 0;
						currentBuffer = i->getSource();
					}
					newDecl->addElement(
						currentBuffer,
						offset,
						i->getType(),
						i->getSemantic(),
						i->getIndex());

					offset += VertexElement::getTypeSize(i->getType());
					
				}
                // Usages don't matter here since we're onlly exporting
                BufferUsageList bufferUsages;
                for (size_t u = 0; u <= newDecl->getMaxSource(); ++u)
                    bufferUsages.push_back(HardwareBuffer::HBU_STATIC_WRITE_ONLY);
				vertexData->reorganiseBuffers(newDecl, bufferUsages);
			}
			else if (response == "n")
			{
				// do nothing
			}
			else
			{
				response = "";
			}
        }
		
	}
		

	
}
// Utility function to allow the user to modify the layout of vertex buffers.
void reorganiseVertexBuffers(Mesh& mesh)
{
	if (mesh.sharedVertexData)
	{
		reorganiseVertexBuffers("Shared Geometry", mesh, mesh.sharedVertexData);
	}

    Mesh::SubMeshIterator smIt = mesh.getSubMeshIterator();
	unsigned short idx = 0;
	while (smIt.hasMoreElements())
	{
		SubMesh* sm = smIt.getNext();
		if (!sm->useSharedVertices)
		{
			StringUtil::StrStreamType str;
			str << "SubMesh " << idx++; 
			reorganiseVertexBuffers(str.str(), mesh, sm->vertexData);
		}
	}
}


int main(int numargs, char** args)
{
    if (numargs < 2)
    {
        help();
        return -1;
    }

    logMgr = new LogManager();
	logMgr->createLog("OgreMeshUpgrade.log", true);
    rgm = new ResourceGroupManager();
    mth = new Math();
    matMgr = new MaterialManager();
    matMgr->initialise();
    skelMgr = new SkeletonManager();
    meshSerializer = new MeshSerializer();
    skeletonSerializer = new SkeletonSerializer();
    bufferManager = new DefaultHardwareBufferManager(); // needed because we don't have a rendersystem
    meshMgr = new MeshManager();
	// don't pad during upgrade
	meshMgr->setBoundsPaddingFactor(0.0f);

    
    UnaryOptionList unOptList;
    BinaryOptionList binOptList;

    unOptList["-e"] = false;
    unOptList["-t"] = false;
    int startIdx = findCommandLineOpts(numargs, args, unOptList, binOptList);

    String source(args[startIdx]);

    logMgr->createLog("OgreMeshUpgrader.log");


    // Load the mesh
    struct stat tagStat;

    FILE* pFile = fopen( source.c_str(), "rb" );
    if (!pFile)
    {
        OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND, 
            "File " + source + " not found.", "OgreMeshUpgrade");
    }
    stat( source.c_str(), &tagStat );
    MemoryDataStream* memstream = new MemoryDataStream(source, tagStat.st_size, true);
    fread( (void*)memstream->getPtr(), tagStat.st_size, 1, pFile );
    fclose( pFile );

	Mesh mesh(meshMgr, "conversion", 0, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    DataStreamPtr stream(memstream);
    meshSerializer->importMesh(stream, &mesh);

    // Write out the converted mesh
    String dest;
    if (numargs == startIdx + 2)
    {
        dest = args[startIdx + 1];
    }
    else
    {
        dest = source;
    }

	String response;

	// Check to see whether we would like to reorganise vertex buffers
    std::cout << "\nWould you like to reorganise the vertex buffers for this mesh?";
	while (response.empty())
	{
		cin >> response;
		StringUtil::toLowerCase(response);
		if (response == "y")
		{
			reorganiseVertexBuffers(mesh);
		}
		else if (response == "n")
		{
			// Do nothing
		}
		else
		{
			response = "";
		}
	}
	
    // Prompt for LOD generation
    bool genLod = false;
    response = "";
    if (mesh.getNumLodLevels() > 1)
    {
        std::cout << "\nMesh already contains level-of detail information.\n"
            "Do you want to: (u)se it, (r)eplace it, or (d)rop it?";
        while (response.empty())
        {
            cin >> response;
			StringUtil::toLowerCase(response);
            if (response == "u")
            {
                // Do nothing
            }
            else if (response == "d")
            {
                mesh.removeLodLevels();
            }
            else if (response == "r")
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
			StringUtil::toLowerCase(response);
            if (response == "n")
            {
                // Do nothing
            }
            else if (response == "y")
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
		StringUtil::toLowerCase(response);
        if (response == "f")
        {
            quota = ProgressiveMesh::VRQ_CONSTANT;
            cout << "\nHow many vertices should be removed at each LOD?";
        }
        else
        {
            quota = ProgressiveMesh::VRQ_PROPORTIONAL;
            cout << "\nWhat proportion of remaining vertices should be removed " <<
                "at each LOD (e.g. 0.5)?";
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

    // Make sure we generate edge lists, provided they are not deliberately disabled
    UnaryOptionList::iterator ui = unOptList.find("-e");

    if (!ui->second)
    {
        cout << "\nGenerating edge lists.." << std::endl;
        mesh.buildEdgeList();
    }

    // Generate tangents?
    ui = unOptList.find("-t");
    bool generateTangents = ui->second;
    if (generateTangents)
    {
        unsigned short srcTex, destTex;
        bool existing = mesh.suggestTangentVectorBuildParams(srcTex, destTex);
        if (existing)
        {
            std::cout << "\nThis mesh appears to already have a set of 3D texture coordinates, " <<
                "which would suggest tangent vectors have already been calculated. Do you really " <<
                "want to generate new tangent vectors (may duplicate)? (y/n)";
            while (response == "")
            {
                cin >> response;
                StringUtil::toLowerCase(response);
                if (response == "y")
                {
                    // Do nothing
                }
                else if (response == "n")
                {
                    generateTangents = false;
                }
                else
                {
                    response = "";
                }
            }

        }
        if (generateTangents)
        {
            cout << "Generating tangent vectors...." << std::endl;
            mesh.buildTangentVectors(srcTex, destTex);
        }
    }



    meshSerializer->exportMesh(&mesh, dest);
    



    delete meshMgr;
    delete skeletonSerializer;
    delete meshSerializer;
    delete skelMgr;
    delete matMgr;
    delete mth;
    delete rgm;
    delete logMgr;

    return 0;

}

