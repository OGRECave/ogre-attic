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
#ifndef __Entity_H__
#define __Entity_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"

#include "OgreString.h"
#include "OgreMovableObject.h"
#include "OgreAnimationState.h"
#include "OgreQuaternion.h"
#include "OgreVector3.h"


namespace Ogre {
    /** Defines an instance of a discrete, movable object based on a Mesh.
        @remarks
            Ogre generally divides renderable objects into 2 groups, discrete
            (separate) and relatively small objects which move around the world,
            and large, sprawling geometry which makes up generally immovable
            scenery, aka 'level geometry'.
        @par
            The Mesh and SubMesh classes deal with the definition of the geometry
            used by discrete movable objects. Entities are actual instances of
            objects based on this geometry in the world. Therefore there is
            usually a single set Mesh for a car, but there may be multiple
            entities based on it in the world. Entities are able to override
            aspects of the Mesh it is defined by, such as changing material
            properties per instance (so you can have many cars using the same
            geometry but different textures for example). Because a Mesh is split
            into SubMeshes for this purpose, the Entity class is a grouping class
            (much like the Mesh class) and much of the detail regarding
            individual changes is kept in the SubEntity class. There is a 1:1
            relationship between SubEntity instances and the SubMesh instances
            associated with the Mesh the Entity is based on.
        @par
            Entity and SubEntity classes are never created directly. Use the
            createEntity method of the SceneManager (passing a model name) to
            create one.
        @par
            Entities are included in the scene by associating them with a
            SceneNode, using the attachEntity method. See the SceneNode class
            for full information.
        @note
            No functions were declared virtual to improve performance.
    */
    class _OgreExport Entity: public MovableObject
    {
        // Allow SceneManager full access
        friend class SceneManager;
        friend class SubEntity;
    private:

        /** Private constructor (instances cannot be created directly).
        */
        Entity();
        /** Private constructor - specify name (the usual constructor used).
        */
        Entity( const String& name, Mesh* mesh, SceneManager* creator);

        /** Name of the entity; used for location in the scene.
        */
        String mName;

        /** The Mesh that this Entity is based on.
        */
        Mesh* mMesh;

        /** List of SubEntities (point to SubMeshes).
        */
        typedef std::vector<SubEntity*> SubEntityList;
        SubEntityList mSubEntityList;

        /** Pointer back to the SceneManager that created this instance, for
            notification purposes.
        */
        SceneManager* mCreatorSceneManager;


        /// State of animation for animable meshes
        AnimationStateSet mAnimationState;

        /// Shared class-level name for Movable type
        static String msMovableType;


        /// Cached bone matrices, including any world transform
        Matrix4 *mBoneMatrices;
        unsigned short mNumBoneMatrices;

        /// Private method to cache bone matrices from skeleton
        void cacheBoneMatrices(void);

        /// Flag determines whether or not to display skeleton
        bool mDisplaySkeleton;


		/// The LOD number of the mesh to use, calculated by _notifyCurrentCamera
		ushort mMeshLodIndex;

		/// LOD bias factor, inverted for optimisation when calculating adjusted depth
		Real mMeshLodFactorInv;
		/// Index of minimum detail LOD (NB higher index is lower detail)
		ushort mMinMeshLodIndex;
		/// Index of maximum detail LOD (NB lower index is higher detail)
		ushort mMaxMeshLodIndex;

		/// LOD bias factor, inverted for optimisation when calculating adjusted depth
		Real mMaterialLodFactorInv;
		/// Index of minimum detail LOD (NB higher index is lower detail)
		ushort mMinMaterialLodIndex;
		/// Index of maximum detail LOD (NB lower index is higher detail)
		ushort mMaxMaterialLodIndex;

        /// Flag indicating that mesh uses manual LOD and so might have multiple SubEntity versions
		bool mUsingManualLOD;
		/** List of SubEntity lists (for manual LODs).
			We don't know when the mesh is using manual LODs whether one LOD to the next will have the
			same number of SubMeshes, therefore we have to allow a SubEntity list with each alternate one.
		*/
		typedef std::vector<SubEntityList*> LODSubEntityList;
		LODSubEntityList mLodSubEntityList;

		/** Builds a list of SubEntities based on the SubMeshes contained in the Mesh. */
		void buildSubEntityList(Mesh* mesh, SubEntityList* sublist);

		/// internal implementation of attaching a 'child' object to this entity and assign the parent node to the child entity
		void attachObjectImpl(MovableObject *pMovable, TagPoint *pAttachingPoint);

    public:
        /// Contains the child objects (attached to bones) indexed by name
        typedef std::map<String, MovableObject*> ChildObjectList;
    protected:
        ChildObjectList mChildObjectList;


		/// Bounding box that 'contains' all the mesh of each child entity
		AxisAlignedBox *mFullBoundingBox;

        bool mNormaliseNormals;


    public:
        /** Default destructor.
        */
        ~Entity();

        /** Gets the Mesh that this Entity is based on.
        */
        Mesh* getMesh(void);

        /** Gets a pointer to a SubEntity, ie a part of an Entity.
        */
        SubEntity* getSubEntity(unsigned int index);
	
		/** Gets a pointer to a SubEntity by name
			@remarks - names should be initialized during a Mesh creation.
		*/
		SubEntity* getSubEntity( const String& name );

        /** Retrieves the number of SubEntity objects making up this entity.
        */
        unsigned int getNumSubEntities(void) const;

        /** Clones this entity and returns a pointer to the clone.
            @remarks
                Useful method for duplicating an entity. The new entity must be
                given a unique name, and is not attached to the scene in any way
                so must be attached to a SceneNode to be visible (exactly as
                entities returned from SceneManager::createEntity).
            @param
                newName Name for the new entity.
        */
        Entity* clone( const String& newName );

        /** Sets the material to use for the whole of this entity.
            @remarks
                This is a shortcut method to set all the materials for all
                subentities of this entity. Only use this method is you want to
                set the same material for all subentities or if you know there
                is only one. Otherwise call getSubEntity() and call the same
                method on the individual SubEntity.
        */
        void setMaterialName(const String& name);

        /** Overridden - see MovableObject.
        */
        void _notifyCurrentCamera(Camera* cam);

        /** Overridden - see MovableObject.
        */
        const AxisAlignedBox& getBoundingBox(void) const;

        /// merge all the child object Bounds a return it
		AxisAlignedBox getChildObjectsBoundingBox(void) const;

        /** Overridden - see MovableObject.
        */
        void _updateRenderQueue(RenderQueue* queue);

        /** Overridden from MovableObject */
        const String& getName(void) const;

        /** Overridden from MovableObject */
        const String& getMovableType(void) const;

        /** For entities based on animated meshes, gets the AnimationState object for a single animation.
        @remarks
            You animate an entity by updating the animation state objects. Each of these represents the
            current state of each animation available to the entity. The AnimationState objects are
            initialised from the Mesh object.
        */
        AnimationState* getAnimationState(const String& name);
        /** For entities based on animated meshes, gets the AnimationState objects for all animations.
        @remarks
            You animate an entity by updating the animation state objects. Each of these represents the
            current state of each animation available to the entity. The AnimationState objects are
            initialised from the Mesh object.
        */
        AnimationStateSet* getAllAnimationStates(void);

        /** Tells the Entity whether or not it should display it's skeleton, if it has one.
        */
        void setDisplaySkeleton(bool display);


		/** Sets a level-of-detail bias for the mesh detail of this entity.
		@remarks
			Level of detail reduction is normally applied automatically based on the Mesh 
			settings. However, it is possible to influence this behaviour for this entity
			by adjusting the LOD bias. This 'nudges' the mesh level of detail used for this 
			entity up or down depending on your requirements. You might want to use this
			if there was a particularly important entity in your scene which you wanted to
			detail better than the others, such as a player model.
		@par
			There are three parameters to this method; the first is a factor to apply; it 
			defaults to 1.0 (no change), by increasing this to say 2.0, this model would 
			take twice as long to reduce in detail, whilst at 0.5 this entity would use lower
			detail versions twice as quickly. The other 2 parameters are hard limits which 
			let you set the maximum and minimum level-of-detail version to use, after all
			other calculations have been made. This lets you say that this entity should
			never be simplified, or that it can only use LODs below a certain level even
			when right next to the camera.
		@param factor Proportional factor to apply to the distance at which LOD is changed. 
			Higher values increase the distance at which higher LODs are displayed (2.0 is 
			twice the normal distance, 0.5 is half).
		@param maxDetailIndex The index of the maximum LOD this entity is allowed to use (lower
			indexes are higher detail: index 0 is the original full detail model).
		@param minDetailIndex The index of the minimum LOD this entity is allowed to use (higher
			indexes are lower detail. Use something like 99 if you want unlimited LODs (the actual
			LOD will be limited by the number in the Mesh)
		*/
		void setMeshLodBias(Real factor, ushort maxDetailIndex = 0, ushort minDetailIndex = 99);

		/** Sets a level-of-detail bias for the material detail of this entity.
		@remarks
			Level of detail reduction is normally applied automatically based on the Material 
			settings. However, it is possible to influence this behaviour for this entity
			by adjusting the LOD bias. This 'nudges' the material level of detail used for this 
			entity up or down depending on your requirements. You might want to use this
			if there was a particularly important entity in your scene which you wanted to
			detail better than the others, such as a player model.
		@par
			There are three parameters to this method; the first is a factor to apply; it 
			defaults to 1.0 (no change), by increasing this to say 2.0, this entity would 
			take twice as long to use a lower detail material, whilst at 0.5 this entity 
            would use lower detail versions twice as quickly. The other 2 parameters are 
            hard limits which let you set the maximum and minimum level-of-detail index 
            to use, after all other calculations have been made. This lets you say that 
            this entity should never be simplified, or that it can only use LODs below 
            a certain level even when right next to the camera.
		@param factor Proportional factor to apply to the distance at which LOD is changed. 
			Higher values increase the distance at which higher LODs are displayed (2.0 is 
			twice the normal distance, 0.5 is half).
		@param maxDetailIndex The index of the maximum LOD this entity is allowed to use (lower
			indexes are higher detail: index 0 is the original full detail model).
		@param minDetailIndex The index of the minimum LOD this entity is allowed to use (higher
			indexes are lower detail. Use something like 99 if you want unlimited LODs (the actual
			LOD will be limited by the number of lod indexes used in the Material)
		*/
        void setMaterialLodBias(Real factor, ushort maxDetailIndex = 0, ushort minDetailIndex = 99);
			
        /** Sets the rendering detail of this entire entity (solid, wireframe etc) */
        void setRenderDetail(SceneDetailLevel renderDetail);

		/** Attaches another object to a certain bone of the skeleton which this entity uses.
        @remarks
            This method can be used to attach another object to an animated part of this entity,
            by attaching it to a bone in the skeleton (with an offset if required). As this entity 
            is animated, the attached object will move relative to the bone to which it is attached.
        @param boneName The name of the bone (in the skeleton) to attach this object
        @param pMovable Pointer to the object to attach
        @param offsetOrientation An adjustment to the orientation of the attached object, relative to the bone.
        @param offsetPosition An adjustment to the position of the attached object, relative to the bone.
        */
		void attachObjectToBone(const String &boneName, MovableObject *pMovable, const Quaternion &offsetOrientation = Quaternion::IDENTITY, const Vector3 &offsetPosition = Vector3::ZERO);

		/// detach a MovableObject previously attached using attachObjectToBone
		MovableObject* detachObjectFromBone(const String &movableName);

        typedef MapIterator<ChildObjectList> ChildObjectListIterator;
        /** Gets an iterator to the list of objects attached to bones on this entity. */
        ChildObjectListIterator getAttachedObjectIterator(void);
        /** @see MovableObject::getBoundingRadius */
		Real getBoundingRadius(void) const;
        /** If set to true, this forces normals of this entity to be normalised
            dynamically by the hardware.
        @remarks
            This option can be used to prevent lighting variations when scaling an
            Entity using a SceneNode - normally because this scaling is hardware
            based, the normals get scaled too which causes lighting to become inconsistent.
            However, this has an overhead so only do this if you really need to.
        */
        void setNormaliseNormals(bool normalise) { mNormaliseNormals = normalise; }

        /** Returns true if this entity has auto-normalisation of normals set. */
        bool getNormaliseNormals(void) const {return mNormaliseNormals; }


    };

} // namespace

#endif
