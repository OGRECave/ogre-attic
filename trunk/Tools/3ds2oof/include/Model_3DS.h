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
// 3DS Model file modified by Steve Streeting 2001 for OGRE
// Original credits shown below
// Modifications clearly marked, delimited by:
// ** BEGIN OGRE CHANGE
//    ...
// ** END OGRE CHANGE

//////////////////////////////////////////////////////////////////////
//
// 3D Studio Model Class
// by: Matthew Fairfax
//
// Model_3DS.h: interface for the Model_3DS class.
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

#ifndef MODEL_3DS_H
#define MODEL_3DS_H

// ** BEGIN OGRE CHANGE
// I decided to use my GLTexture class b/c adding all of its functions
// Would have greatly bloated the model class's code
// Just replace this with your favorite texture class
// #include "GLTexture.h"
// ** END OGRE CHANGE

#include <cstdio>

class Model_3DS
{
public:
    // A VERY simple vector struct
    // I could have included a complex class but I wanted the model class to stand alone
    struct Vector {
        float x;
        float y;
        float z;
    };

    // Vertex struct to make code easier to read in places
    struct Vertex {
        float x;
        float y;
        float z;
    };

    // Color struct holds the diffuse color of the material
    struct Color4i {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };

    // Holds the material info
    // TODO: add color support for non textured polys
    struct Material {
        char name[80];    // The material's name
        // ** BEGIN OGRE CHANGE
        //GLTexture tex;    // The texture (this is the only outside reference in this class)
        char texname[80];    // Texture file name
        // ** END OGRE CHANGES
        bool textured;    // whether or not it is textured
        // ** BEGIN OGRE CHANGES
        Color4i diffuse;
        Color4i ambient;
        Color4i specular;
        short shininess;    // Percentage shininess
        // ** END OGRE CHANGES
    };

    // Every chunk in the 3ds file starts with this struct
    struct ChunkHeader {
        unsigned short id;    // The chunk's id
        unsigned long  len;    // The lenght of the chunk
    };

    // I sort the mesh by material so that I won't have to switch textures a great deal
    struct MaterialFaces {
        unsigned short *subFaces;    // Index to our vertex array of all the faces that use this material
        int numSubFaces;            // The number of faces
        int MatIndex;                // An index to our materials
        // ** BEGIN OGRE CHANGES
        bool useOwnVertexList;
        int numOwnVertices;
        float* ownVertices;
        float* ownNormals;
        float* ownTexCoords;
        // ** END OGRE CHANGES
    };

    // The 3ds file can be made up of several objects
    struct Object {
        char name[80];                // The object name
        float *Vertexes;            // The array of vertices
        float *Normals;                // The array of the normals for the vertices
        float *TexCoords;            // The array of texture coordinates for the vertices
        unsigned short *Faces;        // The array of face indices
        int numFaces;                // The number of faces
        int numMatFaces;            // The number of differnet material faces
        int numVerts;                // The number of vertices
        int numTexCoords;            // The number of vertices
        bool textured;                // True: the object has textures
        MaterialFaces *MatFaces;    // The faces are divided by materials
        Vector pos;                    // The position to move the object to
        Vector rot;                    // The angles to rotate the object
        /* BEGIN OGRE CHANGES */
        Vector *FaceNormals;
        unsigned long *FaceSmoothGroups;
        float localMatrix[4][3];
        bool includeSharedVertices;
        /* END OGRE CHANGES */
    };

    char *modelname;        // The name of the model
    char *path;                // The path of the model
    int numObjects;            // Total number of objects in the model
    int numMaterials;        // Total number of materials in the model
    int totalVerts;            // Total number of vertices in the model
    int totalFaces;            // Total number of faces in the model
    bool shownormals;        // True: show the normals
    Material *Materials;    // The array of materials
    Object *Objects;        // The array of objects in the model
    Vector pos;                // The position to move the model to
    Vector rot;                // The angles to rotate the model
    float scale;            // The size you want the model scaled to
    bool lit;                // True: the model is lit
    bool visible;            // True: the model gets rendered
    void Load(char *name);    // Loads a model
    // ** BEGIN OGRE CHANGE
    //void Draw();            // Draws the model
    // ** END OGRE CHANGE
    FILE *bin3ds;            // The binary 3ds file
    Model_3DS();            // Constructor
    virtual ~Model_3DS();    // Destructor

private:
    // ** BEGIN OGRE CHANGES - support for all colour types
    void IntColorChunkProcessor(long length, long findex, int matindex, unsigned short colType);
    void FloatColorChunkProcessor(long length, long findex, int matindex, unsigned short colType);
    // ** END OGRE CHANGES
    // Processes the Main Chunk that all the other chunks exist is
    void MainChunkProcessor(long length, long findex);
        // Processes the model's info
        void EditChunkProcessor(long length, long findex);

            // Processes the model's materials
            void MaterialChunkProcessor(long length, long findex, int matindex);
                // Processes the names of the materials
                void MaterialNameChunkProcessor(long length, long findex, int matindex);
                // Processes the material's diffuse color
                //** BEGIN OGRE CHANGE
                void ColorChunkProcessor(long length, long findex, int matindex, unsigned short colType);
                // Processes material shininess
                void ShininessChunkProcessor(long length, long findex, int matindex);
                //** END OGRE CHANGE
                // Processes the material's texture maps
                void TextureMapChunkProcessor(long length, long findex, int matindex);
                    // Processes the names of the textures and load the textures
                    void MapNameChunkProcessor(long length, long findex, int matindex);

            // Processes the model's geometry
            void ObjectChunkProcessor(long length, long findex, int objindex);
                // Processes the triangles of the model
                void TriangularMeshChunkProcessor(long length, long findex, int objindex);
                    // Processes the vertices of the model and loads them
                    void VertexListChunkProcessor(long length, long findex, int objindex);
                    // Processes the texture cordiantes of the vertices and loads them
                    void TexCoordsChunkProcessor(long length, long findex, int objindex);
                    /* BEGIN OGRE CHANGES */
                    // Processes smoothing groups
                    void SmoothGroupChunkProcessor(long length, long findex, int objindex);
                    /* END OGRE CHANGES */
                    // Processes the faces of the model and loads the faces
                    void FacesDescriptionChunkProcessor(long length, long findex, int objindex);
                        // Processes the materials of the faces and splits them up by material
                        void FacesMaterialsListChunkProcessor(long length, long findex, int objindex, int subfacesindex);
                    /* BEGIN OGRE CHANGES */
                    // Processes local matrix for verts
                    void LocalCoordinatesChunkProcessor(long length, long findex, int objindex);

    // Calculates the normals of the vertices by averaging
    // the normals of the faces that use that vertex
    void CalculateNormals();
    //** BEGIN OGRE CHANGES
    // Updates the vertex data based on local transform
    void TransformLocalCoords();
    //** END OGRE CHANGES
};

#endif MODEL_3DS_H
