/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
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
#ifndef __SceneQuery_H__
#define __SceneQuery_H__

#include "OgrePrerequisites.h"
#include "OgreAxisAlignedBox.h"
#include "OgreSphere.h"

namespace Ogre {

    typedef std::list<MovableObject*> SceneQueryResultMovableList;
    /** Holds the results of a scene query. */
    struct _OgreExport SceneQueryResult
    {
        /// List of movable objects in the query (entities, particle systems etc)
        SceneQueryResultMovableList movables;
        // TODO: add world geometry fragment list
        //
        
    };

    /** This optional class allows you to receive per-result callbacks from
        SceneQuery executions instead of a single set of consolidated results.
    @remarks
        You should override this with your own subclass. Note that certain query
        classes may refine this listener interface.
    */
    class _OgreExport SceneQueryListener
    {
    public:
        virtual bool queryResult(MovableObject* object) = 0;
        // TODO: world geometry callback
        //virtual bool queryResult(WorldFragment* fragment);
        
    };

    /** A class for performing queries on a scene.
    @remarks
        This is an abstract class for performing a query on a scene, i.e. to retrieve
        a list of objects and/or world geometry sections which are intersecting a
        given region.
    @par
        Different SceneManagers will implement these queries in different ways to
        exploit their particular scene organisation, and thus will provide their own
        concrete subclasses. In fact, these subclasses will be derived from subclasses
        of this class rather than directly because there will be region-type classes
        in between.
    @par
        These queries could have just been implemented as methods on the SceneManager,
        however, they are wrapped up as objects to allow 'compilation' of queries
        if deemed appropriate by the implementation; i.e. each concrete subclass may
        precalculate information (such as fixed scene partitions involved in the query)
        to speed up the repeated use of the query.
    @par
        You should never try to create a SceneQuery object yourself, they should be created
        using the SceneManager interfaces for the type of query required, e.g.
        SceneManager::createQueryForSphere.
    */
    class _OgreExport SceneQuery
    {
    protected:
        SceneQueryResult* mLastResult;
        SceneManager* mParentSceneMgr;
        unsigned long mQueryMask;
    
    public:
        /** Standard constructor, should be called by SceneManager. */
        SceneQuery(SceneManager* mgr);
        virtual ~SceneQuery();
        /** Executes the query, returning the results back in one list.
        @remarks
            This method executes the scene query as configured, gathers the results
            into one structure and returns a reference to that structure. These
            results will also persist in this query object until the next query is
            executed, or clearResults() is called. An alternative callback version of
            this method is also available.
        */
        virtual SceneQueryResult& execute(void) = 0;

        /** Operates just like the other version of execute, except that instead of
            returning details of the query as a list, a listener is called for each
            result of the query.
        */
        virtual void execute(SceneQueryListener* listener) = 0;
        
        /** Gets the results of the last query that was run using this object. */
        virtual SceneQueryResult& getLastResults(void);
        /** Clears the results of the last query execution.
        @remarks
            You only need to call this if you specifically want to free up the memory
            used by this object to hold the last query results. This object clears the
            results itself when executing and when destroying itself.
        */
        virtual void clearResults(void);

        /** Sets the mask for results of this query.
        @remarks
            This method allows you to set a 'mask' to limit the results of this
            query to certain types of result. The actual meaning of this value is
            up to the application; basically MovableObject instances will only be returned
            from this query if a bitwise AND operation between this mask value and the
            MovableObject::getQueryFlags value is non-zero. The application will
            have to decide what each of the bits means.
        */
        virtual void setQueryMask(unsigned long mask);
        /** Returns the current mask for this query. */
        virtual unsigned long getQueryMask(void);

        
    };

    /** Specialises the SceneQuery class for querying within an axis aligned box. */
    class _OgreExport AxisAlignedBoxSceneQuery : public SceneQuery
    {
    protected:
        AxisAlignedBox mAABB;
    public:
        AxisAlignedBoxSceneQuery(SceneManager* mgr);
        virtual ~AxisAlignedBoxSceneQuery();

        /** Sets the size of the box you wish to query. */
        void setBox(const AxisAlignedBox& box);

        /** Gets the box which is being used for this query. */
        const AxisAlignedBox& getBox(void);

    };

    /** Specialises the SceneQuery class for querying within a sphere. */
    class _OgreExport SphereSceneQuery : public SceneQuery
    {
    protected:
        Sphere mSphere;
    public:
        SphereSceneQuery(SceneManager* mgr);
        virtual ~SphereSceneQuery();
        /** Sets the sphere which is to be used for this query. */
        void setSphere(const Sphere& sphere);

        /** Gets the sphere which is being used for this query. */
        const Sphere& getSphere();

    };

    /** Specialises the SceneQuery class for querying along a ray. */
    class _OgreExport RaySceneQuery : public SceneQuery
    {
    };

    /** Specialises the SceneQuery class for querying within a pyramid. */
    class _OgreExport PyramidSceneQuery : public SceneQuery
    {
    };

    /** Specialises the SceneQuery class to query for pairs of objects which are
        possibly intersecting one another.
    @remarks
        This SceneQuery subclass differs from all the others because instead of dealing 
        with a region, it considers the whole world and returns pairs of objects
        which are close enough to each other that they may be intersecting. Because of
        this slightly different focus, the return types and listener interface are
        different for this class. 
    */
    class _OgreExport IntersectionSceneQuery : public SceneQuery
    {
    };
    

}
    


#endif
