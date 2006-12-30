/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreUnifiedHighLevelGpuProgram.h"
#include "OgreString.h"
#include "OgreException.h"

namespace Ogre
{
	//-----------------------------------------------------------------------
	UnifiedHighLevelGpuProgram::CmdDelegate UnifiedHighLevelGpuProgram::msCmdDelegate;
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	UnifiedHighLevelGpuProgram::UnifiedHighLevelGpuProgram(
		ResourceManager* creator, const String& name, ResourceHandle handle,
		const String& group, bool isManual, ManualResourceLoader* loader)
		:HighLevelGpuProgram(creator, name, handle, group, isManual, loader)
	{
		if (createParamDictionary("UnifiedHighLevelGpuProgram"))
		{
			setupBaseParamDictionary();

			ParamDictionary* dict = getParamDictionary();

			dict->addParameter(ParameterDef("delegate", 
				"Additional delegate programs containing implementations.",
				PT_STRING),&msCmdDelegate);
		}

	}
	//-----------------------------------------------------------------------
	UnifiedHighLevelGpuProgram::~UnifiedHighLevelGpuProgram()
	{

	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::chooseDelegate() const
	{
		mChosenDelegate.setNull();

		for (StringVector::const_iterator i = mDelegateNames.begin();
			i != mDelegateNames.end(); ++i)
		{
			HighLevelGpuProgramPtr deleg = 
				HighLevelGpuProgramManager::getSingleton().getByName(*i);

			// Silently ignore missing links
			if(!deleg.isNull()
				&& deleg->isSupported())
			{
				mChosenDelegate = deleg;
				break;
			}

		}

	}
	//-----------------------------------------------------------------------
	const HighLevelGpuProgramPtr& UnifiedHighLevelGpuProgram::_getDelegate() const
	{
		if (mChosenDelegate.isNull())
		{
			chooseDelegate();
		}
		return mChosenDelegate;
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::addDelegateProgram(const String& name)
	{
		mDelegateNames.push_back(name);

		// reset chosen delegate
		mChosenDelegate.setNull();

	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::clearDelegatePrograms()
	{
		mDelegateNames.clear();
		mChosenDelegate.setNull();

	}
	//-----------------------------------------------------------------------
	GpuProgramParametersSharedPtr UnifiedHighLevelGpuProgram::createParameters(void)
	{
		return _getDelegate()->createParameters();
	}
	//-----------------------------------------------------------------------
	GpuProgram* UnifiedHighLevelGpuProgram::_getBindingDelegate(void)
	{
		return _getDelegate()->_getBindingDelegate();
	}
	//-----------------------------------------------------------------------
	bool UnifiedHighLevelGpuProgram::isSupported(void) const
	{
		// Supported if one of the delegates is
		return !(_getDelegate().isNull());
	}
	//-----------------------------------------------------------------------
	bool UnifiedHighLevelGpuProgram::isSkeletalAnimationIncluded(void) const
	{
		return _getDelegate()->isSkeletalAnimationIncluded();
	}
	//-----------------------------------------------------------------------
	bool UnifiedHighLevelGpuProgram::isMorphAnimationIncluded(void) const
	{
		return _getDelegate()->isMorphAnimationIncluded();
	}
	//-----------------------------------------------------------------------
	bool UnifiedHighLevelGpuProgram::isPoseAnimationIncluded(void) const
	{
		return _getDelegate()->isPoseAnimationIncluded();
	}
	//-----------------------------------------------------------------------
	bool UnifiedHighLevelGpuProgram::isVertexTextureFetchRequired(void) const
	{
		return _getDelegate()->isVertexTextureFetchRequired();
	}
	//-----------------------------------------------------------------------
	GpuProgramParametersSharedPtr UnifiedHighLevelGpuProgram::getDefaultParameters(void)
	{
		return _getDelegate()->getDefaultParameters();
	}
	//-----------------------------------------------------------------------
	bool UnifiedHighLevelGpuProgram::hasDefaultParameters(void) const
	{
		return _getDelegate()->hasDefaultParameters();
	}
	//-----------------------------------------------------------------------
	bool UnifiedHighLevelGpuProgram::getPassSurfaceAndLightStates(void) const
	{
		return _getDelegate()->getPassSurfaceAndLightStates();
	}
	//-----------------------------------------------------------------------
	bool UnifiedHighLevelGpuProgram::hasCompileError(void) const
	{
		return _getDelegate()->hasCompileError();
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::resetCompileError(void)
	{
		_getDelegate()->resetCompileError();
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::load(bool backgroundThread)
	{
		_getDelegate()->load(backgroundThread);
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::reload(void)
	{
		_getDelegate()->reload();
	}
	//-----------------------------------------------------------------------
	bool UnifiedHighLevelGpuProgram::isReloadable(void) const
	{
		return _getDelegate()->isReloadable();
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::unload(void)
	{
		_getDelegate()->unload();
	}
	//-----------------------------------------------------------------------
	bool UnifiedHighLevelGpuProgram::isLoaded(void) const
	{
		return _getDelegate()->isLoaded();
	}
	//-----------------------------------------------------------------------
	Resource::LoadingState UnifiedHighLevelGpuProgram::isLoading() const
	{
		return _getDelegate()->isLoading();
	}
	//-----------------------------------------------------------------------
	Resource::LoadingState UnifiedHighLevelGpuProgram::getLoadingState() const
	{
		return _getDelegate()->getLoadingState();
	}
	//-----------------------------------------------------------------------
	size_t UnifiedHighLevelGpuProgram::getSize(void) const
	{
		return _getDelegate()->getSize();
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::touch(void)
	{
		_getDelegate()->touch();
	}
	//-----------------------------------------------------------------------
	bool UnifiedHighLevelGpuProgram::isBackgroundLoaded(void) const
	{
		return _getDelegate()->isBackgroundLoaded();
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::setBackgroundLoaded(bool bl)
	{
		_getDelegate()->setBackgroundLoaded(bl);
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::escalateLoading()
	{
		_getDelegate()->escalateLoading();
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::addListener(Resource::Listener* lis)
	{
		_getDelegate()->addListener(lis);
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::removeListener(Resource::Listener* lis)
	{
		_getDelegate()->removeListener(lis);
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::createLowLevelImpl(void)
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
			"This method should never get called!",
			"UnifiedHighLevelGpuProgram::createLowLevelImpl");
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::unloadHighLevelImpl(void)
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
			"This method should never get called!",
			"UnifiedHighLevelGpuProgram::unloadHighLevelImpl");
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::populateParameterNames(GpuProgramParametersSharedPtr params)
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
			"This method should never get called!",
			"UnifiedHighLevelGpuProgram::populateParameterNames");
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::loadFromSource(void)
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
			"This method should never get called!",
			"UnifiedHighLevelGpuProgram::loadFromSource");
	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	String UnifiedHighLevelGpuProgram::CmdDelegate::doGet(const void* target) const
	{
		// Can't do this (not one delegate), shouldn't matter
		return StringUtil::BLANK;
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgram::CmdDelegate::doSet(void* target, const String& val)
	{
		static_cast<UnifiedHighLevelGpuProgram*>(target)->addDelegateProgram(val);
	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	UnifiedHighLevelGpuProgramFactory::UnifiedHighLevelGpuProgramFactory()
	{
	}
	//-----------------------------------------------------------------------
	UnifiedHighLevelGpuProgramFactory::~UnifiedHighLevelGpuProgramFactory()
	{
	}
	//-----------------------------------------------------------------------
	const String& UnifiedHighLevelGpuProgramFactory::getLanguage(void) const
	{
		static String lang = "unified";
		return lang;
	}
	//-----------------------------------------------------------------------
	HighLevelGpuProgram* UnifiedHighLevelGpuProgramFactory::create(ResourceManager* creator, 
		const String& name, ResourceHandle handle,
		const String& group, bool isManual, ManualResourceLoader* loader)
	{
		return new UnifiedHighLevelGpuProgram(creator, name, handle, group, isManual, loader);
	}
	//-----------------------------------------------------------------------
	void UnifiedHighLevelGpuProgramFactory::destroy(HighLevelGpuProgram* prog)
	{
		delete prog;
	}
	//-----------------------------------------------------------------------

}

