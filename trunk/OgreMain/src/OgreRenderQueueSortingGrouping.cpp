/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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
#include "OgreStableHeaders.h"
#include "OgreRenderQueueSortingGrouping.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    void RenderPriorityGroup::destroySolidPassMap(SolidRenderablePassMap& passmap)
    {
        // destroy all the pass map entries
        SolidRenderablePassMap::iterator i, iend;
        iend = passmap.end();
        for (i = passmap.begin(); i != iend; ++i)
        {
            // Free the list associated with this pass
            delete i->second;
        }
        passmap.clear();
    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::removeSolidPassEntry(Pass* p)
    {
        SolidRenderablePassMap::iterator i;

        i = mSolidPasses.find(p);
        if (i != mSolidPasses.end())
        {
            // free memory
            delete i->second;
            // erase from map
            mSolidPasses.erase(i);
        }

        i = mSolidPassesDiffuseSpecular.find(p);
        if (i != mSolidPassesDiffuseSpecular.end())
        {
            // free memory
            delete i->second;
            // erase from map
            mSolidPassesDiffuseSpecular.erase(i);
        }
        i = mSolidPassesDecal.find(p);
        if (i != mSolidPassesDecal.end())
        {
            // free memory
            delete i->second;
            // erase from map
            mSolidPassesDecal.erase(i);
        }

    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::clearSolidPassMap(SolidRenderablePassMap& passmap)
    {
        SolidRenderablePassMap::iterator i, iend;
        iend = passmap.end();
        for (i = passmap.begin(); i != iend; ++i)
        {
            // Clear the list associated with this pass, but leave the pass entry
            i->second->clear();
        }
    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::addRenderable(Renderable* rend)
    {
        // Check material & technique supplied (the former since the default implementation
        // of getTechnique is based on it for backwards compatibility
        Technique* pTech;
        if(!rend->getMaterial() || !rend->getTechnique())
        {
            // Use default base white
            pTech = static_cast<Material*>(
                MaterialManager::getSingleton().getByName("BaseWhite"))->getTechnique(0);
        }
        else
        {
            // Get technique
            pTech = rend->getTechnique();
        }

        // Transparent?
        if (pTech->isTransparent())
        {
            addTransparentRenderable(pTech, rend);
        }
        else
        {
            if (mSplitPassesByLightingType)
            {
                addSolidRenderableSplitByLightType(pTech, rend);
            }
            else
            {
                addSolidRenderable(pTech, rend);
            }
        }

    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::addSolidRenderable(Technique* pTech, Renderable* rend)
    {
        Technique::PassIterator pi = pTech->getPassIterator();

        while (pi.hasMoreElements())
        {
            // Insert into solid list
            Pass* p = pi.getNext();
            SolidRenderablePassMap::iterator i = mSolidPasses.find(p);
            if (i == mSolidPasses.end())
            {
                std::pair<SolidRenderablePassMap::iterator, bool> retPair;
                // Create new pass entry, build a new list
                // Note that this pass and list are never destroyed until the engine
                // shuts down, although the lists will be cleared
                retPair = mSolidPasses.insert(
                    SolidRenderablePassMap::value_type(p, new RenderableList() ) );
                assert(retPair.second && "Error inserting new pass entry into SolidRenderablePassMap");
                i = retPair.first;
            }
            // Insert renderable
            i->second->push_back(rend);

        }
    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::addSolidRenderableSplitByLightType(Technique* pTech, Renderable* rend)
    {
        // Divide the passes into the 3 categories
        Technique::IlluminationPassIterator pi = 
            pTech->getIlluminationPassIterator();

        while (pi.hasMoreElements())
        {
            // Insert into solid list
            IlluminationPass* p = pi.getNext();
            SolidRenderablePassMap* passMap;
            switch(p->stage)
            {
            case IS_AMBIENT:
                passMap = &mSolidPasses;
                break;
            case IS_PER_LIGHT:
                passMap = &mSolidPassesDiffuseSpecular;
                break;
            case IS_DECAL:
                passMap = &mSolidPassesDecal;
                break;
            };

            SolidRenderablePassMap::iterator i = passMap->find(p->pass);
            if (i == passMap->end())
            {
                std::pair<SolidRenderablePassMap::iterator, bool> retPair;
                // Create new pass entry, build a new list
                // Note that this pass and list are never destroyed until the engine
                // shuts down, although the lists will be cleared
                retPair = passMap->insert(
                    SolidRenderablePassMap::value_type(p->pass, new RenderableList() ) );
                assert(retPair.second && "Error inserting new pass entry into SolidRenderablePassMap");
                i = retPair.first;
            }
            // Insert renderable
            i->second->push_back(rend);
        }
    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::addTransparentRenderable(Technique* pTech, Renderable* rend)
    {
        Technique::PassIterator pi = pTech->getPassIterator();

        while (pi.hasMoreElements())
        {
            // Insert into transparent list
            mTransparentPasses.push_back(RenderablePass(rend, pi.getNext()));
        }
    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::sort(const Camera* cam)
    {
        TransparentQueueItemLess transFunctor;
        transFunctor.camera = cam;

        std::stable_sort(mTransparentPasses.begin(), mTransparentPasses.end(), 
            transFunctor);
    }
    //-----------------------------------------------------------------------
    void RenderPriorityGroup::clear(void)
    {
        SolidRenderablePassMap::iterator i, iend;
        // Delete queue groups which are using passes which are to be
        // deleted, we won't need these any more and they clutter up 
        // the list and can cause problems with future clones
        const Pass::PassSet& graveyardList = Pass::getPassGraveyard();
        Pass::PassSet::const_iterator gi, giend;
        giend = graveyardList.end();
        for (gi = graveyardList.begin(); gi != giend; ++gi)
        {
            removeSolidPassEntry(*gi);
        }

        // Now remove any dirty passes, these will have their hashes recalculated
        // by the parent queue after all groups have been processed
        // If we don't do this, the std::map will become inconsistent for new insterts
        const Pass::PassSet& dirtyList = Pass::getDirtyHashList();
        Pass::PassSet::const_iterator di, diend;
        diend = dirtyList.end();
        for (di = dirtyList.begin(); di != diend; ++di)
        {
            removeSolidPassEntry(*di);
        }
        // NB we do NOT clear the graveyard or the dirty list here, because 
        // it needs to be acted on for all groups, the parent queue takes 
        // care of this afterwards

        // We do not clear the unchanged solid pass maps, only the contents of each list
        // This is because we assume passes are reused a lot and it saves resorting
        clearSolidPassMap(mSolidPasses);
        clearSolidPassMap(mSolidPassesDecal);
        clearSolidPassMap(mSolidPassesDiffuseSpecular);

        // Always empty the transparents list
        mTransparentPasses.clear();

    }
    //-----------------------------------------------------------------------





}

