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
#include "OgreBspLevel.h"
#include "OgreQuake3Level.h"
#include "OgreBspResourceManager.h"
#include "OgreException.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"
#include "OgrePatchSurface.h"
#include "OgreQuake3ShaderManager.h"
#include "OgreQuake3Shader.h"
#include "OgreMath.h"
#include "OgreStringVector.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"


namespace Ogre {


    //-----------------------------------------------------------------------
    BspLevel::BspLevel(String name)
    {
        mName = name;
        mRootNode = 0;
        mBrushes = 0;
    }

    //-----------------------------------------------------------------------
    BspLevel::~BspLevel()
    {
        if (mIsLoaded)
        {
            unload();
            mIsLoaded = false;
        }

    }

    //-----------------------------------------------------------------------
    void BspLevel::load()
    {
        // Use Quake3 file loader
        Quake3Level q3;
        DataChunk chunk;
        BspResourceManager::getSingleton()._findResourceData(mName, chunk);

        q3.loadFromChunk(chunk);

        loadQuake3Level(q3);

        chunk.clear();

    }

    //-----------------------------------------------------------------------
    void BspLevel::unload()
    {
        delete mVertexData;
        delete mIndexData;
        delete [] mFaceGroups;
        delete [] mLeafFaceGroups;
        delete [] mRootNode;
        delete [] mVisData.tableData;
        delete [] mBrushes;

        mRootNode = 0;
        mFaceGroups = 0;
        mLeafFaceGroups = 0;
        mBrushes = 0;
    }

    //-----------------------------------------------------------------------
    void BspLevel::loadQuake3Level(const Quake3Level& q3lvl)
    {
        SceneManager* sm = SceneManagerEnumerator::getSingleton().getSceneManager(ST_INTERIOR);

        loadEntities(q3lvl);

        // Parse shaders
        Quake3ShaderManager::getSingleton().parseAllSources(".shader");
        // Extract lightmaps into textures
        q3lvl.extractLightmaps();

        //-----------------------------------------------------------------------
        // Vertices
        //-----------------------------------------------------------------------
        // Allocate memory for vertices & copy
        mVertexData = new VertexData();

        /// Create vertex declaration
        VertexDeclaration* decl = mVertexData->vertexDeclaration;
        size_t offset = 0;
        decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
        decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
        decl->addElement(0, offset, VET_COLOUR, VES_DIFFUSE);
        offset += VertexElement::getTypeSize(VET_COLOUR);
        decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
        offset += VertexElement::getTypeSize(VET_FLOAT2);
        decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1);
        /// Create the vertex buffer
        HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton()
            .createVertexBuffer(
                sizeof(BspVertex), 
                q3lvl.mNumVertices, 
                HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        /* COPY - Note that we can't just block-copy the vertex data because we have to reorder
            our vertex elements; this is to ensure compatibility with older cards when using
            hardware vertex buffers - Direct3D requires that the buffer format maps onto a
            FVF in those older drivers. 
        */
        BspVertex* pVert = static_cast<BspVertex*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD) );
        // Keep another base pointer for use later in patch building
        BspVertex* pBaseVert = pVert;
        for (int v = 0; v < q3lvl.mNumVertices; ++v)
        {
            memcpy(pVert[v].position, q3lvl.mVertices[v].point, sizeof(Real) * 3);
            memcpy(pVert[v].normal, q3lvl.mVertices[v].normal,  sizeof(Real) * 3);
            pVert[v].colour = q3lvl.mVertices[v].color;
            // Correct texture coords
            // Coords are flipped in Y axis!
            // ---WHY???---
            pVert[v].texcoords[0]  = q3lvl.mVertices[v].texture[0];
            pVert[v].texcoords[1]  = 1 - q3lvl.mVertices[v].texture[1];
            pVert[v].lightmap[0]  = q3lvl.mVertices[v].lightmap[0];
            pVert[v].lightmap[1]  = 1 - q3lvl.mVertices[v].lightmap[1];
        }
        // IMPORTANT: leave the buffer locked for the moment since we use it
        // later on in patch building
        //vbuf->unlock();
        // Setup binding
        mVertexData->vertexBufferBinding->setBinding(0, vbuf);
        // Set other data
        // Note that vertexStart and vertexCount will both be overridden per 
        // rendering subset when rendering for real
        // Just initialised to the whole buffer for fun really :)
        mVertexData->vertexStart = 0;
        mVertexData->vertexCount = q3lvl.mNumVertices;

        //-----------------------------------------------------------------------
        // Faces
        // --------
        mNumLeafFaceGroups = q3lvl.mNumLeafFaces;
        mLeafFaceGroups = new int[mNumLeafFaceGroups];
        memcpy(mLeafFaceGroups, q3lvl.mLeafFaces, sizeof(int)*mNumLeafFaceGroups);
        mNumFaceGroups = q3lvl.mNumFaces;
        mFaceGroups = new StaticFaceGroup[mNumFaceGroups];
        // Set up index buffer
        // NB Quake3 indexes are 32-bit
        mIndexData = new IndexData();
        mIndexData->indexBuffer = HardwareBufferManager::getSingleton()
            .createIndexBuffer(
                HardwareIndexBuffer::IT_32BIT, 
                q3lvl.mNumElements, 
                HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        mIndexData->indexBuffer->writeData(
            0, mIndexData->indexBuffer->getSizeInBytes(), q3lvl.mElements, true);
        // NB indexStart / indexCount will be overridden later based on rendering subsets
        // Just initialised to the whole buffer for fun really :)
        mIndexData->indexStart = 0;
        mIndexData->indexCount = q3lvl.mNumElements;

        //-----------------------------------------------------------------------
        // Create materials for shaders
        //-----------------------------------------------------------------------
        // NB this only works for the 'default' shaders for now
        //  i.e. those that don't have a .shader script and thus default
        //  to just texture + lightmap
        // TODO: pre-parse all .shader files and create lookup for next stage (use ROGL shader_file_t)

        // Material names are shadername#lightmapnumber
        // This is because I like to define materials up front completely
        //  rather than combine lightmap and shader dynamically (it's
        //  more generic). It results in more materials, but they're small
        //  beer anyway. Texture duplication is prevented by infrastructure.
        // To do this I actually need to parse the faces since they have the
        //  shader/lightmap combo (lightmap number is not in the shader since
        //  it can be used with multiple lightmaps)
        char shaderName[72];
        int face;
        face = q3lvl.mNumFaces;
        int matHandle;
        String meshName;

        while(face--)
        {

            // Check to see if existing material
            // Format shader#lightmap
            int shadIdx = q3lvl.mFaces[face].shader;
            sprintf(shaderName, "%s#%d",
                q3lvl.mShaders[shadIdx].name,
                q3lvl.mFaces[face].lm_texture);

            Material *shadMat = sm->getMaterial(shaderName);
            if (shadMat == 0)
            {
                // Build new material
                Material mat(shaderName);

                // Colour layer
                // NB no extension in Q3A(doh), have to try shader, .jpg, .tga
                String tryName = q3lvl.mShaders[shadIdx].name;
                // Try shader first
                Quake3Shader* pShad = (Quake3Shader*)Quake3ShaderManager::getSingleton().getByName(tryName);
                if (pShad)
                {
                    shadMat = pShad->createAsMaterial(sm, q3lvl.mFaces[face].lm_texture);
                    matHandle = shadMat->getHandle();
                }
                else
                {
                    // No shader script, try default type texture
                    // Try jpg
                    Material::TextureLayer* tex = mat.addTextureLayer(tryName + ".jpg");
                    if (tex->isBlank())
                    {
                        // Try tga
                        tex->setTextureName(tryName + ".tga");
                    }
                    // Set replace on all first layer textures for now
                    tex->setColourOperation(LBO_REPLACE);
                    tex->setTextureAddressingMode(Material::TextureLayer::TAM_WRAP);

                    if (q3lvl.mFaces[face].lm_texture != -1)
                    {
                        // Add lightmap, additive blending
                        char lightmapName[16];
                        sprintf(lightmapName, "@lightmap%d",q3lvl.mFaces[face].lm_texture);
                        tex = mat.addTextureLayer(lightmapName);
                        // Blend
                        tex->setColourOperation(LBO_MODULATE);
                        // Use 2nd texture co-ordinate set
                        tex->setTextureCoordSet(1);
                        // Clamp
                        tex->setTextureAddressingMode(Material::TextureLayer::TAM_CLAMP);

                    }
                    // Set culling mode to none
                    mat.setCullingMode(CULL_NONE);
                    // No dynamic lighting
                    mat.setLightingEnabled(false);

                    // Register material
                    shadMat = MaterialManager::getSingleton().add(mat);
                    matHandle = shadMat->getHandle();
                }
            }
            else
            {
                matHandle = shadMat->getHandle();
            }

            // Copy face data
            StaticFaceGroup* dest = &mFaceGroups[face];
            bsp_face_t* src = &q3lvl.mFaces[face];

            if (q3lvl.mShaders[src->shader].surface_flags & SURF_SKY)
            {
                dest->isSky = true;
            }
            else
            {
                dest->isSky = false;
            }


            dest->materialHandle = matHandle;
            dest->elementStart = src->elem_start;
            dest->numElements = src->elem_count;
            dest->numVertices = src->vert_count;
            dest->vertexStart = src->vert_start;
            if (src->type == BSP_FACETYPE_NORMAL)
            {
                dest->fType = FGT_FACE_LIST;
                // Assign plane
                dest->plane.normal = Vector3(src->normal);
                dest->plane.d = -dest->plane.normal.dotProduct(Vector3(src->org));
            }
            else if (src->type == BSP_FACETYPE_PATCH)
            {
                // Seems to be some crap in the Q3 level where vertex count = 0 or num control points = 0?
                if (dest->numVertices == 0 || src->mesh_cp[0] == 0)
                {
                    dest->fType = FGT_UNKNOWN;
                }
                else
                {

                    // Set up patch surface
                    dest->fType = FGT_PATCH;
                    dest->patchSurf = new PatchSurface();
                    // Set up control points & format
                    // Same format as in BspSceneManager::mPendingGeometry - see that for details
                    // Reuse the declaration from mVertexData
                    BspVertex* pPatchVertex = pBaseVert + dest->vertexStart;
                    meshName = "BspBezierPatch" + StringConverter::toString(face);
                    // For the moment, just define 1 level of subdivision ie AUTO_LEVEL
                    dest->patchSurf->defineSurface(
                        meshName, 
                        pPatchVertex, 
                        mVertexData->vertexDeclaration, 
                        src->mesh_cp[0],
                        src->mesh_cp[1],
                        PatchSurface::PST_BEZIER);
                    // Build the patch
                    dest->patchSurf->build();
                }


            }


        }

        // unlock vertex buffer
        vbuf->unlock();

        //-----------------------------------------------------------------------
        // Nodes
        //-----------------------------------------------------------------------
        // Allocate memory for all nodes (leaves and splitters)
        mNumNodes = q3lvl.mNumNodes + q3lvl.mNumLeaves;
        mNumLeaves = q3lvl.mNumLeaves;
        mLeafStart = q3lvl.mNumNodes;
        mRootNode = new BspNode[mNumNodes];
        int i;
        // Convert nodes
        // In our array, first q3lvl.mNumNodes are non-leaf, others are leaves
        for (i = 0; i < q3lvl.mNumNodes; ++i)
        {
            BspNode* node = &mRootNode[i];
            bsp_node_t* q3node = &q3lvl.mNodes[i];

            // Set non-leaf
            node->mIsLeaf = false;
            // Set owner
            node->mOwner = this;
            // Set plane
            node->mSplitPlane.normal = Vector3(q3lvl.mPlanes[q3node->plane].normal);
            node->mSplitPlane.d = -q3lvl.mPlanes[q3node->plane].dist;
            // Set bounding box
            node->mBounds.setMinimum(Vector3(&q3node->bbox[0]));
            node->mBounds.setMaximum(Vector3(&q3node->bbox[3]));
            // Set back pointer
            // Negative indexes in Quake3 mean leaves
            if (q3node->back < 0)
            {
                // Points to leaf, offset to leaf start and negate index
                node->mBack = &mRootNode[mLeafStart + (~(q3node->back))];

            }
            else
            {
                // Points to node
                node->mBack = &mRootNode[q3node->back];
            }
            // Set front pointer
            // Negative indexes in Quake3 mean leaves
            if (q3node->front < 0)
            {
                // Points to leaf, offset to leaf start and negate index
                node->mFront = &mRootNode[mLeafStart + (~(q3node->front))];

            }
            else
            {
                // Points to node
                node->mFront = &mRootNode[q3node->front];
            }


        }
        //-----------------------------------------------------------------------
        // Brushes
        //-----------------------------------------------------------------------
        // Reserve enough memory for all brushes, solid or not (need to maintain indexes)
        mBrushes = new BspNode::Brush[q3lvl.mNumBrushes];
        for (i = 0; i < q3lvl.mNumBrushes; ++i)
        {
            bsp_brush_t* q3brush = &q3lvl.mBrushes[i];

            // Create a new OGRE brush
            BspNode::Brush *pBrush = &(mBrushes[i]);
            int brushSideIdx, numBrushSides;
            numBrushSides = q3brush->numsides;
            brushSideIdx = q3brush->firstside;
            // Iterate over the sides and create plane for each
            while (numBrushSides--)
            {
                bsp_brushside_t* q3brushside = &q3lvl.mBrushSides[brushSideIdx];
                bsp_plane_t* q3brushplane = &q3lvl.mPlanes[q3brushside->planenum];
                // Notice how we normally invert Q3A plane distances, but here we do not
                // Because we want plane normals pointing out of solid brushes, not in
                Plane brushSide(Vector3(q3brushplane->normal), q3brushplane->dist);
                pBrush->planes.push_back(brushSide);
                ++brushSideIdx;
            }
            // Build world fragment
            pBrush->fragment.fragmentType = SceneQuery::WFT_PLANE_BOUNDED_REGION;
            pBrush->fragment.planes = &(pBrush->planes);

        }



        //-----------------------------------------------------------------------
        // Leaves
        //-----------------------------------------------------------------------
        for (i = 0; i < q3lvl.mNumLeaves; ++i)
        {
            BspNode* node = &mRootNode[i + mLeafStart];
            bsp_leaf_t* q3leaf = &q3lvl.mLeaves[i];

            // Set leaf
            node->mIsLeaf = true;
            // Set owner
            node->mOwner = this;
            // Set bounding box
            node->mBounds.setMinimum(Vector3(&q3leaf->bbox[0]));
            node->mBounds.setMaximum(Vector3(&q3leaf->bbox[3]));
            // Set faces
            node->mFaceGroupStart = q3leaf->face_start;
            node->mNumFaceGroups = q3leaf->face_count;

            node->mVisCluster = q3leaf->cluster;

            // Load Brushes for this leaf
            int brushIdx, brushCount, realBrushIdx;
            brushCount = q3leaf->brush_count;
            brushIdx = q3leaf->brush_start;

            while(brushCount--)
            {
                realBrushIdx = q3lvl.mLeafBrushes[brushIdx];
                bsp_brush_t* q3brush = &q3lvl.mBrushes[realBrushIdx];
                // Only load solid ones, we don't care about any other types
                // Shader determines this
                bsp_shader_t* brushShader = &q3lvl.mShaders[q3brush->shaderIndex];
                if (brushShader->content_flags & CONTENTS_SOLID)
                {
                    // Get brush 
                    BspNode::Brush *pBrush = &(mBrushes[realBrushIdx]);
                    assert(pBrush->fragment.fragmentType == SceneQuery::WFT_PLANE_BOUNDED_REGION);
                    // Assign node pointer
                    node->mSolidBrushes.push_back(pBrush);
                }
                ++brushIdx;
            }

        }



        // Vis - just copy
        mVisData.numClusters = q3lvl.mVis->cluster_count;
        mVisData.rowLength = q3lvl.mVis->row_size;
        mVisData.tableData = new unsigned char[q3lvl.mVis->row_size * q3lvl.mVis->cluster_count];
        memcpy(mVisData.tableData, q3lvl.mVis->data, q3lvl.mVis->row_size * q3lvl.mVis->cluster_count);






    }

    //-----------------------------------------------------------------------
    bool BspLevel::isLeafVisible(const BspNode* from, const BspNode* to) const
    {
        if (to->mVisCluster == -1)
            return false;
        if (from->mVisCluster == -1)
            // Camera outside world?
            return true;


        if (!from->isLeaf() || !to->isLeaf())
            throw Exception(Exception::ERR_INVALIDPARAMS,
                "Both nodes must be leaf nodes for visibility testing.",
                "BspLevel::isLeafVisible");

        // Use PVS to determine visibility

        /*
        // In wordier terms, the fairly cryptic (but fast) version is doing this:
        //   Could make it a macro for even more speed?

        // Row offset = from cluster number * row size
        int offset = from->mVisCluster*mVisData.rowLength;

        // Column offset (in bytes) = to cluster number divided by 8 (since 8 bits per bytes)
        offset += to->mVisCluster >> 3;

        // Get the right bit within the byte, i.e. bitwise 'and' with bit at remainder position
        int result = *(mVisData.tableData + offset) & (1 << (to->mVisCluster & 7));

        return (result != 0);
        */

        //return ((*(mVisData.tableData + from->mVisCluster * mVisData.rowLength +
        //           ((to->mVisCluster)>>3)) & (1 << ((to->mVisCluster) & 7))) != 0);

        return (*(mVisData.tableData + from->mVisCluster*mVisData.rowLength +
                   ((to->mVisCluster)>>3)) & (1 << ((to->mVisCluster) & 7))) != 0;

    }
    //-----------------------------------------------------------------------
    const BspNode* BspLevel::getRootNode(void)
    {
        return mRootNode;
    }
    //-----------------------------------------------------------------------
    BspNode* BspLevel::findLeaf(const Vector3& point) const
    {
        BspNode* node = mRootNode;

        while (!node->isLeaf())
        {
            node = node->getNextNode(point);
        }

        return node;

    }
    //-----------------------------------------------------------------------
    void BspLevel::loadEntities(const Quake3Level& q3lvl)
    {
        char* strEnt;
        String line;
        StringVector vecparams;
        Vector3 origin;
        Real angle = 0;
        size_t pos;
        char* lineend;
        bool isPlayerStart;

        isPlayerStart = false;
        strEnt = (char*)q3lvl.mEntities;

        lineend = strchr(strEnt, '\n');
        while (lineend != 0)
        {
            *lineend = '\0';
            line = strEnt;
            strEnt = lineend+1;
            line.trim();
            if (line.length() > 0)
            {
                line = line.toLowerCase();
                // Remove quotes
				while( ( pos = line.find("\"",0) ) != String::npos )
                {
                    line = line.substr(0,pos) + line.substr(pos+1,line.length()-(pos+1));
                }
                vecparams = line.split();
                StringVector::iterator params = vecparams.begin();

                if (params[0] == "origin")
                {
                    origin.x = atof(params[1].c_str());
                    origin.y = atof(params[2].c_str());
                    origin.z = atof(params[3].c_str());
                }
                if (params[0] == "angle")
                {
                    angle = atof(params[1].c_str());
                }
                if (params[0] == "classname" && params[1] == "info_player_deathmatch")
                {
                    isPlayerStart = true;
                }
                if (params[0] == "}")
                {
                    if (isPlayerStart)
                    {
                        // Save player start
                        ViewPoint vp;
                        vp.position = origin;
                        vp.orientation.FromAngleAxis(Math::getSingleton().DegreesToRadians(angle), Vector3::UNIT_Z);
                        mPlayerStarts.push_back(vp);
                    }
                    isPlayerStart = false;
                }
            }

            lineend = strchr(strEnt, '\n');
        }


    }
    //-----------------------------------------------------------------------
    void BspLevel::_notifyObjectMoved(const MovableObject* mov, 
            const Vector3& pos)
    {

        // Locate any current nodes the object is supposed to be attached to
        MovableToNodeMap::iterator i = mMovableToNodeMap.find(mov);
        if (i != mMovableToNodeMap.end())
        {
            std::list<BspNode*>::iterator nodeit, nodeitend;
            nodeitend = i->second.end();
            for (nodeit = i->second.begin(); nodeit != nodeitend; ++nodeit)
            {
                // Tell each node
                (*nodeit)->_removeMovable(mov);
            }
            // Clear the existing list of nodes because we'll reevaluate it
            i->second.clear();
        }

        tagNodesWithMovable(mRootNode, mov, pos);
    }
    //-----------------------------------------------------------------------
    void BspLevel::tagNodesWithMovable(BspNode* node, const MovableObject* mov,
        const Vector3& pos)
    {
        if (node->isLeaf())
        {
            // Add to movable->node map
            // Insert all the time, will get current if already there
            std::pair<MovableToNodeMap::iterator, bool> p = 
                mMovableToNodeMap.insert(
                MovableToNodeMap::value_type(mov, std::list<BspNode*>()));

            p.first->second.push_back(node);

            // Add movable to node
            node->_addMovable(mov);

        }
        else
        {
            // Find distance to dividing plane
            Real dist = node->getDistance(pos);
            if (Math::Abs(dist) < mov->getBoundingRadius())
            {
                // Bounding sphere crosses the plane, do both
                tagNodesWithMovable(node->getBack(), mov, pos);
                tagNodesWithMovable(node->getFront(), mov, pos);
            }
            else if (dist < 0)
            {    //-----------------------------------------------------------------------

                // Do back
                tagNodesWithMovable(node->getBack(), mov, pos);
            }
            else
            {
                // Do front
                tagNodesWithMovable(node->getFront(), mov, pos);
            }
        }
    }
    //-----------------------------------------------------------------------
	void BspLevel::_notifyObjectDetached(const MovableObject* mov)	
	{
        // Locate any current nodes the object is supposed to be attached to
        MovableToNodeMap::iterator i = mMovableToNodeMap.find(mov);
        if (i != mMovableToNodeMap.end())
        {
            std::list<BspNode*>::iterator nodeit, nodeitend;
            nodeitend = i->second.end();
            for (nodeit = i->second.begin(); nodeit != nodeitend; ++nodeit)
            {
                // Tell each node
                (*nodeit)->_removeMovable(mov);
            }
            // delete the entry for this MovableObject
            mMovableToNodeMap.erase(i);
        }
	}
}
