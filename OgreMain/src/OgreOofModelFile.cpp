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
#include "OgreOofModelFile.h"

#include "OgreOofFile.h"
#include "OgreException.h"
#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreSDDataChunk.h"

#include "zlib.h"

namespace Ogre {

    OofModelFile::OofModelFile()
    {
        sharedGeometry.pColours = 0;
        sharedGeometry.pNormals = 0;
        for (int i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; ++i)
            sharedGeometry.pTexCoords[i] = 0;
        sharedGeometry.pVertices = 0;
        autoDeallocateMemory = true;
    }

    OofModelFile::~OofModelFile()
    {
        freeMemory();
    }

    void OofModelFile::load(String filename)
    {
        this->filename = filename;
        // No compression
        FILE* fp;

        fp = fopen(filename.c_str(), "rb");

        // Read entire file into memory
        // This is to standardise dealing with data uncompressed
        // from archives and from files
        fseek(fp, 0, SEEK_END);
        size_t filesize = ftell(fp);
        fseek(fp, 0 , SEEK_SET);

        SDDataChunk tempChunk;
        tempChunk.allocate( static_cast< unsigned long >( filesize ) );

        fread( 
			reinterpret_cast< void * >( const_cast< unsigned char * >( tempChunk.getPtr() ) ), 
			filesize, 1, fp);

        // Use memory loader
        load(tempChunk);

        tempChunk.clear();
    }

    void OofModelFile::load(DataChunk& chunk)
    {
        unsigned short val;
        unsigned short dataSize;

        // Free any exsiting memory
        freeMemory();

        // Read header
        unsigned short numObjects;
        unsigned short numMaterials;

        chunk.read(&val, sizeof(val));
        if (val == OOF_HEADER)
        {
            chunk.read(&dataSize, sizeof(unsigned short));
            // read nummaterials, numobjects
            chunk.read(&numMaterials, sizeof(unsigned short));
            chunk.read(&numObjects, sizeof(unsigned short));
        }
        else
        {
            Except(999, "Missing header in OOF file.", "OofModelFile::loadOof");
        }


        int readCount;
        readCount = chunk.read(&val, sizeof(val));

        // int matNum = -1;
        unsigned short matIndex;
        while (readCount > 0)
        {
            unsigned short chunkId = val;
            // Get data size
            chunk.read(&val, sizeof(val));
            dataSize = val;

            MaterialData matdata;            
            ColourValue ambient, diffuse, specular;
            Real shininess;
            // determine type
            switch (chunkId)
            {
            case OOF_MATERIAL:
                char matName[255];
                readCount = chunk.readUpTo(matName, 255);
                // Terminate
                matName[readCount] = '\0';
                matdata.material = Material(matName);
                // Init geometry info
                matdata.useSharedVertices = true;
                matdata.pIndexes = 0;
                matdata.numFaces = 0;
                chunk.read(&ambient.r, sizeof(float));
                chunk.read(&ambient.g, sizeof(float));
                chunk.read(&ambient.b, sizeof(float));
                chunk.read(&diffuse.r, sizeof(float));
                chunk.read(&diffuse.g, sizeof(float));
                chunk.read(&diffuse.b, sizeof(float));
                chunk.read(&specular.r, sizeof(float));
                chunk.read(&specular.g, sizeof(float));
                chunk.read(&specular.b, sizeof(float));
                chunk.read(&shininess, sizeof(float));
                matdata.material.setAmbient(ambient);
                matdata.material.setDiffuse(diffuse);
                matdata.material.setSpecular(specular);
                matdata.material.setShininess(shininess);
                materials.push_back(matdata);
                break;
            case OOF_TEXTURE_LAYER:
                char texname[64];
                readCount = chunk.readUpTo(texname, 64);
                // Terminate
                texname[readCount] = '\0';
                // Add to latest material
                materials.back().material.addTextureLayer(String(texname));

                break;
            case OOF_OBJECT:
                // assume single object for now
                char objname[64];
                readCount = chunk.readUpTo(objname, 64);
                // Terminate
                objname[readCount] = '\0';
                chunk.read(&val, sizeof(unsigned short));
                sharedGeometry.numVertices = val;
                // Assume no normals, colours, textures for now
                sharedGeometry.hasNormals = false;
                sharedGeometry.numTexCoords = 0;
                sharedGeometry.hasColours = false;
                // Init strides
                sharedGeometry.vertexStride = 0;
                sharedGeometry.normalStride = 0;
                sharedGeometry.colourStride = 0;
                sharedGeometry.texCoordStride[0] = 0;
                sharedGeometry.texCoordStride[1] = 0;

                break;
            case OOF_VSHAREDPOSITIONS:
                sharedGeometry.pVertices =
                    new Real[sharedGeometry.numVertices*3];
                chunk.read(sharedGeometry.pVertices,
                    sizeof(Real) * sharedGeometry.numVertices * 3);
                break;
            case OOF_VSHAREDNORMALS:
                sharedGeometry.hasNormals = true;
                sharedGeometry.pNormals =
                    new Real[sharedGeometry.numVertices*3];
                chunk.read(sharedGeometry.pNormals ,
                    sizeof(Real) * sharedGeometry.numVertices * 3);
                break;
            case OOF_VSHAREDTEXCOORDS:
                // Assume 2D textures for now, 1 tex coord set
                sharedGeometry.numTexCoords = 1;
                sharedGeometry.numTexCoordDimensions[0] = 2;
                sharedGeometry.pTexCoords[0] =
                    new Real[sharedGeometry.numVertices*2];
                chunk.read(sharedGeometry.pTexCoords[0],
                    sizeof(Real) * sharedGeometry.numVertices * 2);
                break;
            case OOF_VSHAREDCOLOURS:
                // TODO - not yet supported
                break;
            case OOF_MATERIAL_GROUP:
                // Read material index
                chunk.read(&matIndex, sizeof(unsigned short));
                // Read number of faces
                chunk.read(&materials[matIndex].numFaces, sizeof(unsigned short));
                // Allocate memory for face indexes
                materials[matIndex].pIndexes = new unsigned short[materials[matIndex].numFaces * 3];
                // Read faces
                chunk.read(materials[matIndex].pIndexes,
                    sizeof(unsigned short) * materials[matIndex].numFaces * 3);
                // Number of dedicated vertices
                chunk.read(&val, sizeof(unsigned short));
                if (val > 0)
                {
                    materials[matIndex].useSharedVertices = false;
                    materials[matIndex].materialGeometry.numVertices = val;
                    // Assume no normals, colours, textures for now
                    materials[matIndex].materialGeometry.hasNormals = false;
                    materials[matIndex].materialGeometry.numTexCoords = 0;
                    materials[matIndex].materialGeometry.hasColours = false;
                    // Init strides
                    materials[matIndex].materialGeometry.vertexStride = 0;
                    materials[matIndex].materialGeometry.normalStride = 0;
                    materials[matIndex].materialGeometry.colourStride = 0;
                    materials[matIndex].materialGeometry.texCoordStride[0] = 0;
                    materials[matIndex].materialGeometry.texCoordStride[1] = 0;
                }
                else
                {
                    materials[matIndex].useSharedVertices = true;
                }

                break;
            case OOF_VPOSITIONS:
                materials[matIndex].materialGeometry.pVertices =
                    new Real[materials[matIndex].materialGeometry.numVertices*3];
                chunk.read(materials[matIndex].materialGeometry.pVertices,
                    sizeof(Real) * materials[matIndex].materialGeometry.numVertices * 3);
                break;
            case OOF_VNORMALS:
                materials[matIndex].materialGeometry.hasNormals = true;
                materials[matIndex].materialGeometry.pNormals =
                    new Real[materials[matIndex].materialGeometry.numVertices*3];
                chunk.read(materials[matIndex].materialGeometry.pNormals ,
                    sizeof(Real) * materials[matIndex].materialGeometry.numVertices * 3);
                break;
            case OOF_VTEXCOORDS:
                // Assume 2D textures for now, 1 tex coord set
                materials[matIndex].materialGeometry.numTexCoords = 1;
                materials[matIndex].materialGeometry.numTexCoordDimensions[0] = 2;
                materials[matIndex].materialGeometry.pTexCoords[0] =
                    new Real[materials[matIndex].materialGeometry.numVertices*2];
                chunk.read(materials[matIndex].materialGeometry.pTexCoords[0],
                    sizeof(Real) * materials[matIndex].materialGeometry.numVertices * 2);
                break;
            case OOF_VCOLOURS:
                // TODO - not yet supported
                break;

            }

            // Read next header
            readCount = chunk.read(&val, sizeof(val));
        }



    }

    void OofModelFile::freeMemory(void)
    {
        if (autoDeallocateMemory)
        {
            if (sharedGeometry.pVertices)
            {
                delete[] sharedGeometry.pVertices;
                sharedGeometry.pVertices = 0;
            }
            if (sharedGeometry.pNormals)
            {
                delete[] sharedGeometry.pNormals;
                sharedGeometry.pNormals = 0;
            }
            for (int j = 0; j < OGRE_MAX_TEXTURE_COORD_SETS; ++j)
            {
                if (sharedGeometry.pTexCoords[j])
                {
                    delete[] sharedGeometry.pTexCoords[j];
                    sharedGeometry.pTexCoords[j] = 0;
                }
            }
            if (sharedGeometry.pColours)
            {
                delete[] sharedGeometry.pColours;
                sharedGeometry.pColours = 0;
            }

            std::vector<MaterialData>::iterator i =
                materials.begin();
            for (; i != materials.end(); ++i)
            {
                if (i->pIndexes)
                    delete[] i->pIndexes;

                if (!i->useSharedVertices)
                {
                    if (i->materialGeometry.pVertices)
                        delete[] i->materialGeometry.pVertices;

                    if (i->materialGeometry.pNormals)
                        delete[] i->materialGeometry.pNormals;

                    for (int j = 0; j < OGRE_MAX_TEXTURE_COORD_SETS; ++j)
                    {
                        if (i->materialGeometry.pTexCoords[j])
                        {
                            delete[] i->materialGeometry.pTexCoords[j];
                        }
                    }

                    if (i->materialGeometry.pColours)
                        delete[] i->materialGeometry.pColours;
                }

            }
        }
        materials.clear();



    }


}
