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
#include "OgrePythonPrerequisites.h"
#include "Ogre.h"


#include <boost/python/class_builder.hpp>

using namespace boost::python;


namespace Ogre {

    /** Class for defining the link between Ogre and Python
     *
     * This class exposes Ogre objects to the Python scripting language, using
     * Boost Python for the detail (www.boost.org).
     * Note that no platform or API specific classes are exported (eg. Win32Window, D3DRenderSystem)
     * since whilst these objects are created by Ogre the user application only interacts with them
     * through their non-specific superclasses i.e. RenderWindow and RenderSystem respectively.
     */
    class PythonLink
    {
    protected:
        module_builder* boostModule;

        void exportGeneral(void);
        void exportVector3(void);
        void exportMatrix3(void);
        void exportMatrix4(void);
        void exportAxisAlignedBox();
        void exportQuaternion(void);
        void exportRoot(void);
        void exportRenderWindow(void);
        void exportResource(void);
        void exportTextureManager(void);
        void exportSceneManager(void);
        void exportViewport(void);
        void exportCamera(void);
        void exportColourValue(void);
        void exportPlane(void);
        void exportRenderSystem(void);
        void exportMaterial(void);
        void exportMeshManager(void);
        void exportMesh(void);
        void exportLight(void);
        void exportEntity(void);
        void exportSceneNode(void);
        void exportFrameListener(void);

    public:
        PythonLink();
        ~PythonLink();

        /** Exports all the Ogre classes to Python - should be called as part of Python initOgre()) */
        void exportClasses(void);


    };

}


