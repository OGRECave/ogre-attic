/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
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

/** Converts .3DS files to .OOF (Ogre Object File).
    Uses 3DS loader from Matthew Fairfax, with some alterations / enhancements.

    Known limitations:
        Animation not supported
        3DS object hierarchy is not preserved
        Only one set of texture coordinates are read from 3DS
        Composite materials (multitexturing) are not supported
            (although you can add extra layers in Ogre).
        Every face must be assigned a material, otherwise it will be
            excluded from the output.

  Yeah, I know this code is bad quality & needs refactoring. It started
  small & grew fast.

 */
#pragma warning (disable : 4786)

#include <iostream>
#include <map>

#include "zlib.h"
#include "Model_3DS.h"
#include "OgreException.h"
#include "OgreOofFile.h"


using namespace Ogre;
using namespace std;

struct OofOptions
{
    bool incNormals;
    bool incTexCoords;
    bool incVertColours;
    bool onlyNamedObj;
    Ogre::String objName;
    int  shareVerticesLimit;
    bool changeTextureExtensions;
    Ogre::String newTextureExtension;
    bool promptMatNames;
    bool resetMatColours;
    bool separateObjects;
};

// TODO - move constants & methods for saving / loading OOF into a core class
void help(void)
{
    // Print help message
    cout << endl << "3ds2oof: Converts 3DS data to OGRE Object Format (OOF)" << endl;
    cout << "Provided for OGRE by Steve Streeting 2001" << endl << endl;
    cout << "Usage: 3ds2oof filename [-vn] [-tc] [-vc] [-oObjectName] [-lnnn] [-snnn] [-extExtension]" << endl;
    cout << "filename = name of 3DS file to open" << endl;
    cout << "The following optional parameters EXCLUDE features from the output " << endl;
    cout << "even if they are in the 3DS file being loaded." << endl;
    cout << "-vn      = EXCLUDES vertex normals." << endl;
    cout << "-tc      = EXCLUDES texture coordinates." << endl;
    cout << "-vc      = EXCLUDES vertex colours." << endl;
    cout << "Other options:" << endl;
    cout << "-o<Name> = Outputs just the named object." << endl;
    cout << "-snnn    = Share vertices percentage limit. If a material group uses more than " << endl;
    cout << "           this percentage of vertices, it will use a shared buffer instead of " << endl;
    cout << "           a dedicated one.See readme.txt for more information. " << endl;
    cout << "-ext<Extension> = Changes the extension of the texture map references." << endl;
    cout << "-matnames = Prompt for new names of materials." << endl;
    cout << "-usecolours = Use the material colours from 3DS (not normally recommended)." << endl;
    cout << "-separate = Produce separate .oof files for each object in the .3ds. " << endl;
    cout << endl << "See README.TXT for full information on use of this tool." << endl;
    cout << endl;
}

// Globals for file
OofOptions opt;
gzFile gzfp;
FILE* fp;

int fileWrite(const void* buf, size_t size, size_t count)
{
    // Writes compressed bytes
    //return gzwrite(gzfp, (void* const)buf, size*count);
    // Write uncompressed
    return fwrite((void* const)buf, size, count, fp);

}

int filePuts(const char* text)
{
    // Writes compressed bytes
    //int res =  gzputs(gzfp, text);
    // Write terminating newline char
    //gzputc(gzfp, '\n');

    // Write uncompressed
    int res = fputs(text, fp);
    // Write terminating newline char
    fputc('\n', fp);

    return res;
}

void outputLine()
{
    cout << "-----------------------------------------------------------------------------" << endl;
}

void writeOof(Ogre::String& filename, Model_3DS& model, int objIndex = -1)
{
    unsigned short val;
    float fval;
    int i, vert;

    //gzfp = gzopen(filename.c_str(), "wb");
    fp = fopen(filename.c_str(), "wb");
    if (!fp)
        throw Ogre::String("Cannot open  output file.");

    // Write header
    val = OOF_HEADER;
    fileWrite(&val, sizeof(val), 1);
    // Write size of data
    val = sizeof(unsigned short) * 2;
    fileWrite(&val, sizeof(val), 1);

    // Number of materials
    val = model.numMaterials;
    fileWrite(&val, sizeof(val), 1);
    // Number of objects
    val = model.numObjects;
    fileWrite(&val, sizeof(val), 1);


    // Write rest of file
    // Materials
    outputLine();
    cout << "Materials" << endl;
    outputLine();

    cout << "Number of Materials: " << model.numMaterials << endl;
    for (i = 0; i < model.numMaterials; ++i)
    {
        outputLine();
        cout << "Material Entry " << i << ": " << model.Materials[i].name << endl;
        outputLine();
        val = OOF_MATERIAL;
        fileWrite(&val, sizeof(val), 1);
        // write data size
        val = strlen(model.Materials[i].name) + 1; // name
        val += sizeof(float) * 10; // colours + shininess
        val += sizeof(unsigned short); // texture layer count
        if (model.Materials[i].textured)
        {
            val += sizeof(unsigned short) * 2; // texture layer chunk header
            val += strlen(model.Materials[i].texname) + 1; // name + \n
        }
        fileWrite(&val, sizeof(val), 1);

        // Reset material colours if required
        if (opt.resetMatColours)
        {
            model.Materials[i].ambient.r = 255;
            model.Materials[i].ambient.g = 255;
            model.Materials[i].ambient.b = 255;
            model.Materials[i].diffuse.r = 255;
            model.Materials[i].diffuse.g = 255;
            model.Materials[i].diffuse.b = 255;
            model.Materials[i].specular.r = 255;
            model.Materials[i].specular.g = 255;
            model.Materials[i].specular.b = 255;
        }

        cout << "Name: " << model.Materials[i].name << endl;
        filePuts(model.Materials[i].name);
        cout << "Ambient Colour: ";
        fval = model.Materials[i].ambient.r / 255.0;
        cout << "R" << fval;
        fileWrite(&fval, sizeof(fval), 1);
        fval = model.Materials[i].ambient.g / 255.0;
        cout << " G" << fval;
        fileWrite(&fval, sizeof(fval), 1);
        fval = model.Materials[i].ambient.b / 255.0;
        cout << " B" << fval << endl;
        fileWrite(&fval, sizeof(fval), 1);


        cout << "Diffuse Colour: ";
        fval = model.Materials[i].diffuse.r / 255.0;
        cout << "R" << fval;
        fileWrite(&fval, sizeof(fval), 1);
        fval = model.Materials[i].diffuse.g / 255.0;
        cout << " G" << fval;
        fileWrite(&fval, sizeof(fval), 1);
        fval = model.Materials[i].diffuse.b / 255.0;
        cout << " B" << fval << endl;
        fileWrite(&fval, sizeof(fval), 1);


        cout << "Specular Colour: ";
        fval = model.Materials[i].specular.r / 255.0;
        cout << "R" << fval;
        fileWrite(&fval, sizeof(fval), 1);
        fval = model.Materials[i].specular.g / 255.0;
        cout << " G" << fval;
        fileWrite(&fval, sizeof(fval), 1);
        fval = model.Materials[i].specular.b / 255.0;
        cout << " B" << fval << endl;
        fileWrite(&fval, sizeof(fval), 1);


        fval = (float)model.Materials[i].shininess;
        fileWrite(&fval, sizeof(fval), 1);

        cout << "Shininess: " << val << endl;

        // Texture layers (only 0 or 1)
        if (model.Materials[i].textured)
        {
            // Change texture extension if required
            char texName[100];
            strcpy(texName, model.Materials[i].texname);

            if (opt.changeTextureExtensions)
            {
                char* pDot = strrchr(texName, '.');
                if (pDot)
                {
                    // copy in new extension
                    pDot++; // skip dot
                    strcpy(pDot, opt.newTextureExtension.c_str());
                }
            }

            cout << "Texture Layer 0: " << texName << endl;
            // Define single texture
            val = OOF_TEXTURE_LAYER;
            fileWrite(&val, sizeof(val), 1);
            // Write size
            val = strlen(texName) + 1;
            fileWrite(&val, sizeof(val), 1);
            // data
            filePuts(texName);
        }

    } // Next Material


    // Now go through objects
    outputLine();
    cout << "Objects" << endl;
    unsigned short objTotalVerts = 0;;

    int lbound, ubound;

    if (objIndex == -1)
    {
        lbound = 0;
        ubound = model.numObjects - 1;
    }
    else
    {
        lbound = objIndex;
        ubound = objIndex;
    }

    // Count total vertices
    for (i = lbound; i <= ubound; ++i)
        objTotalVerts += model.Objects[i].numVerts;

    unsigned short totalSharedVerts = 0;
    bool allSharedVertices = true;

    // Map to record which vertices used by each material,
    // and (if applicable) where they are in a dedicated list
    typedef std::map<int, int> VertIndexMap;
    VertIndexMap vertIndexMap;

    // Skim through each material group in each object first, to see if any
    // will require a shared vertex pool.
    for (i = lbound; i <= ubound; ++i)
    {
        // Skip this one if not named object
        if (opt.onlyNamedObj &&
            strcmp(model.Objects[i].name, opt.objName.c_str()))
        {
            cout << "Skipped " << model.Objects[i].name << 
                " because you chose to export only object named " << 
                opt.objName << "." << endl;
            continue;
        }
        // Skip this one if there is no geometry 
        if (model.Objects[i].numFaces == 0)
        {
            cout << "Skipped " << model.Objects[i].name << 
                " because it has no faces." << endl;
            continue;
        }
        // Init
        model.Objects[i].includeSharedVertices = false;

        for (int grp = 0; grp < model.Objects[i].numMatFaces; ++grp)
        {
            vertIndexMap.clear();

            for (int vertIdx = 0;
            vertIdx < model.Objects[i].MatFaces[grp].numSubFaces;
            ++vertIdx)
            {
                // Try to insert index into map (will only succeed if
                // it is unique), note destination vert reference is
                // the size() i.e. start at 0 and increase each time
                // unique
                vertIndexMap.insert(
                    VertIndexMap::value_type(model.Objects[i].MatFaces[grp].subFaces[vertIdx],vertIndexMap.size()));
            };

            // Count unique vertices used, compare to total vertices
            if ((vertIndexMap.size() * 100 / objTotalVerts) >= static_cast<unsigned>(opt.shareVerticesLimit))
            {
                // Use shared vertices - number used exceeds limit
                model.Objects[i].MatFaces[grp].useOwnVertexList = false;
                model.Objects[i].MatFaces[grp].numOwnVertices = 0;
                model.Objects[i].includeSharedVertices = true;
                if (totalSharedVerts > 0)
                {
                    // Adjust vert indexes to compensate for combined buffer between objects
                    for (vert = 0; vert < model.Objects[i].MatFaces[grp].numSubFaces; ++vert)
                        model.Objects[i].MatFaces[grp].subFaces[vert] += totalSharedVerts;

                }
                totalSharedVerts += model.Objects[i].numVerts;
            }
            else
            {
                // Use dedicated vertices
                allSharedVertices = false;
                model.Objects[i].MatFaces[grp].useOwnVertexList = true;
                model.Objects[i].MatFaces[grp].numOwnVertices = vertIndexMap.size();
                // Copy vertex details into material-face list
                model.Objects[i].MatFaces[grp].ownVertices = new float[vertIndexMap.size() * 3];
                model.Objects[i].MatFaces[grp].ownNormals = new float[vertIndexMap.size() * 3];
                model.Objects[i].MatFaces[grp].ownTexCoords = new float[vertIndexMap.size() * 2];

                VertIndexMap::iterator setIter = vertIndexMap.begin();

                for (unsigned int setIdx = 0; setIdx < vertIndexMap.size(); ++setIdx)
                {
                    // Copy data - source is index provided by key of
                    // the map (ie. first)
                    // Dest is value (second)
                    // Copy vertices
                    memcpy(&(model.Objects[i].MatFaces[grp].ownVertices[setIter->second*3]),
                        &(model.Objects[i].Vertexes[setIter->first*3]),
                        sizeof(float)*3);
                    // Copy normals
                    memcpy(&(model.Objects[i].MatFaces[grp].ownNormals[setIter->second*3]),
                        &(model.Objects[i].Normals[setIter->first*3]),
                        sizeof(float)*3);
                    // Copy Texture Coords
                    memcpy(&(model.Objects[i].MatFaces[grp].ownTexCoords[setIter->second*2]),
                        &(model.Objects[i].TexCoords[setIter->first*2]),
                        sizeof(float)*2);
                    setIter++;
                }

                // Re-point vert indexes using map
                for (int faceIndex = 0; faceIndex < model.Objects[i].MatFaces[grp].numSubFaces; ++faceIndex)
                {
                    // Look up original index in map
                    setIter = vertIndexMap.find(model.Objects[i].MatFaces[grp].subFaces[faceIndex]);
                    // Alter to new index
                    model.Objects[i].MatFaces[grp].subFaces[faceIndex] = setIter->second;
                }

            }

        }

        outputLine();
        cout << "Object Entry " << i << ": " << model.Objects[i].name << endl;
        outputLine();
        cout << "Name: " << model.Objects[i].name << endl;
        cout << "Total Vertices: " << model.Objects[i].numVerts << endl;
        if (allSharedVertices)
        {
            cout << "A shared vertex list will be used for ALL material faces." << endl;
        }
        else if (totalSharedVerts > 0)
        {
            cout << "A mixture of shared and dedicated vertex lists will be used for material faces." << endl;
        }
        else
        {
            cout << "Separate vertex lists will be used per material." << endl;
        }
        if (i > lbound)
        {
            cout << "This object will be merged into the mesh definition of " << model.Objects[lbound].name << endl;
        }

    }

    // Write single object entry for one or more objects (merged if more than one)

    val = OOF_OBJECT;
    fileWrite(&val, sizeof(val), 1);

    // write size
    val = strlen(model.Objects[lbound].name) + 1;                // name
    val += sizeof(unsigned short);                            // number of verts
    if (totalSharedVerts > 0)
    {
        val += sizeof(unsigned short);                            // position header
        val += sizeof(float) * model.Objects[lbound].numVerts * 3;    // positions
        if (opt.incNormals)
        {
            val += sizeof(unsigned short);                            // normal header
            val += sizeof(float) * model.Objects[lbound].numVerts * 3;    // normals
        }
        if (model.Objects[lbound].textured && opt.incTexCoords)
        {
            val += sizeof(unsigned short);                        // texture header
            val += sizeof(float) * model.Objects[lbound].numVerts * 2; // tex coords
        }
    }
    val += sizeof(unsigned short) *
        model.Objects[lbound].numMatFaces * 2; // mat group headers
    val += sizeof(unsigned short) *
        model.Objects[lbound].numMatFaces * 2; // mat group mat index & num faces
    for (int j = 0; j < model.Objects[lbound].numMatFaces; ++j)
    {
        val += sizeof(unsigned short) *
            model.Objects[lbound].MatFaces[j].numSubFaces * 3;    // mat group faces
    }
    fileWrite(&val, sizeof(val), 1);

    // Object details
    filePuts(model.Objects[lbound].name);
    val = totalSharedVerts;
    fileWrite(&val, sizeof(val), 1);


    if (totalSharedVerts > 0)
    {
        // Shared vertex positions for all objects
        val = OOF_VSHAREDPOSITIONS;
        fileWrite(&val, sizeof(val), 1);
        val = sizeof(float) * totalSharedVerts * 3;
        fileWrite(&val, sizeof(val), 1);
        // Write shared vertices for all objects
        for (i = lbound; i <= ubound; ++i)
        {
            if (model.Objects[i].includeSharedVertices)
            {
                fileWrite(model.Objects[i].Vertexes, sizeof(float),
                    model.Objects[i].numVerts * 3);
            }
        }

        // Vertex normals
        if (opt.incNormals)
        {
            val = OOF_VSHAREDNORMALS;
            fileWrite(&val, sizeof(val), 1);
            val = sizeof(float) * totalSharedVerts * 3;
            fileWrite(&val, sizeof(val), 1);
            for (i = lbound; i <= ubound; ++i)
            {
                if (model.Objects[i].includeSharedVertices)
                {
                    fileWrite(model.Objects[i].Normals, sizeof(float),
                        model.Objects[i].numVerts * 3);
                }
            }
        }

        // Texture coords
        if (model.Objects[i].textured && opt.incTexCoords)
        {
            val = OOF_VSHAREDTEXCOORDS;
            fileWrite(&val, sizeof(val), 1);
            val = sizeof(float) * totalSharedVerts * 2;
            fileWrite(&val, sizeof(val), 1);
            for (i = lbound; i <= ubound; ++i)
            {
                if (model.Objects[i].includeSharedVertices)
                {
                    fileWrite(model.Objects[i].TexCoords, sizeof(float),
                        model.Objects[i].numVerts * 2);
                }
            }
        }

        // TODO - vertex colours
    }
    for (i = lbound; i <= ubound; ++i)
    {
        // Material groups
        for (j = 0; j < model.Objects[i].numMatFaces; ++j)
        {
            val = OOF_MATERIAL_GROUP;
            fileWrite(&val, sizeof(val), 1);
            //size
            // Number of subFaces is number of INDEXES, i.e. already mult by 3
            val = sizeof(unsigned short); // material index
            val += sizeof(unsigned short); // number of faces
            val+= sizeof(unsigned short) * // 3x vertex indices, already mult by 3
                model.Objects[i].MatFaces[j].numSubFaces;
            val += sizeof(unsigned short); // number of dedicated vertices
            if (model.Objects[i].MatFaces[j].useOwnVertexList)
            {
                val += sizeof(unsigned short);    // header
                val += sizeof(float) * 3 * model.Objects[i].MatFaces[j].numOwnVertices; // own vertices
                if (opt.incNormals)
                {
                    val += sizeof(unsigned short);    // header
                    val += sizeof(float) * 3 * model.Objects[i].MatFaces[j].numOwnVertices; // own normals
                }
                if (opt.incTexCoords)
                {
                    val += sizeof(unsigned short);    // header
                    val += sizeof(float) * 2 * model.Objects[i].MatFaces[j].numOwnVertices; // own tex coords
                }
            }
            fileWrite(&val, sizeof(val), 1);
            // data
            val = model.Objects[i].MatFaces[j].MatIndex;
            fileWrite(&val, sizeof(val), 1);
            // Divide by 3 to list number of faces, not indexes
            val = model.Objects[i].MatFaces[j].numSubFaces / 3;
            cout << "Faces For Material " << model.Objects[i].MatFaces[j].MatIndex << ": " <<
                val << endl;
            fileWrite(&val, sizeof(val), 1);

            // Number of subFaces is number of INDEXES, i.e. already mult by 3
            fileWrite(model.Objects[i].MatFaces[j].subFaces,
                sizeof(unsigned short), model.Objects[i].MatFaces[j].numSubFaces);

            // Number of dedicated vertices
            val = model.Objects[i].MatFaces[j].numOwnVertices;
            fileWrite(&val, sizeof(val), 1);

            // Own vertex data if required
            if (model.Objects[i].MatFaces[j].useOwnVertexList)
            {
                cout << "This material group has it's own vertex list "
                    "of " << model.Objects[i].MatFaces[j].numOwnVertices <<
                    " vertices." << endl;
                // Vertex positions
                val = OOF_VPOSITIONS;
                fileWrite(&val, sizeof(val), 1);
                val = sizeof(float) * model.Objects[i].MatFaces[j].numOwnVertices * 3;
                fileWrite(&val, sizeof(val), 1);
                fileWrite(model.Objects[i].MatFaces[j].ownVertices, sizeof(float),
                    model.Objects[i].MatFaces[j].numOwnVertices * 3);

                // Vertex normals
                if (opt.incNormals)
                {
                    val = OOF_VNORMALS;
                    fileWrite(&val, sizeof(val), 1);
                    val = sizeof(float) * model.Objects[i].MatFaces[j].numOwnVertices * 3;
                    fileWrite(&val, sizeof(val), 1);
                    fileWrite(model.Objects[i].MatFaces[j].ownNormals, sizeof(float),
                        model.Objects[i].MatFaces[j].numOwnVertices * 3);
                }

                // Texture coords
                if (model.Objects[i].textured && opt.incTexCoords)
                {
                    val = OOF_VTEXCOORDS;
                    fileWrite(&val, sizeof(val), 1);
                    val = sizeof(float) * model.Objects[i].MatFaces[j].numOwnVertices * 2;
                    fileWrite(&val, sizeof(val), 1);
                    fileWrite(model.Objects[i].MatFaces[j].ownTexCoords, sizeof(float),
                        model.Objects[i].MatFaces[j].numOwnVertices * 2);
                }

                // TODO - vertex colours

            }
        } // next Material Group

    }

    //gzclose(gzfp);
    fclose(fp);
}

String niceBoolean(bool in)
{
    // Boolean translator for logging
    if (in)
        return "Yes";
    else
        return "No";
}
void logOptions()
{
    cout << endl << "Conversion Settings:" << endl;

    cout << "Texture Extensions: ";
    if (opt.changeTextureExtensions)
        cout << "Change to " << opt.newTextureExtension << endl;
    else
        cout << "As input file" << endl;

    cout << "Geometry data to include:" << endl;
    cout << "          Normals: " << niceBoolean(opt.incNormals) << endl;
    cout << "   Texture Coords: " << niceBoolean(opt.incTexCoords) << endl;
    cout << "   Vertex Colours: " << niceBoolean(opt.incVertColours) << endl;
    cout << " Material Colours: " << niceBoolean(!opt.resetMatColours) << endl;

    if(opt.onlyNamedObj)
        cout << "Outputting ONLY object '" << opt.objName << "'" << endl;
    else
        cout << "Outputting ALL objects" << endl;

    cout << "Use shared vertices for " << opt.shareVerticesLimit << "%+ usage" << endl;



}

void askMaterialNames(Model_3DS &model)
{
    // Prompt for a new name for each material
    char newname[64];

    outputLine();
    cout << "Please enter new names for each Material (leave blank to keep):" << endl;

    for (int i = 0; i < model.numMaterials; ++i)
    {
        cout << model.Materials[i].name << " : ";
        // Input using getLine to allow blanks
        cin.getline(newname, 64);
        if (strlen(newname) != 0)
            strcpy(model.Materials[i].name, newname);
        cout << endl;
    }

    cout << "Finished renaming materials. " << endl;
    outputLine();
}






int main(int numargs, char** args)
{
    if (numargs < 2)
    {
        help();
        return -1;
    }

    char* filename = args[1];
    // Init options
    opt.incNormals = true;
    opt.incTexCoords = true;
    opt.incVertColours = true;
    opt.onlyNamedObj = false;
    opt.shareVerticesLimit = 100;
    opt.changeTextureExtensions = false;
    opt.promptMatNames = false;
    opt.resetMatColours = true;
    opt.separateObjects = false;
    // Parse options
    for (int i = 2; i < numargs; i++)
    {
        if (stricmp(args[i], "-vn") == 0)
        {
            opt.incNormals = false;
        }
        else if (stricmp(args[i], "-tc") == 0)
        {
            opt.incTexCoords = false;
        }
        else if (stricmp(args[i], "-vc") == 0)
        {
            opt.incVertColours = false;
        }
        else if (strnicmp(args[i], "-s",2) == 0)
        {
            opt.shareVerticesLimit = atoi(&args[i][2]);
        }
        else if (strnicmp(args[i], "-o", 2) == 0)
        {
            opt.onlyNamedObj = true;
            opt.objName = args[i];
            opt.objName = opt.objName.substr(2, opt.objName.length()-2);
        }
        else if (strnicmp(args[i], "-ext", 4) == 0)
        {
            opt.changeTextureExtensions = true;
            opt.newTextureExtension = args[i];
            opt.newTextureExtension = opt.newTextureExtension.substr(4, opt.objName.length()-4);
        }
        else if (strnicmp(args[i], "-matnames", 9) == 0)
        {
            opt.promptMatNames = true;
        }
        else if (strnicmp(args[i], "-usecolours", 11) == 0)
        {
            opt.resetMatColours = false;
        }
        else if (strnicmp(args[i], "-separate", 9) == 0)
        {
            opt.separateObjects = true;
        }
    }



    Model_3DS model;
    try {

        // Log options:
        outputLine();
        cout << "Converting " << filename << endl;
        outputLine();
        logOptions();
        outputLine();
        cout << "Loading " << filename << ", please wait...." << endl;
        model.Load(filename);
        cout << "Successfully loaded " << filename << endl;
        if (opt.promptMatNames)
            askMaterialNames(model);

        if (opt.separateObjects)
        {
            // Iterate
            for (int obj = 0; obj < model.numObjects; ++obj)
            {
                Ogre::String newFile(model.Objects[obj].name);
                newFile += ".oof";
                writeOof(newFile, model, obj);
            }
        }
        else
        {
            // Determine new name
            Ogre::String newFile(filename);
            newFile = newFile.substr(0, newFile.length() - 3);
            newFile += "oof";
            writeOof(newFile, model);
        }
        outputLine();
        cout << "Completed OK." << endl;
    }
    catch (Ogre::String& s)
    {
        cout << s;
        return -1;
    }


    return 0;
}
