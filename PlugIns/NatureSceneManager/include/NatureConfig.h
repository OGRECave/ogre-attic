/*****************************************************************************

	File: NatureConfig.h
	Desc: Some default values, don't change any if you don't know what 
	      you are doing :)
	Date: 2003/02/22

	Author: Martin Persson

*****************************************************************************/

#ifndef __NATURECONFIG_H
#define __NATURECONFIG_H

namespace Ogre
{

//----------------------------------------------------------------------------

#define QUADTREE_SIZE	    65

/*
 *  QUADTREE_DEPTH = log2(QUADTREE_SIZE - 1)
 */
#define QUADTREE_DEPTH	    6

/*
 *  QUADTREE_NODES = 
 *
 *  for (nodes = 0, lvl = 0; lvl < QUADTREE_DEPTH; lvl++)
 *  {
 *	nodes += (1 << (2 * lvl));
 *  }
 */
#define QUADTREE_NODES	    1365

#define EDGE_LENGTH	    (QUADTREE_SIZE - 1)
#define PATCH_SIZE	    (QUADTREE_SIZE - 1)

#define USE_NORMALS	    0

#define USE_COLOURS	    0

#define USE_TEXTURES    1

} // namespace Ogre

#endif
