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
#include "OgreRay.h"

namespace Ogre {

    typedef std::list<MovableObject*> SceneQueryResultMovableList;
    //typedef std::list<WorldFragment*> SceneQueryResultWorldFragmentList;
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
        /** Called when a MovableObject is returned by a query.
        @remarks
            The implementor should return 'true' to continue returning objects,
            or 'false' to abandon any further results from this query.
        */
        virtual bool queryResult(MovableObject* object) = 0;
        // TODO: world geometry callback
        //virtual bool queryResult(WorldFragment* fragment);
        
    };

    /** A class for performing queries on a scene.
    @remarks
        This is an abstract class for performing a query on a scene, i.e. to retrieve
        a list of objects and/or world geometry sections which are potentially intersecting a
        given region. Note the use of the word 'potentially': the results of a scene query
        are generated based on bounding volumes, and as such are not correct at a triangle
        level; the user of the SceneQuery is expected to filter the results further if
        greater accuracy is required.
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
        SceneManager::createSphereSceneQuery.
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
    protected:
        Ray mRay;
    public:
        RaySceneQuery(SceneManager* mgr);
        virtual ~RaySceneQuery();
        /** Sets the ray which is to be used for this query. */
        void setRay(const Ray& ray);
        /** Gets the ray which is to be used for this query. */
        const Ray& getRay(void);

    };

    /** Specialises the SceneQuery class for querying within a pyramid. */
    class _OgreExport PyramidSceneQuery : public SceneQuery
    {
    public:
        PyramidSceneQuery(SceneManager* mgr);
        virtual ~PyramidSceneQuery();
    };


    /** Alternative listener class for dealing with IntersectionSceneQuery.
    @remarks
        Because the IntersectionSceneQuery returns results in pairs, rather than singularly,
        the listener interface must be customised from the standard SceneQueryListener.
    */
    class _OgreExport IntersectionSceneQueryListener
    {
    public:
        /** Called when 2 movable objects intersect one another.
        @remarks
            As with SceneQueryListener, the implementor of this method should return 'true'
            if further results are required, or 'false' to abandon any further results from
            the current query.
        */
        virtual bool queryResult(MovableObject* first, MovableObject* second);

        /** Called when a movable intersects a world fragment. 
        @remarks
            As with SceneQueryListener, the implementor of this method should return 'true'
            if further results are required, or 'false' to abandon any further results from
            the current query.
        */
        //virtual bool queryResult(MovableObject* movable, WorldFragment* fragment);

        /* NB there are no results for world fragments intersecting other world fragments;
           it is assumed that world geometry is either static or at least that self-intersections
           are irrelevant or dealt with elsewhere (such as the custom scene manager) */
        
    
    };
        
    typedef std::pair<MovableObject*, MovableObject*> SceneQueryMovableObjectPair;
    //typedef std::pair<MovableObject*, WorldFragment*> SceneQueryMovableObjectWorldFragmentPair;
    typedef std::list<SceneQueryMovableObjectPair> SceneQueryMovableIntersectionList;
    //typedef std::list<MovableObjectWorldFragmentPair> SceneQueryMovableWorldFragmentIntersectionList;
    /** Holds the results of an intersection scene query (pair values). */
    struct _OgreExport IntersectionSceneQueryResult
    {
        /// List of movable objects in the query (entities, particle systems etc)
        SceneQueryMovableIntersectionList movables2movables;
        // TODO: add world geometry fragment list
        //SceneQueryMovableWorldFragmentIntersectionList movables2world;
        
        

    };

    /** Separate SceneQuery class to query for pairs of objects which are
        possibly intersecting one another.
    @remarks
        This SceneQuery subclass differs from all the others because instead of dealing 
        with a region, it considers the whole world and returns pairs of objects
        which are close enough to each other that they may be intersecting. Because of
        this slightly different focus, the return types and listener interface are
        different for this class.
    @par
        NB this is not a subclass of SceneQuery because return types and listeners are
        not polymorphic therefore inheritance for the sake of reusing a couple of utility
        methods is not worth it. There is almost certainly a better design than this out there,
        but it's probably overcomplex for this simple case.
    */
    class _OgreExport IntersectionSceneQuery 
    {
    protected:
        IntersectionSceneQueryResult* mLastResult;
        SceneManager* mParentSceneMgr;
        unsigned long mQueryMask;
    public:
        IntersectionSceneQuery(SceneManager* mgr);
        virtual ~IntersectionSceneQuery();

        /** Executes the query, returning the results back in one list.
        @remarks
            This method executes the scene query as configured, gathers the results
            into one structure and returns a reference to that structure. These
            results will also persist in this query object until the next query is
            executed, or clearResults() is called. An alternative callback version of
            this method is also available.
        */
        virtual IntersectionSceneQueryResult& execute(void) = 0;

        /** Operates just like the other version of execute, except that instead of
            returning details of the query as a list, a listener is called for each
            result of the query.
        */
        virtual void execute(IntersectionSceneQueryListener* listener) = 0;

        /** Gets the results of the last query that was run using this object. */
        virtual IntersectionSceneQueryResult& getLastResults(void);
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
    

}
    


#endif
