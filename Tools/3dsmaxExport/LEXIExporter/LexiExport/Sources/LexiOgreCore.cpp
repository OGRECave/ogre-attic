/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn

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

#include "LexiStdAfx.h"
#include <direct.h>

template<> COgreCore* Ogre::Singleton<COgreCore>::ms_Singleton = 0;

COgreCore& COgreCore::getSingleton( void )
{	
	assert( ms_Singleton );  
	return ( *ms_Singleton ); 
}

COgreCore* COgreCore::getSingletonPtr( void )
{ 
	return ms_Singleton; 
}

COgreCore::COgreCore(HWND hwnd)
{

	char* szOldPath;
	szOldPath = _getcwd(NULL, 0);
	char szAppPath[MAX_PATH] = "";
	::GetModuleFileName(NULL,szAppPath,sizeof(szAppPath) - 1);

	Ogre::String cwd(szAppPath);
	Ogre::String fileName, filePath;
	Ogre::StringUtil::splitFilename(cwd, fileName, filePath);

	_chdir(filePath.c_str());
	filePath+=Ogre::String("plugins.cfg");

	m_pRoot = new Ogre::Root(filePath.c_str());

	// setup resources
	Ogre::ConfigFile cf;
	cf.load("resources.cfg");

	// Go through all sections & settings in the file
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}

	configureRenderer(hwnd);

	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup( Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
}

bool COgreCore::configureRenderer(HWND hwnd)
{
	//setup config options
	//Render System
	m_pRenderSystem=NULL;
	Ogre::RenderSystemList::iterator pRend = m_pRoot->getAvailableRenderers()->begin();
	while (pRend != m_pRoot->getAvailableRenderers()->end())
	{
		Ogre::String rName = (*pRend)->getName();
		if ( rName == "OpenGL Rendering Subsystem")
		{
			m_pRenderSystem = *pRend;
			break;
		}
		pRend++;
	}
	if(m_pRenderSystem==NULL)
	{
		Ogre::LogManager::getSingletonPtr()->logMessage("Failed to find Direc3D9 render system. Do you have DirectX installed?");
		return false;
	}

	m_pRoot->setRenderSystem(m_pRenderSystem);

	{
		//Video Mode
		char buffer[80];
		// DX9:
		//_snprintf(buffer, 80, "%d x %d @ %d-bit colour", 640, 480, 16);
		//m_pRenderSystem->setConfigOption("Anti aliasing", "None");
		//m_pRenderSystem->setConfigOption("Floating-point mode", "Fastest");
		//m_pRenderSystem->setConfigOption("Allow NVPerfHUD", "No");

		// GL
		_snprintf(buffer, 80, "%d x %d", 640, 480);
		m_pRenderSystem->setConfigOption("Video Mode", (const char *)buffer);
		m_pRenderSystem->setConfigOption("Colour Depth", "32");
		m_pRenderSystem->setConfigOption("Full Screen", "No");
		m_pRenderSystem->setConfigOption("VSync", "No");	
	} 

	Ogre::LogManager::getSingletonPtr()->logMessage("Ready to validate");

	try {
		// Validate configuration
		m_pRenderSystem->validateConfigOptions();
	} catch(...)
	{
		Ogre::LogManager::getSingletonPtr()->logMessage( "Caught exception in validateConfigOptions");
	}

	Ogre::LogManager::getSingletonPtr()->logMessage( "Render System Validated");


	//initialize Root
	m_pRoot->initialise(false);

	Ogre::NameValuePairList miscParams;
	miscParams["border"] = "none";
	miscParams["left"] = Ogre::StringConverter::toString(162); // the width of maxs tool panel.
	miscParams["top"] = Ogre::StringConverter::toString(0);
	miscParams["parentWindowHandle"] = Ogre::StringConverter::toString((unsigned int)hwnd);
	
	Ogre::RenderWindow* window = m_pRoot->createRenderWindow("Hidden MaxExporter Window", 640, 480, false, &miscParams);
	
	return true;
}
