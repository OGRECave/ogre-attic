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
#include "OgreHardwareBufferManager.h"


namespace Ogre {

    //-----------------------------------------------------------------------
    template<> HardwareBufferManager* Singleton<HardwareBufferManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    HardwareBufferManager::HardwareBufferManager()
    {
    }
    //-----------------------------------------------------------------------
    HardwareBufferManager::~HardwareBufferManager()
    {
        // Destroy everything
        destroyAllDeclarations();
        destroyAllBindings();
        // No need to destroy buffers - they will be destroyed by removal of bindings
    }
    //-----------------------------------------------------------------------
    HardwareBufferManager& HardwareBufferManager::getSingleton(void)
    {
        return Singleton<HardwareBufferManager>::getSingleton();
    }
    //-----------------------------------------------------------------------
    VertexDeclaration* HardwareBufferManager::createVertexDeclaration(void)
    {
        VertexDeclaration* decl = new VertexDeclaration();
        mVertexDeclarations.push_back(decl);
        return decl;
        
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::destroyVertexDeclaration(VertexDeclaration* decl)
    {
        mVertexDeclarations.remove(decl);
        delete decl;
    }
    //-----------------------------------------------------------------------
	VertexBufferBinding* HardwareBufferManager::createVertexBufferBinding(void)
	{
		VertexBufferBinding* ret = new VertexBufferBinding();
		mVertexBufferBindings.push_back(ret);
		return ret;
	}
    //-----------------------------------------------------------------------
	void HardwareBufferManager::destroyVertexBufferBinding(VertexBufferBinding* binding)
	{
		mVertexBufferBindings.remove(binding);
		delete binding;
	}
    //-----------------------------------------------------------------------
    void HardwareBufferManager::destroyAllDeclarations(void)
    {
        VertexDeclarationList::iterator decl;
        for (decl = mVertexDeclarations.begin(); decl != mVertexDeclarations.end(); ++decl)
        {
            delete *decl;
        }
        mVertexDeclarations.clear();

    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::destroyAllBindings(void)
    {
        VertexBufferBindingList::iterator bind;
        for (bind = mVertexBufferBindings.begin(); bind != mVertexBufferBindings.end(); ++bind)
        {
            delete *bind;
        }
        mVertexBufferBindings.clear();
    }


}
