/*****************************************************************************

	File: NatureSceneManager.h
	Desc: 
	Date: 2003/02/23

	Author: Martin Persson

*****************************************************************************/

#ifndef __NATURESCENEMANAGER_H
#define __NATURESCENEMANAGER_H

#include <OgreSceneManager.h>

#include "NaturePatchLoader.h"
#include "NaturePatchManager.h"

namespace Ogre
{

//----------------------------------------------------------------------------

class NatureSceneManager : public SceneManager
{
public:
    NatureSceneManager();

    virtual ~NatureSceneManager();

    void setWorldGeometry(const String &filename);

    bool setOption(const String& strKey, const void *pValue);

    virtual void _renderVisibleObjects();

    virtual void _updateSceneGraph(Camera *cam); 

private:
    SceneNode	    *mNatureRoot;

    NaturePatchManager *mNaturePatchManager;
    NaturePatchLoader  *mNaturePatchLoader;
};

} // namespace Ogre


#endif
