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
#ifndef __BspSceneManager_H__
#define __BspSceneManager_H__


#include "OgreBspPrerequisites.h"
#include "OgreSceneManager.h"
#include "OgreStaticFaceGroup.h"
#include "OgreRenderOperation.h"
#include <set>


namespace Ogre {

    /** Specialisation of the SceneManager class to deal with indoor scenes
        based on a BSP tree.
        This class refines the behaviour of the default SceneManager to manage
        a scene whose bulk of geometry is made up of an indoor environment which
        is organised by a Binary Space Partition (BSP) tree. </p>
        A BSP tree progressively subdivides the space using planes which are the nodes of the tree.
        At some point we stop subdividing and everything in the remaining space is part of a 'leaf' which
        contains a number of polygons. Typically we traverse the tree to locate the leaf in which a
        point in space is (say the camera origin) and work from there. A second structure, the
        Potentially Visible Set, tells us which other leaves can been seen from this
        leaf, and we test their bounding boxes against the camera frustum to see which
        we need to draw. Leaves are also a good place to start for collision detection since
        they divide the level into discrete areas for testing.</p>
        This BSP and PVS technique has been made famous by engines such as Quake and Unreal. Ogre
        provides support for loading Quake3 level files to populate your world through this class,
        by calling the BspSceneManager::setWorldGeometry. Note that this interface is made
        available at the top level of the SceneManager class so you don't have to write your code
        specifically for this class - just call Root::getSceneManager passing a SceneType of ST_INDOOR
        and in the current implementation you will get a BspSceneManager silently disguised as a
        standard SceneManager.
    */
    class BspSceneManager : public SceneManager
    {
    protected:

        // Pointer to resource manager just for singleton management
        BspResourceManager* mBspResMgr;

        // World geometry
        BspLevel* mLevel;

        // State variables for rendering WIP
        // Set of face groups (by index) already included
        typedef std::set<int> FaceGroupSet;
        FaceGroupSet mFaceGroupSet;
        // Material -> face group hashmap
        typedef std::map<Material*, std::vector<StaticFaceGroup*>, materialLess > MaterialFaceGroupMap;
        MaterialFaceGroupMap mMatFaceGroupMap;
        /** Cache of data about to go to the renderer. */
        RenderOperation mPendingGeometry;

        // Debugging features
        bool mShowNodeAABs;
        RenderOperation mAABGeometry;

        /** Walks the BSP tree looking for the node which the camera
            is in, and tags any geometry which is in a visible leaf for
            later processing.
            @param camera Pointer to the viewpoint.
            @returns The BSP node the camera was found in, for info.
        */
        BspNode* walkTree(Camera* camera);
        /** Tags geometry in the leaf specified for later rendering. */
        void processVisibleLeaf(BspNode* leaf, Camera* cam);

        /** Clears the caches of vertex anf face index data. Does not deallocate. */
        void clearGeometryCaches(void);
        /** Caches a face group for imminent rendering. */
        void cacheGeometry(const StaticFaceGroup* faceGroup);

        /** Frees up allocated memory for geometry caches. */
        void freeMemory(void);

        /** Adds a bounding box to draw if turned on. */
        void addBoundingBox(AxisAlignedBox& aab, bool visible);

        /** Renders the static level geometry tagged in walkTree. */
        void renderStaticGeometry(void);


    public:
        BspSceneManager();
        ~BspSceneManager();

        /** Specialised from SceneManager to support Quake3 bsp files. */
        void setWorldGeometry(const String& filename);

        /** Tells the manager whether to draw the axis-aligned boxes that surround
            nodes in the Bsp tree. For debugging purposes.
        */
        void showNodeBoxes(bool show);

        /** Specialised to suggest viewpoints. */
        ViewPoint getSuggestedViewpoint(bool random = false);

        /** Overriden from SceneManager. */
        void _findVisibleObjects(Camera* cam);

        /** Overriden from SceneManager. */
        void _renderVisibleObjects(void);
    };

}

#endif
