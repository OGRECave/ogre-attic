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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/
/* 3DS Model file loader modified by Steve Streeting 2001 for OGRE
   Original credits are shown below.
   My changes include:
        - removed the display functions, generalised to be a loader only
        - removed OpenGL dependencies
        - added proper vertex normal support using smooth groups
        - added full vertex colour support (diffuse, ambient, specular & shininess)
        - a few bugfixes


*/
//////////////////////////////////////////////////////////////////////
//
// 3D Studio Model Class
// by: Matthew Fairfax
//
// Model_3DS.cpp: implementation of the Model_3DS class.
// This is a simple class for loading and viewing
// 3D Studio model files (.3ds). It supports models
// with multiple objects. It also supports multiple
// textures per object. It does not support the animation
// for 3D Studio models b/c there are simply too many
// ways for an artist to animate a 3D Studio model and
// I didn't want to impose huge limitations on the artists.
// However, I have imposed a limitation on how the models are
// textured:
// 1) Every faces must be assigned a material
// 2) If you want the face to be textured assign the
//    texture to the Diffuse Color map
// 3) The texture must be supported by the GLTexture class
//    which only supports bitmap and targa right now
// 4) The texture must be located in the same directory as
//    the model
//
// Support for non-textured faces is done by reading the color
// from the material's diffuse color.
//
// Some models have problems loading even if you follow all of
// the restrictions I have stated and I don't know why. If you
// can import the 3D Studio file into Milkshape 3D
// (http://www.swissquake.ch/chumbalum-soft) and then export it
// to a new 3D Studio file. This seems to fix many of the problems
// but there is a limit on the number of faces and vertices Milkshape 3D
// can read.
//
// Usage:
// Model_3DS m;
//
// m.Load("model.3ds"); // Load the model
// m.Draw();            // Renders the model to the screen
//
// // If you want to show the model's normals
// m.shownormals = true;
//
// // If the model is not going to be lit then set the lit
// // variable to false. It defaults to true.
// m.lit = false;
//
// // You can disable the rendering of the model
// m.visible = false;
//
// // You can move and rotate the model like this:
// m.rot.x = 90.0f;
// m.rot.y = 30.0f;
// m.rot.z = 0.0f;
//
// m.pos.x = 10.0f;
// m.pos.y = 0.0f;
// m.pos.z = 0.0f;
//
// // If you want to move or rotate individual objects
// m.Objects[0].rot.x = 90.0f;
// m.Objects[0].rot.y = 30.0f;
// m.Objects[0].rot.z = 0.0f;
//
// m.Objects[0].pos.x = 10.0f;
// m.Objects[0].pos.y = 0.0f;
// m.Objects[0].pos.z = 0.0f;
//
//////////////////////////////////////////////////////////////////////

// This was used to generate a warning from the compiler
/*
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "
#define warn( x )  message( __FILE__LINE__ #x "\n" )
(Defined differently elsewhere by Adrian)
*/

// You need to uncomment this if you are using MFC

//#include "stdafx.h"

#include "Model_3DS.h"

#include <math.h>            // Header file for the math library
#include <string.h>
//** BEGIN OGRE CHANGE
#include <OgreString.h>
//** END OGRE CHANGE

// The chunk's id numbers
#define MAIN3DS                0x4D4D
 #define MAIN_VERS            0x0002
 #define EDIT3DS            0x3D3D
  #define MESH_VERS            0x3D3E
  #define OBJECT            0x4000
   #define TRIG_MESH        0x4100
    #define VERT_LIST        0x4110
    #define FACE_DESC        0x4120
     #define FACE_MAT        0x4130
    #define TEX_VERTS        0x4140
     #define SMOOTH_GROUP    0x4150
    #define LOCAL_COORDS    0x4160
  #define MATERIAL            0xAFFF
   #define MAT_NAME            0xA000
   #define MAT_AMBIENT        0xA010
   #define MAT_DIFFUSE        0xA020
   #define MAT_SPECULAR        0xA030
   #define SHINY_PERC        0xA040
   #define SHINY_STR_PERC    0xA041
   #define TRANS_PERC        0xA050
   #define TRANS_FOFF_PERC    0xA052
   #define REF_BLUR_PERC    0xA053
   #define RENDER_TYPE        0xA100
   #define SELF_ILLUM        0xA084
   #define MAT_SELF_ILPCT    0xA08A
   #define WIRE_THICKNESS    0xA087
   #define MAT_TEXMAP        0xA200
    #define MAT_MAPNAME        0xA300
  #define ONE_UNIT            0x0100
 #define KEYF3DS            0xB000
  #define FRAMES            0xB008
  #define MESH_INFO            0xB002
   #define HIER_POS            0xB030
   #define HIER_FATHER        0xB010
   #define PIVOT_PT            0xB013
   #define TRACK00            0xB020
   #define TRACK01            0xB021
   #define TRACK02            0xB022
#define    COLOR_RGB            0x0010
#define COLOR_TRU            0x0011
#define COLOR_TRUG            0x0012
#define COLOR_RGBG            0x0013
#define PERC_INT            0x0030
#define PERC_FLOAT            0x0031

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Model_3DS::Model_3DS()
{
    // Initialization

    // Don't show the normals by default
    shownormals = false;

    // The model is lit by default
    lit = true;

    // The model is visible by default
    visible = true;

    // Set up the default position
    pos.x = 0.0f;
    pos.y = 0.0f;
    pos.z = 0.0f;
    // Set up the default rotation
    rot.x = 0.0f;
    rot.y = 0.0f;
    rot.z = 0.0f;

    // Set up the path
    path = new char[80];
    sprintf(path, "");

    // Zero out our counters for MFC
    numObjects = 0;
    numMaterials = 0;

    // Set the scale to one
    scale = 1.0f;
}

Model_3DS::~Model_3DS()
{


}

void Model_3DS::Load(char *name)
{
    // holds the main chunk header
    ChunkHeader main;

    // strip "'s
    if (strstr(name, "\""))
        name = strtok(name, "\"");

    // Find the path
    if (strstr(name, "/") || strstr(name, "\\"))
    {
        // Holds the name of the model minus the path
        char *temp;

        // Find the name without the path
        if (strstr(name, "/"))
            temp = strrchr(name, '/');
        else
            temp = strrchr(name, '\\');

        // Allocate space for the path
        path = new char[strlen(name)-strlen(temp)+1];

        // Get a pointer to the end of the path and name
        char *src = name + strlen(name) - 1;

        // Back up until a \ or the start
        while (src != path && !((*(src-1)) == '\\' || (*(src-1)) == '/'))
            src--;

        // Copy the path into path
        memcpy (path, name, src-name);
        path[src-name] = 0;
    }

    // Load the file
    bin3ds = fopen(name,"rb");

    // ** BEGIN OGRE CHANGE
    // Check file opened ok
    if (!bin3ds)
    {
        Ogre::String s("Cannot open file ");
        s += name;
        throw s;
    }

    // Make sure we are at the beginning
    fseek(bin3ds, 0, SEEK_SET);

    // Load the Main Chunk's header
    fread(&main.id,sizeof(main.id),1,bin3ds);
    fread(&main.len,sizeof(main.len),1,bin3ds);

    // Start Processing
    MainChunkProcessor(main.len, ftell(bin3ds));

    // Don't need the file anymore so close it
    fclose(bin3ds);

    // Transform by local coords
    TransformLocalCoords();

    // Calculate the vertex normals
    CalculateNormals();

    // For future reference
    modelname = name;

    // Find the total number of faces and vertices
    totalFaces = 0;
    totalVerts = 0;

    for (int i = 0; i < numObjects; i ++)
    {
        totalFaces += Objects[i].numFaces/3;
        totalVerts += Objects[i].numVerts;
    }

    // If the object doesn't have any texcoords generate some
    for (int k = 0; k < numObjects; k++)
    {
        if (Objects[k].numTexCoords == 0)
        {
            // Set the number of texture coords
            Objects[k].numTexCoords = Objects[k].numVerts;

            // Allocate an array to hold the texture coordinates
            Objects[k].TexCoords = new float[Objects[k].numTexCoords * 2];

            // Make some texture coords
            for (int m = 0; m < Objects[k].numTexCoords; m++)
            {
                Objects[k].TexCoords[2*m] = Objects[k].Vertexes[3*m];
                Objects[k].TexCoords[2*m+1] = Objects[k].Vertexes[3*m+1];
            }
        }
    }

    // ** BEGIN OGRE CHANGE
    /*
    // Let's build simple colored textures for the materials w/o a texture
    for (int j = 0; j < numMaterials; j++)
    {
        if (Materials[j].textured == false)
        {
            unsigned char r = Materials[j].color.r;
            unsigned char g = Materials[j].color.g;
            unsigned char b = Materials[j].color.b;
            Materials[j].tex.BuildColorTexture(r, g, b);
            Materials[j].textured = true;
        }
    }
    */
    // ** END OGRE CHANGE
}

// ** BEGIN OGRE CHANGE
/*
void Model_3DS::Draw()
{
    if (visible)
    {
    glPushMatrix();

        // Move the model
        glTranslatef(pos.x, pos.y, pos.z);

        // Rotate the model
        glRotatef(rot.x, 1.0f, 0.0f, 0.0f);
        glRotatef(rot.y, 0.0f, 1.0f, 0.0f);
        glRotatef(rot.z, 0.0f, 0.0f, 1.0f);

        glScalef(scale, scale, scale);

        // Loop through the objects
        for (int i = 0; i < numObjects; i++)
        {
            // Enable texture coordiantes, normals, and vertices arrays
            if (Objects[i].textured)
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            if (lit)
                glEnableClientState(GL_NORMAL_ARRAY);
            glEnableClientState(GL_VERTEX_ARRAY);

            // Point them to the objects arrays
            if (Objects[i].textured)
                glTexCoordPointer(2, GL_FLOAT, 0, Objects[i].TexCoords);
            if (lit)
                glNormalPointer(GL_FLOAT, 0, Objects[i].Normals);
            glVertexPointer(3, GL_FLOAT, 0, Objects[i].Vertexes);

            // Loop through the faces as sorted by material and draw them
            for (int j = 0; j < Objects[i].numMatFaces; j ++)
            {
                // Use the material's texture
                Materials[Objects[i].MatFaces[j].MatIndex].tex.Use();

                glPushMatrix();

                    // Move the model
                    glTranslatef(Objects[i].pos.x, Objects[i].pos.y, Objects[i].pos.z);

                    // Rotate the model
                    //glRotatef(Objects[i].rot.x, 1.0f, 0.0f, 0.0f);
                    //glRotatef(Objects[i].rot.y, 0.0f, 1.0f, 0.0f);
                    //glRotatef(Objects[i].rot.z, 0.0f, 0.0f, 1.0f);

                    glRotatef(Objects[i].rot.z, 0.0f, 0.0f, 1.0f);
                    glRotatef(Objects[i].rot.y, 0.0f, 1.0f, 0.0f);
                    glRotatef(Objects[i].rot.x, 1.0f, 0.0f, 0.0f);

                    // Draw the faces using an index to the vertex array
                    glDrawElements(GL_TRIANGLES, Objects[i].MatFaces[j].numSubFaces, GL_UNSIGNED_SHORT, Objects[i].MatFaces[j].subFaces);

                glPopMatrix();
            }

            // Show the normals?
            if (shownormals)
            {
                // Loop through the vertices and normals and draw the normal
                for (int k = 0; k < Objects[i].numVerts * 3; k += 3)
                {
                    // Disable texturing
                    glDisable(GL_TEXTURE_2D);
                    // Disbale lighting if the model is lit
                    if (lit)
                        glDisable(GL_LIGHTING);
                    // Draw the normals blue
                    glColor3f(0.0f, 0.0f, 1.0f);

                    // Draw a line between the vertex and the end of the normal
                    glBegin(GL_LINES);
                        glVertex3f(Objects[i].Vertexes[k], Objects[i].Vertexes[k+1], Objects[i].Vertexes[k+2]);
                        glVertex3f(Objects[i].Vertexes[k]+Objects[i].Normals[k], Objects[i].Vertexes[k+1]+Objects[i].Normals[k+1], Objects[i].Vertexes[k+2]+Objects[i].Normals[k+2]);
                    glEnd();

                    // Reset the color to white
                    glColor3f(1.0f, 1.0f, 1.0f);
                    // If the model is lit then renable lighting
                    if (lit)
                        glEnable(GL_LIGHTING);
                }
            }
        }

    glPopMatrix();
    }
}
*/

void Model_3DS::CalculateNormals()
{
    /* BEGIN OGRE CHANGES */

    // Go through each face, and look for faces in the same smooth
    // group. For these, look for vertices with the same position
    // on each face (but not the same vertex index) - these are vertices
    // whose normals won't have already been averaged by the addition
    // of face normals to their vertices. This happens along a 'seam'
    // where different vertices in the same position are required (e.g.
    // for different texture coords) but where averaged normals are still
    // If we don't do this we get a hard edge where we don't want one.
    for (int objidx = 0; objidx < numObjects; ++objidx)
    {
        Object obj = Objects[objidx];
        // Skip this object if there are no smooth groups
        if (!obj.FaceSmoothGroups)
            continue;

        // NB obj.numFaces is actually number of faces * 3
        for (int face1 = 0; face1 < (obj.numFaces/3); ++face1)
        {
            for (int face2 = face1+1; face2 < (obj.numFaces/3); ++face2)
            {
                if (obj.FaceSmoothGroups[face1] ==
                    obj.FaceSmoothGroups[face2])
                {
                    // Same smooth group - check matching vertices
                    for (int vert1 = 0; vert1 < 3; vert1++)
                    {
                        for (int vert2 = 0; vert2 < 3; vert2++)
                        {
                            // Check if indexes are not the same
                            //   but position is
                            unsigned short vertIndex1 = obj.Faces[face1*3+vert1];
                            unsigned short vertIndex2 = obj.Faces[face2*3+vert2];
                            Vector vertex1, vertex2;
                            vertex1.x = obj.Vertexes[vertIndex1*3];
                            vertex1.y = obj.Vertexes[vertIndex1*3+1];
                            vertex1.z = obj.Vertexes[vertIndex1*3+2];
                            vertex2.x = obj.Vertexes[vertIndex2*3];
                            vertex2.y = obj.Vertexes[vertIndex2*3+1];
                            vertex2.z = obj.Vertexes[vertIndex2*3+2];

                            if ((vertIndex1 != vertIndex2) &&
                                (vertex1.x == vertex2.x &&
                                 vertex1.y == vertex2.y &&
                                 vertex1.z == vertex2.z))
                            {
                                // Both vertex normals must be averaged
                                // with the other face normal to apply smooth group
                                obj.Normals[vertIndex1*3]   += obj.FaceNormals[face2].x;
                                obj.Normals[vertIndex1*3+1] += obj.FaceNormals[face2].y;
                                obj.Normals[vertIndex1*3+2] += obj.FaceNormals[face2].z;

                                obj.Normals[vertIndex2*3]   += obj.FaceNormals[face1].x;
                                obj.Normals[vertIndex2*3+1] += obj.FaceNormals[face1].y;
                                obj.Normals[vertIndex2*3+2] += obj.FaceNormals[face1].z;
                            }// matched position but not index, same smooth group


                        } // vert2

                    } // vert1
                } //smoothgroup match for faces
            } // face2
        } // face1


    } // obj
    /* END OGRE CHANGES */

    // Normalise all vertex normals
    for (int i = 0; i < numObjects; i++)
    {
        for (int g = 0; g < Objects[i].numVerts; g++)
        {
            // Reduce each vert's normal to unit
            float length;
            Vector unit;

            unit.x = Objects[i].Normals[g*3];
            unit.y = Objects[i].Normals[g*3+1];
            unit.z = Objects[i].Normals[g*3+2];

            length = (float)sqrt((unit.x*unit.x) + (unit.y*unit.y) + (unit.z*unit.z));

            if (length == 0.0f)
                length = 1.0f;

            unit.x /= length;
            unit.y /= length;
            unit.z /= length;

            Objects[i].Normals[g*3]   = unit.x;
            Objects[i].Normals[g*3+1] = unit.y;
            Objects[i].Normals[g*3+2] = unit.z;
        }
    }

}

void Model_3DS::MainChunkProcessor(long length, long findex)
{
    ChunkHeader h;

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    while (ftell(bin3ds) < (findex + length - 6))
    {
        fread(&h.id,sizeof(h.id),1,bin3ds);
        fread(&h.len,sizeof(h.len),1,bin3ds);

        switch (h.id)
        {
            // This is the mesh information like vertices, faces, and materials
            case EDIT3DS    :
                EditChunkProcessor(h.len, ftell(bin3ds));
                break;
            // I left this in case anyone gets very ambitious
            case KEYF3DS    :
                //KeyFrameChunkProcessor(h.len, ftell(bin3ds));
                break;
            default            :
                break;
        }

        fseek(bin3ds, (h.len - 6), SEEK_CUR);
    }

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

void Model_3DS::EditChunkProcessor(long length, long findex)
{
    ChunkHeader h;

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    // First count the number of Objects and Materials
    while (ftell(bin3ds) < (findex + length - 6))
    {
        fread(&h.id,sizeof(h.id),1,bin3ds);
        fread(&h.len,sizeof(h.len),1,bin3ds);

        switch (h.id)
        {
            case OBJECT    :
                numObjects++;
                break;
            case MATERIAL    :
                numMaterials++;
                break;
            default            :
                break;
        }

        fseek(bin3ds, (h.len - 6), SEEK_CUR);
    }

    // Now load the materials
    if (numMaterials > 0)
    {
        Materials = new Material[numMaterials];

        // Material is set to untextured until we find otherwise
        for (int d = 0; d < numMaterials; d++)
            Materials[d].textured = false;

        fseek(bin3ds, findex, SEEK_SET);

        int i = 0;

        while (ftell(bin3ds) < (findex + length - 6))
        {
            fread(&h.id,sizeof(h.id),1,bin3ds);
            fread(&h.len,sizeof(h.len),1,bin3ds);

            switch (h.id)
            {
                case MATERIAL    :
                    MaterialChunkProcessor(h.len, ftell(bin3ds), i);
                    i++;
                    break;
                default            :
                    break;
            }

            fseek(bin3ds, (h.len - 6), SEEK_CUR);
        }
    }

    // Load the Objects (individual meshes in the whole model)
    if (numObjects > 0)
    {
        Objects = new Object[numObjects];

        // Set the textured variable to false until we find a texture
        for (int k = 0; k < numObjects; k++)
            Objects[k].textured = false;

        // Zero the objects position and rotation
        for (int m = 0; m < numObjects; m++)
        {
            Objects[m].pos.x = 0.0f;
            Objects[m].pos.y = 0.0f;
            Objects[m].pos.z = 0.0f;

            Objects[m].rot.x = 0.0f;
            Objects[m].rot.y = 0.0f;
            Objects[m].rot.z = 0.0f;
        }

        // Zero out the number of texture coords
        for (int n = 0; n < numObjects; n++)
            Objects[n].numTexCoords = 0;

        fseek(bin3ds, findex, SEEK_SET);

        int j = 0;

        while (ftell(bin3ds) < (findex + length - 6))
        {
            fread(&h.id,sizeof(h.id),1,bin3ds);
            fread(&h.len,sizeof(h.len),1,bin3ds);

            switch (h.id)
            {
                case OBJECT    :
                    ObjectChunkProcessor(h.len, ftell(bin3ds), j);
                    j++;
                    break;
                default            :
                    break;
            }

            fseek(bin3ds, (h.len - 6), SEEK_CUR);
        }
    }

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

void Model_3DS::MaterialChunkProcessor(long length, long findex, int matindex)
{
    ChunkHeader h;

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    while (ftell(bin3ds) < (findex + length - 6))
    {
        fread(&h.id,sizeof(h.id),1,bin3ds);
        fread(&h.len,sizeof(h.len),1,bin3ds);

        switch (h.id)
        {
            case MAT_NAME    :
                // Loads the material's names
                MaterialNameChunkProcessor(h.len, ftell(bin3ds), matindex);
                break;
            // ** BEGIN OGRE CHANGES
            case MAT_AMBIENT    :
            case MAT_DIFFUSE    :
            case MAT_SPECULAR    :
                ColorChunkProcessor(h.len, ftell(bin3ds), matindex, h.id);
                break;
            // ** END OGRE CHANGES
            case MAT_TEXMAP    :
                // Finds the names of the textures of the material and loads them
                TextureMapChunkProcessor(h.len, ftell(bin3ds), matindex);
                break;
            //** BEGIN OGRE CHANGE
            case SHINY_PERC:
                ShininessChunkProcessor(h.len, ftell(bin3ds), matindex);
                break;
            //** END OGRE CHANGE
            default            :
                break;
        }

        fseek(bin3ds, (h.len - 6), SEEK_CUR);
    }

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

void Model_3DS::MaterialNameChunkProcessor(long length, long findex, int matindex)
{
    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    // Read the material's name
    for (int i = 0; i < 80; i++)
    {

        Materials[matindex].name[i] = fgetc(bin3ds);
        if (Materials[matindex].name[i] == 0)
        {
            Materials[matindex].name[i] = NULL;
            break;
        }
    }

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

//** BEGIN OGRE CHANGES (generalised to all colour not just diffuse)
void Model_3DS::ColorChunkProcessor(long length, long findex, int matindex, unsigned short colType)
{
    ChunkHeader h;

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    while (ftell(bin3ds) < (findex + length - 6))
    {
        fread(&h.id,sizeof(h.id),1,bin3ds);
        fread(&h.len,sizeof(h.len),1,bin3ds);

        // Determine the format of the color and load it
        switch (h.id)
        {
            case COLOR_RGB    :
                // A rgb float color chunk
                FloatColorChunkProcessor(h.len, ftell(bin3ds), matindex, colType);
                break;
            case COLOR_TRU    :
                // A rgb int color chunk
                IntColorChunkProcessor(h.len, ftell(bin3ds), matindex, colType);
                break;
            case COLOR_RGBG    :
                // A rgb gamma corrected float color chunk
                FloatColorChunkProcessor(h.len, ftell(bin3ds), matindex, colType);
                break;
            case COLOR_TRUG    :
                // A rgb gamma corrected int color chunk
                IntColorChunkProcessor(h.len, ftell(bin3ds), matindex, colType);
                break;
            default            :
                break;
        }

        fseek(bin3ds, (h.len - 6), SEEK_CUR);
    }

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}
// ** END OGRE CHANGES

//** BEGIN OGRE CHANGES
void Model_3DS::ShininessChunkProcessor(long length, long findex, int matindex)
{
    ChunkHeader h;

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    fread(&h.id,sizeof(h.id),1,bin3ds);
    fread(&h.len,sizeof(h.len),1,bin3ds);

    // Determine type of percentage
    if (h.id == PERC_INT)
    {
        fread(&(Materials[matindex].shininess), sizeof(short),1, bin3ds);
    }
    else if (h.id == PERC_FLOAT)
    {
        float floatPerc;
        fread(&floatPerc, sizeof(float),1, bin3ds);
        Materials[matindex].shininess = (short)floatPerc;
    }


    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);


}
//** END OGRE CHANGES


// ** BEGIN OGRE CHANGES
// Generalised to handle diffuse, abient and specular
// Also bugfixed - original was using red for all components!
void Model_3DS::FloatColorChunkProcessor(long length, long findex, int matindex, unsigned short colType)
{
    float r;
    float g;
    float b;

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    fread(&r,sizeof(r),1,bin3ds);
    fread(&g,sizeof(g),1,bin3ds);
    fread(&b,sizeof(b),1,bin3ds);

    if (colType == MAT_AMBIENT)
    {
        Materials[matindex].ambient.r = (unsigned char)(r*255.0f);
        Materials[matindex].ambient.g = (unsigned char)(g*255.0f);
        Materials[matindex].ambient.b = (unsigned char)(b*255.0f);
        Materials[matindex].ambient.a = 255;
    }
    else if (colType == MAT_DIFFUSE)
    {
        Materials[matindex].diffuse.r = (unsigned char)(r*255.0f);
        Materials[matindex].diffuse.g = (unsigned char)(g*255.0f);
        Materials[matindex].diffuse.b = (unsigned char)(b*255.0f);
        Materials[matindex].diffuse.a = 255;
    }
    else if (colType == MAT_SPECULAR)
    {
        Materials[matindex].specular.r = (unsigned char)(r*255.0f);
        Materials[matindex].specular.g = (unsigned char)(g*255.0f);
        Materials[matindex].specular.b = (unsigned char)(b*255.0f);
        Materials[matindex].specular.a = 255;
    }


    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}
// ** END OGRE CHANGES

// ** BEGIN OGRE CHANGES
// Generalised to handle diffuse, abient and specular
void Model_3DS::IntColorChunkProcessor(long length, long findex, int matindex, unsigned short colType)
{
    unsigned char r;
    unsigned char g;
    unsigned char b;

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    fread(&r,sizeof(r),1,bin3ds);
    fread(&g,sizeof(g),1,bin3ds);
    fread(&b,sizeof(b),1,bin3ds);

    if (colType == MAT_AMBIENT)
    {
        Materials[matindex].ambient.r = r;
        Materials[matindex].ambient.g = g;
        Materials[matindex].ambient.b = b;
        Materials[matindex].ambient.a = 255;
    }
    else if (colType == MAT_DIFFUSE)
    {
        Materials[matindex].diffuse.r = r;
        Materials[matindex].diffuse.g = g;
        Materials[matindex].diffuse.b = b;
        Materials[matindex].diffuse.a = 255;
    }
    else if (colType == MAT_SPECULAR)
    {
        Materials[matindex].specular.r = r;
        Materials[matindex].specular.g = g;
        Materials[matindex].specular.b = b;
        Materials[matindex].specular.a = 255;
    }
    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}
// ** END OGRE CHANGES

void Model_3DS::TextureMapChunkProcessor(long length, long findex, int matindex)
{
    ChunkHeader h;

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    while (ftell(bin3ds) < (findex + length - 6))
    {
        fread(&h.id,sizeof(h.id),1,bin3ds);
        fread(&h.len,sizeof(h.len),1,bin3ds);

        switch (h.id)
        {
            case MAT_MAPNAME:
                // Read the name of texture in the Diffuse Color map
                MapNameChunkProcessor(h.len, ftell(bin3ds), matindex);
                break;
            default            :
                break;
        }

        fseek(bin3ds, (h.len - 6), SEEK_CUR);
    }

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

void Model_3DS::MapNameChunkProcessor(long length, long findex, int matindex)
{
    char name[80];

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    // Read the name of the texture
    for (int i = 0; i < 80; i++)
    {
        name[i] = fgetc(bin3ds);
        if (name[i] == 0)
        {
            name[i] = NULL;
            break;
        }
    }

    // ** BEGIN OGRE CHANGE
    // Just copy texture name, don't use texture loader
    /*
    // Load the name and indicate that the material has a texture
    char fullname[80];
    sprintf(fullname, "%s%s", path, name);
    Materials[matindex].tex.Load(fullname);
    */
    strcpy(Materials[matindex].texname, name);
    // ** END OGRE CHANGE
    Materials[matindex].textured = true;

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

void Model_3DS::ObjectChunkProcessor(long length, long findex, int objindex)
{
    ChunkHeader h;
    int i;

	// move the file pointer to the beginning of the main
	// chunk's data findex + the size of the header
	fseek(bin3ds, findex, SEEK_SET);

	// Load the object's name
	for (i = 0; i < 80; i++)
	{
		Objects[objindex].name[i] = fgetc(bin3ds);
		if (Objects[objindex].name[i] == 0)
		{
			Objects[objindex].name[i] = NULL;
			break;
		}
	}

	//** BEGIN OGRE CHANGES
    // Init
    Objects[objindex].numVerts = 0;
    Objects[objindex].numFaces = 0;
    Objects[objindex].numMatFaces = 0;
    Objects[objindex].numTexCoords = 0;
    Objects[objindex].FaceSmoothGroups = 0;

	// Init local matrix
	for (int row = 0; row < 4; ++row)
		for (int col = 0; col < 3; ++col)
			Objects[objindex].localMatrix[row][col] = 0.0;

	Objects[objindex].localMatrix[0][0] = Objects[objindex].localMatrix[1][1] = 
		Objects[objindex].localMatrix[2][2] = 1.0;

	//** END OGRE CHANGES

	while (ftell(bin3ds) < (findex + length - 6))
	{
		fread(&h.id,sizeof(h.id),1,bin3ds);
		fread(&h.len,sizeof(h.len),1,bin3ds);

		switch (h.id)
		{
			case TRIG_MESH	:
				// Process the triangles of the object
				TriangularMeshChunkProcessor(h.len, ftell(bin3ds), objindex);
				break;
			default			:
				break;
		}

		fseek(bin3ds, (h.len - 6), SEEK_CUR);
	}

	// move the file pointer back to where we got it so
	// that the ProcessChunk() which we interrupted will read
	// from the right place
	fseek(bin3ds, findex, SEEK_SET);
}

void Model_3DS::TriangularMeshChunkProcessor(long length, long findex, int objindex)
{
    ChunkHeader h;

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    while (ftell(bin3ds) < (findex + length - 6))
    {
        fread(&h.id,sizeof(h.id),1,bin3ds);
        fread(&h.len,sizeof(h.len),1,bin3ds);

        switch (h.id)
        {
            case VERT_LIST    :
                // Load the vertices of the onject
                VertexListChunkProcessor(h.len, ftell(bin3ds), objindex);
                break;
            case LOCAL_COORDS    :
                LocalCoordinatesChunkProcessor(h.len, ftell(bin3ds), objindex);
                break;
            case TEX_VERTS    :
                // Load the texture coordinates for the vertices
                TexCoordsChunkProcessor(h.len, ftell(bin3ds), objindex);
                Objects[objindex].textured = true;
                break;

            default            :
                break;
        }

        fseek(bin3ds, (h.len - 6), SEEK_CUR);
    }

    // After we have loaded the vertices we can load the faces
    fseek(bin3ds, findex, SEEK_SET);

    while (ftell(bin3ds) < (findex + length - 6))
    {
        fread(&h.id,sizeof(h.id),1,bin3ds);
        fread(&h.len,sizeof(h.len),1,bin3ds);

        switch (h.id)
        {
            case FACE_DESC    :
                // Load the faces of the object
                FacesDescriptionChunkProcessor(h.len, ftell(bin3ds), objindex);
                break;
            default            :
                break;
        }

        fseek(bin3ds, (h.len - 6), SEEK_CUR);
    }

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

void Model_3DS::VertexListChunkProcessor(long length, long findex, int objindex)
{
    unsigned short numVerts;

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    // Read the number of vertices of the object
    fread(&numVerts,sizeof(numVerts),1,bin3ds);

    // Allocate arrays for the vertices and normals
    Objects[objindex].Vertexes = new float[numVerts * 3];
    Objects[objindex].Normals = new float[numVerts * 3];

    // Assign the number of vertices for future use
    Objects[objindex].numVerts = numVerts;

    // Zero out the normals array
    for (int j = 0; j < numVerts * 3; j++)
        Objects[objindex].Normals[j] = 0.0f;

    for (int i = 0; i < numVerts * 3; i+=3)
    {
        fread(&Objects[objindex].Vertexes[i],sizeof(float),1,bin3ds);
        fread(&Objects[objindex].Vertexes[i+1],sizeof(float),1,bin3ds);
        fread(&Objects[objindex].Vertexes[i+2],sizeof(float),1,bin3ds);

    }

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

void Model_3DS::TexCoordsChunkProcessor(long length, long findex, int objindex)
{
    // The number of texture coordinates
    unsigned short numCoords;

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    // Read the number of coordinates
    fread(&numCoords,sizeof(numCoords),1,bin3ds);

    // Allocate an array to hold the texture coordinates
    Objects[objindex].TexCoords = new float[numCoords * 2];

    // Set the number of texture coords
    Objects[objindex].numTexCoords = numCoords;

    // Read teh texture coordiantes into the array
    for (int i = 0; i < numCoords * 2; i+=2)
    {
        fread(&Objects[objindex].TexCoords[i],sizeof(float),1,bin3ds);
        fread(&Objects[objindex].TexCoords[i+1],sizeof(float),1,bin3ds);
        // BEGIN OGRE CHANGE
        // 3DS uses a V co-ordinate which is at the bottom of the map at 0 rather than the top
        // Dunno why since everybody else says 0.0 is at the top!
        // Reverse this
        //Objects[objindex].TexCoords[i+1] = 1.0 - Objects[objindex].TexCoords[i+1];

    }

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

void Model_3DS::FacesDescriptionChunkProcessor(long length, long findex, int objindex)
{
    ChunkHeader h;
    unsigned short numFaces;    // The number of faces in the object
    unsigned short vertA;        // The first vertex of the face
    unsigned short vertB;        // The second vertex of the face
    unsigned short vertC;        // The third vertex of the face
    unsigned short flags;        // The winding order flags
    long subs;                    // Holds our place in the file
    int numMatFaces = 0;        // The number of different materials

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    // Read the number of faces
    fread(&numFaces,sizeof(numFaces),1,bin3ds);

    // Allocate an array to hold the faces
    Objects[objindex].Faces = new unsigned short[numFaces * 3];
    /* BEGIN OGRE CHANGES */
    // Allocate memory for face normals
    Objects[objindex].FaceNormals = new Vector[numFaces];
    /* END OGRE CHANGES */
    // Store the number of faces
    Objects[objindex].numFaces = numFaces * 3;

    // Read the faces into the array
    for (int i = 0; i < numFaces * 3; i+=3)
    {
        // Read the vertices of the face
        fread(&vertA,sizeof(vertA),1,bin3ds);
        fread(&vertB,sizeof(vertB),1,bin3ds);
        fread(&vertC,sizeof(vertC),1,bin3ds);
        fread(&flags,sizeof(flags),1,bin3ds);

        // Place them in the array
        Objects[objindex].Faces[i]   = vertA;
        Objects[objindex].Faces[i+1] = vertB;
        Objects[objindex].Faces[i+2] = vertC;

        // Calculate the face's normal
        Vector n;
        Vertex v1;
        Vertex v2;
        Vertex v3;

        v1.x = Objects[objindex].Vertexes[vertA*3];
        v1.y = Objects[objindex].Vertexes[vertA*3+1];
        v1.z = Objects[objindex].Vertexes[vertA*3+2];
        v2.x = Objects[objindex].Vertexes[vertB*3];
        v2.y = Objects[objindex].Vertexes[vertB*3+1];
        v2.z = Objects[objindex].Vertexes[vertB*3+2];
        v3.x = Objects[objindex].Vertexes[vertC*3];
        v3.y = Objects[objindex].Vertexes[vertC*3+1];
        v3.z = Objects[objindex].Vertexes[vertC*3+2];

        // calculate the normal
        float u[3], v[3];

        // V2 - V3;
        u[0] = v2.x - v3.x;
        u[1] = v2.y - v3.y;
        u[2] = v2.z - v3.z;

        // V2 - V1;
        v[0] = v2.x - v1.x;
        v[1] = v2.y - v1.y;
        v[2] = v2.z - v1.z;

        n.x = (u[1]*v[2] - u[2]*v[1]);
        n.y = (u[2]*v[0] - u[0]*v[2]);
        n.z = (u[0]*v[1] - u[1]*v[0]);

        /* BEGIN OGRE CHANGES */
        //  Store the face normal for this purpose for later use in the
        //  CalclateNormals() routine
        //  Note this is not normalised yet - this is OK
        Objects[objindex].FaceNormals[(int)(i/3)] = n;
        // Add this normal to its verts' normals
        Objects[objindex].Normals[vertA*3]   += n.x;
        Objects[objindex].Normals[vertA*3+1] += n.y;
        Objects[objindex].Normals[vertA*3+2] += n.z;
        Objects[objindex].Normals[vertB*3]   += n.x;
        Objects[objindex].Normals[vertB*3+1] += n.y;
        Objects[objindex].Normals[vertB*3+2] += n.z;
        Objects[objindex].Normals[vertC*3]   += n.x;
        Objects[objindex].Normals[vertC*3+1] += n.y;
        Objects[objindex].Normals[vertC*3+2] += n.z;
        /* END OGRE CHANGES */
    }

    // Store our current file position
    subs = ftell(bin3ds);

    // Check to see how many materials the faces are split into
    while (ftell(bin3ds) < (findex + length - 6))
    {
        fread(&h.id,sizeof(h.id),1,bin3ds);
        fread(&h.len,sizeof(h.len),1,bin3ds);

        switch (h.id)
        {
            case FACE_MAT    :
                //FacesMaterialsListChunkProcessor(h.len, ftell(bin3ds), objindex);
                numMatFaces++;
                break;
            /* BEGIN OGRE CHANGE */
            // Deal with smoothing groups
            case SMOOTH_GROUP:
                SmoothGroupChunkProcessor(h.len, ftell(bin3ds),objindex);
                break;
            /* END OGRE CHANGE */
            default            :
                break;
        }

        fseek(bin3ds, (h.len - 6), SEEK_CUR);
    }

    // Split the faces up according to their materials
    if (numMatFaces > 0)
    {
        // Allocate an array to hold the lists of faces divided by material
        Objects[objindex].MatFaces = new MaterialFaces[numMatFaces];
        // Store the number of material faces
        Objects[objindex].numMatFaces = numMatFaces;

        // Init num subfaces
        for (i = 0; i < numMaterials; ++i)
        {
            Objects[objindex].MatFaces[i].numSubFaces = 0;
        }

        fseek(bin3ds, subs, SEEK_SET);

        int j = 0;

        // Split the faces up
        while (ftell(bin3ds) < (findex + length - 6))
        {
            fread(&h.id,sizeof(h.id),1,bin3ds);
            fread(&h.len,sizeof(h.len),1,bin3ds);

            switch (h.id)
            {
                case FACE_MAT    :
                    // Process the faces and split them up
                    FacesMaterialsListChunkProcessor(h.len, ftell(bin3ds), objindex, j);
                    j++;
                    break;
                default            :
                    break;
            }

            fseek(bin3ds, (h.len - 6), SEEK_CUR);
        }
    }

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

void Model_3DS::FacesMaterialsListChunkProcessor(long length, long findex, int objindex, int subfacesindex)
{
    char name[80];                // The material's name
    unsigned short numEntries;    // The number of faces associated with this material
    unsigned short Face;        // Holds the faces as they are read
    int material;                // An index to the Materials array for this material

    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    // Read the material's name
    for (int i = 0; i < 80; i++)
    {
        name[i] = fgetc(bin3ds);
        if (name[i] == 0)
        {
            name[i] = NULL;
            break;
        }
    }

    // Faind the material's index in the Materials array
    for (material = 0; material < numMaterials; material++)
    {
        if (strcmp(name, Materials[material].name) == 0)
            break;
    }

    // Store this value for later so that we can find the material
    Objects[objindex].MatFaces[subfacesindex].MatIndex = material;

    // Read the number of faces associated with this material
    fread(&numEntries,sizeof(numEntries),1,bin3ds);

    // Check if already some faces for this material
    int startIdx;
    if (Objects[objindex].MatFaces[subfacesindex].numSubFaces > 0)
    {
        // reallocate memory
        unsigned short* oldPtr = Objects[objindex].MatFaces[subfacesindex].subFaces;
        Objects[objindex].MatFaces[subfacesindex].subFaces = new unsigned short[numEntries * 3];
        memcpy(Objects[objindex].MatFaces[subfacesindex].subFaces, oldPtr, 
            sizeof(unsigned short) * Objects[objindex].MatFaces[subfacesindex].numSubFaces);
        // Delete old data
        delete [] oldPtr;

        startIdx = Objects[objindex].MatFaces[subfacesindex].numSubFaces;

        Objects[objindex].MatFaces[subfacesindex].numSubFaces += numEntries * 3;

    }
    else
    {
        // Allocate an array to hold the list of faces associated with this material
        Objects[objindex].MatFaces[subfacesindex].subFaces = new unsigned short[numEntries * 3];
        // Store this number for later use
        Objects[objindex].MatFaces[subfacesindex].numSubFaces = numEntries * 3;
        startIdx = 0;
    }
    

    // Read the faces into the array
    for (i = 0; i < numEntries * 3; startIdx+=3, i+=3)
    {
        // read the face
        fread(&Face,sizeof(Face),1,bin3ds);
        // Add the face's vertices to the list
        Objects[objindex].MatFaces[subfacesindex].subFaces[startIdx] = Objects[objindex].Faces[Face * 3];
        Objects[objindex].MatFaces[subfacesindex].subFaces[startIdx+1] = Objects[objindex].Faces[Face * 3 + 1];
        Objects[objindex].MatFaces[subfacesindex].subFaces[startIdx+2] = Objects[objindex].Faces[Face * 3 + 2];
    }

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

/* BEGIN OGRE CHANGES */
void Model_3DS::SmoothGroupChunkProcessor(long length, long findex, int objindex)
{
    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    // For this object, read in the smooth group for each face
    // Reserve memory first
    Objects[objindex].FaceSmoothGroups = new unsigned long[Objects[objindex].numFaces];

    fread(Objects[objindex].FaceSmoothGroups, sizeof(unsigned long), Objects[objindex].numFaces, bin3ds);

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);

}

void Model_3DS::LocalCoordinatesChunkProcessor(long length, long findex, int objindex)
{
    // move the file pointer to the beginning of the main
    // chunk's data findex + the size of the header
    fseek(bin3ds, findex, SEEK_SET);

    fread(Objects[objindex].localMatrix, sizeof(float), 12, bin3ds);

    // move the file pointer back to where we got it so
    // that the ProcessChunk() which we interrupted will read
    // from the right place
    fseek(bin3ds, findex, SEEK_SET);
}

void Model_3DS::TransformLocalCoords()
{

    // Transform by matrix
    // Note that 3DS matrix is COLUMN major not row major
    for (int objidx = 0; objidx < numObjects; ++objidx)
    {
        Object obj = Objects[objidx];

        for (int vert = 0; vert < obj.numVerts*3; vert+=3)
        {

            // Assumed w value of vector is 1.0
            // w result is discarded
            // Copy current value
            Vertex orig;
            orig.x = obj.Vertexes[vert];
            orig.y = obj.Vertexes[vert+1];
            orig.z = obj.Vertexes[vert+2];

            for (int iCol = 0; iCol < 3; ++iCol)
            {
                obj.Vertexes[vert+iCol] =
                    obj.localMatrix[iCol][0] * orig.x +
                    obj.localMatrix[iCol][1] * orig.y +
                    obj.localMatrix[iCol][2] * orig.z +
                    obj.localMatrix[iCol][3] * 1.0;
            }
        }
    }

}
/* END OGRE CHANGES */
