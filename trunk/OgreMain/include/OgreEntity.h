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
#ifndef __Entity_H__
#define __Entity_H__

#include "OgrePrerequisites.h"

#include "OgreString.h"
#include "OgreMovableObject.h"
#include "OgreAnimationState.h"

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

    public:
        /** Default destructor.
        */
        ~Entity();

        /** Retrieves the Entity's name (cannot be changed after creation).
        */
        const String& getName(void) const;

        /** Gets the Mesh that this Entity is based on.
        */
        Mesh* getMesh(void);


        /** Gets a pointer to a SubEntity, ie a part of an Entity.
        */
        SubEntity* getSubEntity(unsigned int index);

        /** Retrieves the number of SubEntity objects making up this entity.
        */
        unsigned int getNumSubEntities(void);

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

        /** Overridden - see MovableObject.
        */
        void _updateRenderQueue(RenderQueue* queue);

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
    };

} // namespace

#endif
