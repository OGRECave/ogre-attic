/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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
#ifndef __ShadowCaster_H__
#define __ShadowCaster_H__

#include "OgrePrerequisites.h"
#include "OgreEdgeListBuilder.h"
#include "OgreRenderable.h"


namespace Ogre {


    /** Class which represents the renderable aspects of a set of shadow volume faces. 
    @remarks
        Note that for casters comprised of more than one set of vertex buffers (e.g. SubMeshes each
        using their own geometry), it will take more than one ShadowRenderable to render the 
        shadow volume. Therefore for shadow caster geometry, it is best to stick to one set of
        vertex buffers (not necessarily one buffer, but the positions for the entire geometry 
        should come from one buffer if possible)
    */
    class _OgreExport ShadowRenderable : public Renderable
    {
    protected:
        Material* mMaterial;
        RenderOperation mRenderOp;
    public:
        ShadowRenderable() {}
        /** Set the material to be used by the shadow, should be set by the caller 
          before adding to a render queue
        */
        void setMaterial(Material* mat) { mMaterial = mat; }
        /// Overridden from Renderable
        Material* getMaterial(void) const { return mMaterial; }
        /// Overridden from Renderable
        void getRenderOperation(RenderOperation& op) { op = mRenderOp; }
        /// Get the internal render operation for set up
        RenderOperation* getRenderOperationForUpdate(void) {return &mRenderOp;}
        /// Overridden from Renderable
        void getWorldTransforms(Matrix4* xform) const = 0;
        /// Overridden from Renderable
        const Quaternion& getWorldOrientation(void) const = 0;
        /// Overridden from Renderable
        const Vector3& getWorldPosition(void) const = 0;
        /// Overridden from Renderable
        Real getSquaredViewDepth(const Camera* cam) const{ return 0; /* not used */}
        /// Overridden from Renderable
        const LightList& getLights(void) const 
        {
            // return empty
            static LightList ll;
            return ll;
        }

    };

    /** A set of flags that can be used to influence ShadowRenderable creation. */
    enum ShadowRenderableFlags
    {
        /// For shadow volume techniques only, generate a light cap on the volume
        SRF_INCLUDE_LIGHT_CAP = 0x00000001,
        /// For shadow volume techniques only, generate a dark cap on the volume
        SRF_INCLUDE_DARK_CAP  = 0x00000002
    };

    /** This class defines the interface that must be implemented by shadow casters.
    */
    class _OgreExport ShadowCaster
    {
    public:
        /** Returns whether or not this object currently casts a shadow. */
        virtual bool getCastShadows(void) const = 0;

        /** Returns details of the edges which might be used to determine a silhouette. */
        virtual EdgeData* getEdgeList(void) = 0;

        typedef std::vector<ShadowRenderable*> ShadowRenderableList;
        typedef VectorIterator<ShadowRenderableList> ShadowRenderableListIterator;

        /** Gets an iterator over the renderables required to render the shadow volume. 
        @remarks
            Shadowable geometry should ideally be designed such that there is only one
            ShadowRenderable required to render the the shadow; however this is not a necessary
            limitation and it can be exceeded if required.
        @param shadowTechnique The technique being used to generate the shadow
        @param light The light to generate the shadow from
        @param flags Technique-specific flags, see ShadowRenderableFlags
        @param useThisIndexBuffer If non-null, the caster will use this index buffer
            to build the renderables, using space right from the start to whatever
            it needs for all renderables.
        */
        virtual ShadowRenderableListIterator getShadowVolumeRenderableIterator(
            ShadowTechnique shadowTechnique, const Light* light, 
            unsigned long flags = 0, HardwareIndexBufferSharedPtr* useThisIndexBuffer = 0) = 0;

    protected:
        /** Tells the caster to perform the tasks necessary to update the 
            edge data's light listing. Can be overridden if the subclass needs 
            to do additional things. 
        @param edgeData The edge information to update
        @param lightPos 4D vector representing the light, a directional light
            has w=0.0
       */
        virtual void updateEdgeListLightFacing(EdgeData* edgeData, 
            const Vector4& lightPos);

        /** Generates the indexes required to render a shadow volume into the 
            index buffer which is passed in, and updates shadow renderables
            to use it.
        @param edgeData The edge information to use
        @param indexBuffer The buffer into which to write data into; current 
            contents are assumed to be discardable.
        @param light The light, mainly for type info as silhouette calculations
            should already have been done in updateEdgeListLightFacing
        @param shadowRenderables A list of shadow renderables which has 
            already been constructed but will need populating with details of
            the index ranges to be used.
        @param flags Additional controller flags, see ShadowRenderableFlags
        */
        virtual void generateShadowVolume(EdgeData* edgeData, 
            HardwareIndexBufferSharedPtr indexBuffer, const Light* light,
            ShadowRenderableList& shadowRenderables, unsigned long flags);

    };
}

#endif 
