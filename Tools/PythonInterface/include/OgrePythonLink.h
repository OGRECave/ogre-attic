/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "Ogre.h"

#ifdef HAVE_SNPRINTF
#   undef HAVE_SNPRINTF
#endif

#include <boost/python.hpp>
#include <boost/python/return_internal_reference.hpp>

// Shorten the names
typedef boost::python::return_value_policy<boost::python::copy_const_reference> ccr;
typedef boost::python::return_internal_reference<> rir;

void exportEnums(void);
void exportGeneral(void);
void exportVector3(void);
void exportMatrix3(void);
void exportMatrix4(void);
void exportAxisAlignedBox();
void exportSphere(void);
void exportQuaternion(void);
void exportRoot(void);
void exportRenderTarget(void);
void exportRenderWindow(void);
void exportResource(void);
void exportResourceManager(void);
void exportTexture(void);
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
void exportSubMesh(void);
void exportLight(void);
void exportMovableObject(void);
void exportEntity(void);
void exportNode(void);
void exportSceneNode(void);
void exportFrameListener(void);
void exportNode(void);
void exportInputEvent(void);
void exportKeyEvent(void);
void exportKeyListener(void);
void exportEventTarget(void);
void exportPositionTarget(void);
void exportPlatformManager(void);
void exportConfigDialog(void);
void exportErrorDialog(void);
void exportInput(void);
void exportEventQueue(void);
void exportMouseMotionListener(void);
