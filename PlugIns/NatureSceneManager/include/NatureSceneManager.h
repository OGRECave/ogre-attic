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

// forward decl
class NatureIntersectionSceneQuery;

class NatureSceneManager : public SceneManager
{
    friend class NatureIntersectionSceneQuery;
public:
    NatureSceneManager();

    virtual ~NatureSceneManager();

    void setWorldGeometry(const String &filename);

    bool setOption(const String& strKey, const void *pValue);

    virtual void _renderVisibleObjects();

    virtual void _updateSceneGraph(Camera *cam); 
    IntersectionSceneQuery* createIntersectionQuery(unsigned long mask);

	void clearScene(void);

private:
    SceneNode	    *mNatureRoot;
    // Passthrough call to allow queries to access parent protected entity list
    EntityList& getEntities() { return mEntities; }

	void flushWorldGeometry(void);
    NaturePatchManager *mNaturePatchManager;
    NaturePatchLoader  *mNaturePatchLoader;
};


/** Nature's specialisation of IntersectionSceneQuery. */
class NatureIntersectionSceneQuery : 
    public DefaultIntersectionSceneQuery
{
public:
    NatureIntersectionSceneQuery(SceneManager* creator) : DefaultIntersectionSceneQuery(creator) 
    { mSupportedWorldFragments.insert(SceneQuery::WFT_RENDER_OPERATION);}
    ~NatureIntersectionSceneQuery() {}

    /** See IntersectionSceneQuery. */
    void execute(IntersectionSceneQueryListener* listener);
};
} // namespace Ogre


#endif
