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
#ifndef _Ogre_H__
#define _Ogre_H__
// This file includes all the other files which you will need to build a client application
#include "OgrePrerequisites.h"

#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreArchive.h"
#include "OgreArchiveManager.h"
#include "OgreAxisAlignedBox.h"
#include "OgreBillboardSet.h"
#include "OgreBone.h"
#include "OgreCamera.h"
#include "OgreConfigFile.h"
#include "OgreControllerManager.h"
#include "OgreDataStream.h"
#include "OgreEntity.h"
#include "OgreEventProcessor.h"
#include "OgreException.h"
#include "OgreFrustum.h"
#include "OgreGpuProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreGuiContainer.h"
#include "OgreGuiManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreHardwareOcclusionQuery.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreInput.h"
#include "OgreKeyFrame.h"
#include "OgreLight.h"
#include "OgreLogManager.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"
#include "OgreMaterialSerializer.h"
#include "OgreMath.h"
#include "OgreMatrix3.h"
#include "OgreMatrix4.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMeshSerializer.h"
#include "OgreOverlay.h"
#include "OgreOverlayElement.h"
#include "OgreOverlayManager.h"
#include "OgreParticleAffector.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleSystemManager.h"
#include "OgrePass.h"
#include "OgrePatchMesh.h"
#include "OgrePatchSurface.h"
#include "OgrePlatformManager.h"
#include "OgreProfiler.h"
#include "OgreRenderQueueListener.h"
#include "OgreRenderSystem.h"
#include "OgreRenderTargetListener.h"
#include "OgreRenderTexture.h"
#include "OgreRenderWindow.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreSceneManagerEnumerator.h"
#include "OgreSceneNode.h"
#include "OgreSimpleRenderable.h"
#include "OgreSkeleton.h"
#include "OgreSkeletonInstance.h"
#include "OgreSkeletonManager.h"
#include "OgreSkeletonSerializer.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreStringVector.h"
#include "OgreSubEntity.h"
#include "OgreSubMesh.h"
#include "OgreTechnique.h"
#include "OgreTextureManager.h"
#include "OgreTextureManager.h"
#include "OgreTextureUnitState.h"
#include "OgreUserDefinedObject.h"
#include "OgreVector2.h"
#include "OgreViewport.h"
// .... more to come

#endif
