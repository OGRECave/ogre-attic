/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgreTechnique.h"
#include "OgreMaterial.h"
#include "OgrePass.h"


namespace Ogre {
    //-----------------------------------------------------------------------------
    Technique::Technique(Material* parent)
        : mParent(parent), mIsSupported(false)
    {
        // See above, defaults to unsupported until examined
    }
    //-----------------------------------------------------------------------------
    Technique::Technique(Material* parent, const Technique& oth)
        : mParent(parent)
    {
        // Copy using operator=
        *this = oth;
    }
    //-----------------------------------------------------------------------------
    Technique::~Technique()
    {
        removeAllPasses();
    }
    //-----------------------------------------------------------------------------
    bool Technique::isSupported(void)
    {
        return mIsSupported;
    }
    //-----------------------------------------------------------------------------
    void Technique::_compile(bool autoManageTextureUnits)
    {
		// TODO
    }
    //-----------------------------------------------------------------------------
    Pass* Technique::createPass(bool programmable)
    {
		Pass* newPass = new Pass(this, programmable);
		mPasses.push_back(newPass);
		return newPass;
    }
    //-----------------------------------------------------------------------------
    Pass* Technique::getPass(unsigned short index)
    {
		assert(index < mPasses.size() && "Index out of bounds");
		return mPasses[index];
    }
    //-----------------------------------------------------------------------------
    unsigned short Technique::getNumPasses(void)
    {
		return static_cast<unsigned short>(mPasses.size());
    }
    //-----------------------------------------------------------------------------
    void Technique::removePass(unsigned short index)
    {
		assert(index < mPasses.size() && "Index out of bounds");
		Passes::iterator i = mPasses.begin() + index;
		delete *i;
		mPasses.erase(i);
    }
    //-----------------------------------------------------------------------------
    void Technique::removeAllPasses(void)
    {
        Passes::iterator i, iend;
        iend = mPasses.end();
        for (i = mPasses.begin(); i != iend; ++i)
        {
            delete(*i);
        }
        mPasses.clear();
    }
    //-----------------------------------------------------------------------------
    const Technique::PassIterator Technique::getPassIterator(void)
    {
		return PassIterator(mPasses.begin(), mPasses.end());
    }
    //-----------------------------------------------------------------------------
    Technique& Technique::operator=(const Technique& rhs)
    {
		this->mIsSupported = rhs.mIsSupported;
		// copy passes
		removeAllPasses();
		Passes::const_iterator i, iend;
		iend = rhs.mPasses.end();
		for (i = rhs.mPasses.begin(); i != iend; ++i)
		{
			Pass* p = new Pass(this, *(*i));
			mPasses.push_back(p);
		}
		return *this;
    }
    //-----------------------------------------------------------------------------
    void Technique::_load(void)
    {
		assert (mIsSupported && "This technique is not supported");
		// Load each pass
		Passes::iterator i, iend;
		iend = mPasses.end();
		for (i = mPasses.begin(); i != iend; ++i)
		{
			(*i)->_load();
		}
    }
    //-----------------------------------------------------------------------------
    void Technique::_unload(void)
    {
		// Unload each pass
		Passes::iterator i, iend;
		iend = mPasses.end();
		for (i = mPasses.begin(); i != iend; ++i)
		{
			(*i)->_unload();
		}
    }
    //-----------------------------------------------------------------------------
    bool Technique::isLoaded(void)
    {
        return mParent->isLoaded();
    }


}
