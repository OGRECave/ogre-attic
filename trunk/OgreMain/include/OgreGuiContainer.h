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

#ifndef __GuiContainer_H__
#define __GuiContainer_H__

#include "OgrePrerequisites.h"
#include "OgreGuiElement.h"
#include "OgreIteratorWrappers.h"


namespace Ogre {


    /** A 2D element which contains other GuiElement instances.
    @remarks
        This is a specialisation of GuiElement for 2D elements that contain other
        elements. These are also the smallest elements that can be attached directly
        to an Overlay.
    @remarks
        GuiContainers should be managed using GuiManager. This class is responsible for
        instantiating / deleting elements, and also for accepting new types of element
        from plugins etc.
    */
    class _OgreExport GuiContainer : public GuiElement
    {
    public:
        typedef std::map<String, GuiElement*> ChildMap;
        typedef MapIterator<ChildMap> ChildIterator;
        typedef std::map<String, GuiContainer*> ChildContainerMap;
        typedef MapIterator<ChildContainerMap> ChildContainerIterator;
    protected:
        // Map of all children
        ChildMap mChildren;
        // Map of container children (subset of mChildren)
        ChildContainerMap mChildContainers;

		bool mChildrenProcessEvents;
 
    public:
        /// Constructor: do not call direct, use GuiManager::createContainer
        GuiContainer(const String& name);
        virtual ~GuiContainer();

        /** Adds another GuiElement to this container. */
        virtual void addChild(GuiElement* elem);
        /** Adds another GuiElement to this container. */
        virtual void addChildImpl(GuiElement* elem);
        /** Add a nested container to this container. */
        virtual void addChildImpl(GuiContainer* cont);
        /** Removes a named element from this container. */
        virtual void removeChild(const String& name);
        /** Gets the named child of this container. */
        virtual GuiElement* getChild(const String& name);

        void _addChild(GuiElement* elem);
        void _removeChild(GuiElement* elem) { _removeChild(elem->getName()); }
        void _removeChild(const String& name);

        /** Gets an object for iterating over all the children of this object. */
        virtual ChildIterator getChildIterator(void);

        /** Gets an iterator for just the container children of this object.
        @remarks
            Good for cascading updates without having to use RTTI
        */
        virtual ChildContainerIterator getChildContainerIterator(void);

		/** Tell the object and its children to recalculate */
		virtual void _positionsOutOfDate(void);

        /** Overridden from GuiElement. */
        virtual void _update(void);

        /** Overridden from GuiElement. */
        virtual void _notifyZOrder(ushort newZOrder);

        /** Overridden from GuiElement. */
        virtual void _notifyViewport();

        /** Overridden from GuiElement. */
        virtual void _notifyWorldTransforms(const Matrix4& xform);

        /** Overridden from GuiElement. */
	    virtual void _notifyParent(GuiContainer* parent, Overlay* overlay);

        /** Overridden from GuiElement. */
        virtual void _updateRenderQueue(RenderQueue* queue);

        /** Overridden from GuiElement. */
		inline bool isContainer() const
		{ return true; }

		/** Should this container pass events to their children */
		virtual inline bool isChildrenProcessEvents() const
		{ return true; }

		/** Should this container pass events to their children */
		virtual inline void setChildrenProcessEvents(bool val)
		{ mChildrenProcessEvents = val; }

        /** This returns a GuiElement at position x,y. */
		virtual GuiElement* findElementAt(Real x, Real y);		// relative to parent

	    void copyFromTemplate(GuiElement* templateGui);
        virtual GuiElement* clone(const String& instanceName);

    };



}


#endif

