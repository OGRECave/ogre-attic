/***************************************************************************
octreescenemanager.h  -  description
-------------------
begin                : Fri Sep 27 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Lesser General Public License as        *
*   published by the Free Software Foundation; either version 2 of the    * 
*   License, or (at your option) any later version.                       *
*                                                                         *
***************************************************************************/

#ifndef OCTREESCENEMANAGER_H
#define OCTREESCENEMANAGER_H

#include <OgreSceneManager.h>
#include <OgreRenderOperation.h>
#include <OgreSphere.h>

#include <list>
#include <algorithm>

#include <OgreOctree.h>


namespace Ogre
{

    class OctreeNode;

    class OctreeCamera;


    typedef std::list < AxisAlignedBox * > BoxList;
    typedef std::list < unsigned long > ColorList;

    /** Specialized SceneManager that divides the geometry into an octree in order to faciliate spatial queries.
    @remarks
    For debugging purposes, a special "CullCamera" can be defined.  To use it, call setUseCallCamera( true ),
    and create a camera named "CullCamera".  All culling will be performed using that camera, instead of the viewport
    camera, allowing you to fly around and examine culling.
    */

    class OctreeSceneManager : public SceneManager
    {
    public:
        static int intersect_call;
        /** Standard Constructor.  Initializes the octree to -500,-500,-500 to 500,500,500 with unlimited depth. */
        OctreeSceneManager( );
        /** Standard Constructor */
        OctreeSceneManager( AxisAlignedBox &box, int max_depth );
        /** Standard desctructor */
        ~OctreeSceneManager();

        /** Initializeds the manager to the given box and depth.
        */
        void init( AxisAlignedBox &box, int d );

        /** Creates a specialized OctreeNode */
        virtual	SceneNode * createSceneNode ( void );
        /** Creates a specialized OctreeNode */
        virtual SceneNode * createSceneNode ( const String &name );
        /** Creates a specialized OctreeCamera */
        virtual Camera * createCamera( const String &name );

	/** Deletes a scene node */
	virtual void destroySceneNode( const String &name );



        /** Does nothing more */
        virtual void _updateSceneGraph( Camera * cam );
        /** Renders the visible objects in the Octree */
        virtual void _renderVisibleObjects( void );
        /** Recurses through the octree determining which nodes are visible. */
        virtual void _findVisibleObjects ( Camera * cam );

        /** Walks through the octree, adding any visible objects to the render queue.
        @remarks
        If any octant in the octree if completely within the the view frustum,
        all subchildren are automatically added with no visibility tests.
        */
        void walkOctree( OctreeCamera *, RenderQueue *, Octree *, bool foundvisible );

        /** Checks the given OctreeNode, and determines if it needs to be moved
        * to a different octant.
        */
        void _updateOctreeNode( OctreeNode * );
        /** Removes the given octree node */
        void _removeOctreeNode( OctreeNode * );
        /** Adds the Octree Node, starting at the given octree, and recursing at max to the specified depth.
        */
        void _addOctreeNode( OctreeNode *, Octree *octree, int depth = 0 );

        /** Recurses the octree, adding any nodes intersecting with the box into the given list.
        It ignores the exclude scene node.
        */
        void findNodesIn( const AxisAlignedBox &box, std::list < SceneNode * > &list, SceneNode *exclude = 0, bool full = false, Octree *octant = 0 );

        /** Recurses the octree, adding any nodes intersecting with the sphere into the given list.
        It ignores the exclude scene node.
        */
        void findNodesIn( const Sphere &sphere, std::list < SceneNode * > &list, SceneNode *exclude = 0, bool full = false, Octree *octant = 0 );


        /** Sets the box visibility flag */
        void setShowBoxes( bool b )
        {
            mShowBoxes = b;
        };

        /** Sets the cull camera flag */
        void setUseCullCamera( bool b )
        {
            mCullCamera = b;
        };

        void setLooseOctree( bool b )
        {
            mLoose = b;
        };


	/** Resizes the octree to the given size */
	void resize( const AxisAlignedBox &box );

	/** Sets the given option for the SceneManager 
            @remarks
	    Options are:
	    "Size", AxisAlignedBox *;
	    "CullCamera", bool *;
	    "Depth", int *;
	    "ShowOctree", bool *;
	*/
	  
	virtual bool  setOption( const String &, const void * );
	/** Gets the given option for the Scene Manager.
	    @remarks
	    See setOption
	*/
	virtual bool getOption( const String &, void * );
  
	bool getOptionValues( const String & key, std::list<SDDataChunk>&refValueList );
	bool getOptionKeys( std::list< String > & refKeys);




    protected:

        /** Inserts the boxes corners into the Real array */
        void getBoxVerts( AxisAlignedBox &box, Real *r );

        /// the world size
        float mWorldSize;

        /// The root octree
        Octree *mOctree;

        /// list of boxes to be rendered
        BoxList mBoxes;

        /// temp render operation
        RenderOperation mRenderOp;

        /// number of rendered objs
        int mNumObjects;

        /// max depth for the tree.
        int mMaxDepth;

        /// box visibility flag
        bool mShowBoxes;

        /// cull camera flag
        bool mCullCamera;


        bool mLoose;

        Real mCorners[ 24 ];
        static unsigned long mColors[ 8 ];
        static unsigned short mIndexes[ 24 ];

        Matrix4 mScaleFactor;

    };

}

#endif

