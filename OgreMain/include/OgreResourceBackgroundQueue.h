/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2005 The OGRE Team
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
#ifndef __ResourceBackgroundQueue_H__
#define __ResourceBackgroundQueue_H__


#include "OgrePrerequisites.h"

namespace Ogre {

	/// Identifier of a background process
	typedef unsigned long BackgroundProcessTicket;

	/** This abstract listener interface lets you get immediate notifications of
		completed background processes instead of having to check ticket statuses.
	@note
		These callbacks occur in the <i>background thread</i>, not the thread
		which you queued your request from. You should only use this method
		if you understand the implications of threading and the use of locks, 
		monitor objects or other such thread safety techniques. If you don't, 
		use the simpler 'ticket' approach and poll the isProcessComplete() method
		to determine when your processes complete.
	*/
	class _OgreExport ResourceBackgroundQueueListener
	{
	public:
		/** Called when a requested operation completes. 
		@note Called in the <i>background thread</i>, not your queueing
		thread, so be careful!
		*/
		virtual void operationCompleted(BackgroundProcessTicket ticket) = 0;
	};
	
	/** This class is used to perform Resource operations in a
		background thread. 
	@remarks
		If threading is enabled, Ogre will create a single background thread
		which can be used to load / unload resources in parallel. Only one
		resource will be processed at once in this background thread, but it
		will be in parallel with the main thread. 
	@par
		The general approach here is that on requesting a background resource
		process, your request is placed on a queue ready for the background
		thread to be picked up, and you will get a 'ticket' back, identifying the 
		request. Your call will then return and your thread can
		proceed, knowing that at some point in the background the operation will 
		be performed. In it's own thread, the resource operation will be performed, 
		and once finished the ticket will be marked as complete. You can check the
		status of tickets by calling isProcessComplete() from your queueing thread. 
		It is also possible to get immediate callbacks on completion, but these
		callbacks happen in the background loading thread (not your calling thread),
		so should only be used if you really understand multithreading. 
	@note
		This class will only perform tasks in a background thread if 
		OGRE_THREAD_SUPPORT is defined to be 1. Otherwise, all methods will
		call their exact equivalents in ResourceGroupManager synchronously. 
	*/
	class _OgreExport ResourceBackgroundQueue
	{
	protected:

	public:
		ResourceBackgroundQueue();
		virtual ~ResourceBackgroundQueue();

		/** Initialise a resource group in the background.
		@see ResourceGroupManager::initialiseResourceGroup
		@param name The name of the resource group to initialise
		@param listener Optional callback interface, take note of warnings in 
			the header and only use if you understand them.
		@returns Ticket identifying the request, use isProcessComplete() to 
			determine if completed if not using listener
		*/
		BackgroundProcessTicket initialiseResourceGroup(const String& name, 
			ResourceBackgroundQueueListener* listener = 0);

		/** Initialise all resource groups which are yet to be initialised in the 
			background.
		@see ResourceGroupManager::intialiseResourceGroup
		@param listener Optional callback interface, take note of warnings in 
			the header and only use if you understand them.
		@returns Ticket identifying the request, use isProcessComplete() to 
			determine if completed if not using listener
		*/
		BackgroundProcessTicket initialiseAllResourceGroups( 
			ResourceBackgroundQueueListener* listener = 0);
		/** Loads a resource group in the background.
		@see ResourceGroupManager::intialiseResourceGroup
		@param listener Optional callback interface, take note of warnings in 
			the header and only use if you understand them.
		@returns Ticket identifying the request, use isProcessComplete() to 
			determine if completed if not using listener
		*/
		BackgroundProcessTicket loadResourceGroup(const String& name, 
			ResourceBackgroundQueueListener* listener = 0);


		/** Load a single resource in the background. 
		@see ResourceManager::load
		@param resType The type of the resource (from ResourceManager::getResourceType())
		@param name The name of the Resource
		@param group The resource group to which this resource will belong
		@param isManual Is the resource to be manually loaded? If so, you should
			provide a value for the loader parameter
		@param loader The manual loader which is to perform the required actions
			when this resource is loaded; only applicable when you specify true
			for the previous parameter. NOTE: must be thread safe!!
        @param loadParams Optional pointer to a list of name/value pairs 
            containing loading parameters for this type of resource.
		*/
		BackgroundProcessTicket load(const String& resType, const String& name, 
            const String& group, bool isManual = false, 
			ManualResourceLoader* loader = 0, const NameValuePairList* loadParams = 0, 
			ResourceBackgroundQueueListener* listener = 0);

		/** Returns whether a previously queued process has completed or not. 
		@param ticket The ticket which was returned when the process was queued
		@returns true if process has completed (or if the ticket is unrecognised), 
			false otherwise
		@note Tickets are not stored onced complete so do not accumulate over time.
		This is why a non-existent ticket will return 'true'.
		*/
		bool isProcessComplete(BackgroundProcessTicket ticket);

	};


}

#endif

