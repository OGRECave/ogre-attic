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
#include "OgreSubMesh.h"

#include "OgreMesh.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SubMesh::SubMesh()
    {
        useSharedVertices = true;
        useTriStrips = false;
        faceVertexIndices = 0;
        numFaces = 0;
        geometry.hasColours = false;
        geometry.hasNormals = false;
        geometry.vertexStride = 0;
        geometry.normalStride =0;
        geometry.colourStride = 0;
        geometry.numTexCoords = 1;
        geometry.numTexCoordDimensions[0] = 2;
        geometry.numVertices = 0;
        geometry.pColours = 0;
        geometry.pNormals = 0;
        for (int i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; ++i)
        {
            geometry.pTexCoords[i] = 0;
            geometry.texCoordStride[i] = 0;
        }
        geometry.pVertices = 0;

        mMatInitialised = false;

    }
    //-----------------------------------------------------------------------
    SubMesh::~SubMesh()
    {
        if (geometry.pVertices)
        {
            delete[] geometry.pVertices;
            geometry.pVertices = 0;
        }
        // Deallocate individual components if they have their own buffers
        // NB Assuming that if some components use the same buffer, all do and vice versa
        if (geometry.vertexStride == 0)
        {
            if (geometry.hasColours && geometry.pColours)
            {
                delete[] geometry.pColours;
                geometry.pColours = 0;
            }
            if (geometry.hasNormals && geometry.pNormals)
            {
                delete[] geometry.pNormals;
                geometry.pNormals = 0;
            }
            for (int i = 0; i < geometry.numTexCoords; ++i)
            {
                if (geometry.pTexCoords[i])
                {
                    delete[] geometry.pTexCoords[i];
                    geometry.pTexCoords[i] = 0;
                }
            }
        }
        if (faceVertexIndices)
        {
            delete[] faceVertexIndices;
            faceVertexIndices = 0;
        }
    }

    //-----------------------------------------------------------------------
    void SubMesh::setMaterial(const Material& mat)
    {
        mMaterial = mat;
        mMatInitialised = true;
    }
    //-----------------------------------------------------------------------
    const Material* SubMesh::getMaterial() const
    {
        return &mMaterial;
    }
    //-----------------------------------------------------------------------
    bool SubMesh::isMatInitialised(void) const
    {
        return mMatInitialised;

    }
    //-----------------------------------------------------------------------
    void SubMesh::_getRenderOperation(RenderOperation& ro) 
    {

        // SubMeshes always use indexes
        ro.useIndexes = true;
        GeometryData* geom;

        if (useTriStrips)
            ro.operationType = RenderOperation::OT_TRIANGLE_STRIP;
        else
            ro.operationType = RenderOperation::OT_TRIANGLE_LIST;

        if (useSharedVertices)
        {
            geom = &(parent->sharedGeometry);
        }
        else
        {
            geom = &(geometry);
        }

        if (geom->numTexCoords > 0)
        {
            ro.vertexOptions = RenderOperation::VO_TEXTURE_COORDS;
            ro.numTextureCoordSets = geom->numTexCoords;
            for (int tex = 0; tex < ro.numTextureCoordSets; ++tex)
            {
                ro.numTextureDimensions[tex] = geom->numTexCoordDimensions[tex];
                ro.pTexCoords[tex] = geom->pTexCoords[tex];
                ro.texCoordStride[tex] = geom->texCoordStride[tex];
            }

        }

        if (geom->hasNormals)
        {
            ro.vertexOptions |= RenderOperation::VO_NORMALS;
            ro.pNormals = geom->pNormals;
        }

        if (geom->hasColours)
        {
            ro.vertexOptions = RenderOperation::VO_DIFFUSE_COLOURS;
            ro.pDiffuseColour = geom->pColours;
        }

        ro.numVertices = geom->numVertices;
        ro.pVertices = geom->pVertices;
        ro.diffuseStride = geom->colourStride;
        ro.normalStride= geom->normalStride;
        ro.vertexStride = geom->vertexStride;

        if (useTriStrips)
            ro.numIndexes = numFaces + 2;
        else
            ro.numIndexes = numFaces * 3;

        ro.pIndexes = faceVertexIndices;
    }



}
