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
#ifndef __SubEntity_H__
#define __SubEntity_H__

#include "OgrePrerequisites.h"

#include "OgreString.h"
#include "OgreRenderable.h"

namespace Ogre {

    /** Utility class which defines the sub-parts of an Entity.
        @remarks
            Just as meshes are split into submeshes, an Entity is made up of
            potentially multiple SubMeshes. These are mainly here to provide the
            link between the Material which the SubEntity uses (which may be the
            default Material for the SubMesh or may have been changed for this
            object) and the SubMesh data.
        @par
            The SubEntity also allows the application some flexibility in the
            material properties for this section of a particular instance of this
            Mesh, e.g. tinting the windows on a car model.
        @par
            SubEntity instances are never created manually. They are created at
            the same time as their parent Entity by the SceneManager method
            createEntity.
    */
    class _OgreExport SubEntity: public Renderable
    {
        // Note no virtual functions for efficiency
        friend class Entity;
        friend class SceneManager;
    private:
        /** Private constructor - don't allow creation by anybody else.
        */
        SubEntity();

        /** Private destructor.
        */
        virtual ~SubEntity() { }

        /// Pointer to parent.
        Entity* mParentEntity;

        /// Name of Material in use by this SubEntity.
        String mMaterialName;

        /// Cached pointer to material.
        Material* mpMaterial;

        // Pointer to the SubMesh defining geometry.
        SubMesh* mSubMesh;

    public:
        /** Gets the name of the Material in use by this instance.
        */
        const String& getMaterialName() const;

        /** Sets the name of the Material to be used.
            @remarks
                By default a SubEntity uses the default Material that the SubMesh
                uses. This call can alter that so that the Material is different
                for this instance.
        */
        void setMaterialName( const String& name );

        /** Accessor method to read mesh data.
        */
        SubMesh* getSubMesh(void);

        /** Overridden - see Renderable.
        */
        Material* getMaterial(void) const;

        /** Overridden - see Renderable.
        */
        void getRenderOperation(RenderOperation& rend);

        /** Overridden - see Renderable.
        */
        void getWorldTransforms(Matrix4* xform);
        
        /** Overridden - see Renderable.
        */
        unsigned short getNumWorldTransforms(void);
        /** Overridden, see Renderable */
        Real getSquaredViewDepth(const Camera* cam) const;

    };

}


#endif
