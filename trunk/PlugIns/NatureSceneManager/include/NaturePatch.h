/*****************************************************************************

File: NaturePatch.h
Desc: This class provides basic functionality of a terrain patch
Date: 2003/02/27

Author: Martin Persson

*****************************************************************************/

#ifndef __NATUREPATCH_H
#define __NATUREPATCH_H

#include <OgreSimpleRenderable.h>
#include <OgreRenderOperation.h>
#include <OgreCamera.h>
#include <OgreSceneNode.h>

namespace Ogre
{

    //----------------------------------------------------------------------------

    /// template for converting numbers into strings
    template<typename T> inline std::string toString(T t)
    {
        std::ostringstream s;
        s << t;
        return s.str();
    }

    //----------------------------------------------------------------------------

    class NaturePatch : public Renderable, public MovableObject
    {
        friend class NaturePatchManager;

    public:
        enum PatchType
        {
            TYPE_INVALID = 0,
            TYPE_TERRAIN = 1,
            TYPE_CUSTOM  = 2
        };

        struct TerrainData
        {
            PatchType type;

            // pointer to heightmap
            unsigned char *pHeightMap;
            // pointers to blendmaps
            unsigned char *pBlendMap[2];

            // number of blendmaps
            int numBlendMaps;

            // width of the heightmap
            int heightMapWidth;

            // material
            Material *pMaterial;
        };

        struct CustomData
        {
            PatchType type;

            int replace_with_some_stuff;
        };

        struct NaturePatchData
        {
            union
            {
                PatchType type;

                TerrainData terrain;
                CustomData  custom;
            };
        };

        NaturePatch()
        {
            // clear neighbor pointers
            mNorthNeighbor = mSouthNeighbor = mWestNeighbor = mEastNeighbor = 0;

            // generate name for this instance
            mName = "NatureQuad" + toString(mGenNameCount++);

            mNeedRendering = false;
        }

        virtual bool initialise(Vector3 world, Vector3 zone, Vector3 scale,
            NaturePatchData *data) = 0;

        /// Prepare the mesh for generation
        virtual void prepareMesh()  = 0;
        /// Generate mesh to be rendered
        virtual void generateMesh() = 0;

        /// Returns true if a vertex on the north edge is enabled
        virtual bool isNorthEdgeVertexEnabled(int vertexPos) = 0;

        /// Returns true if a vertex on the south edge is enabled
        virtual bool isSouthEdgeVertexEnabled(int vertexPos) = 0;

        /// Returns true if a vertex on the west edge is enabled
        virtual bool isWestEdgeVertexEnabled(int vertexPos)  = 0;

        /// Returns true if a vertex on the east edge is enabled
        virtual bool isEastEdgeVertexEnabled(int vertexPos)  = 0;

        /// Returns height in world units at given position (x/z unscaled units)
        virtual Real getHeightAt(int x, int z) const = 0;

        /// Link patch to its neighbors
        void attach(NaturePatch *north, NaturePatch *south,
            NaturePatch *west, NaturePatch *east)
        {
            if (north != 0)
            {
                north->mSouthNeighbor = this;
                mNorthNeighbor = north;
            }

            if (south != 0)
            {
                south->mNorthNeighbor = this;
                mSouthNeighbor = south;
            }

            if (west != 0)
            {
                west->mEastNeighbor = this;
                mWestNeighbor = west;
            }

            if (east != 0)
            {
                east->mWestNeighbor = this;
                mEastNeighbor = east;
            }
        }

        /// Unlinks this patch from its neighbors
        void detach()
        {
            if (mNorthNeighbor != 0)
            {
                mNorthNeighbor->mSouthNeighbor = 0;
                mNorthNeighbor = 0;
            }

            if (mSouthNeighbor != 0)
            {
                mSouthNeighbor->mNorthNeighbor = 0;
                mSouthNeighbor = 0;
            }

            if (mWestNeighbor != 0)
            {
                mWestNeighbor->mEastNeighbor = 0;
                mWestNeighbor = 0;
            }

            if (mEastNeighbor != 0)
            {
                mEastNeighbor->mWestNeighbor = 0;
                mEastNeighbor = 0;
            }
        }

        Real getSquaredViewDepth(const Camera *cam) const
        {
            return 0;
        }

        /// Returns the name of instance
        virtual const String &getName(void) const
        { 
            return mName;
        };

        /// Returns the movable type
        virtual const String& getMovableType(void) const
        {
            static String myType = "NaturePatch";
            return myType;
        }

        /// Returns the material
        virtual Material *getMaterial(void) const
        {
            return mMaterial;
        }

        /// Sets the material
        void setMaterial(Material *material)
        {
            mMaterial = material;
        }

        /// Returns the bounding box
        const AxisAlignedBox &getBoundingBox() const
        {
            return mBounds;
        }

        virtual void getWorldTransforms(Matrix4 *xform) const
        {
            *xform = mParentNode->_getFullTransform();
        }

        virtual const Quaternion& getWorldOrientation(void) const
        {
            return mParentNode->_getDerivedOrientation();
        }
        virtual const Vector3& getWorldPosition(void) const
        {
            return mParentNode->_getDerivedPosition();
        }
        virtual void _updateRenderQueue(RenderQueue *queue)
        {
            queue->addRenderable(this);
        }

        virtual void getRenderOperation(RenderOperation& op) = 0;

        virtual void _notifyCurrentCamera(Camera *cam) = 0;

        /** @copydoc Renderable::getLights */
        const LightList& getLights(void) const { return getParentSceneNode()->findLights(getBoundingRadius()); }

    protected:
        /// Pointers to neighboring patches
        NaturePatch *mNorthNeighbor;
        NaturePatch *mSouthNeighbor;
        NaturePatch *mWestNeighbor;
        NaturePatch *mEastNeighbor;

        /// Position in world coordinates
        Vector3	mWorld;
        /// Position relative to zone
        Vector3	mZone;
        /// Scaling info
        Vector3	mScale;

        /// Bounding box
        AxisAlignedBox mBounds;

        /// Material used for rendering
        Material *mMaterial;

        /// Pointer to patch data
        NaturePatchData *mData;

        /// Unique name for each instance
        String mName;
        static unsigned int mGenNameCount;

        /// Flag that indicates if this patch needs to be re-rendered
        bool mNeedRendering;
    };

}

#endif
