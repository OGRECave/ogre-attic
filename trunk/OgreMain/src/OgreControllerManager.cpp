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
#include "OgreStableHeaders.h"
#include "OgreControllerManager.h"

#include "OgreLogManager.h"
#include "OgreTextureUnitState.h"


namespace Ogre {
    //-----------------------------------------------------------------------
    template<> ControllerManager* Singleton<ControllerManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    ControllerManager::ControllerManager()
		: mFrameTimeController(new FrameTimeControllerValue())
    {

    }
    //-----------------------------------------------------------------------
    ControllerManager::~ControllerManager()
    {
        clearControllers();
    }
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createController(
		SharedPtr< ControllerValue<Real> > src, SharedPtr< ControllerValue<Real> > dest, 
		SharedPtr< ControllerFunction<Real> > func)
    {
        Controller<Real>* c = new Controller<Real>(src, dest, func);

        mControllers.insert(c);
        return c;
    }
    //-----------------------------------------------------------------------
    void ControllerManager::updateAllControllers(void)
    {
        ControllerList::const_iterator ci;
        for (ci = mControllers.begin(); ci != mControllers.end(); ++ci)
        {
            (*ci)->update();
        }
    }
    //-----------------------------------------------------------------------
    void ControllerManager::clearControllers(void)
    {
        ControllerList::iterator ci;
        for (ci = mControllers.begin(); ci != mControllers.end(); ++ci)
        {
            delete *ci;
        }
        mControllers.clear();
    }
    //-----------------------------------------------------------------------
    SharedPtr< ControllerValue<Real> > ControllerManager::getFrameTimeSource(void) const
    {
        return mFrameTimeController;
    }
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createTextureAnimator(TextureUnitState* layer, Real sequenceTime)
    {
        SharedPtr< ControllerValue<Real> > texVal(new TextureFrameControllerValue(layer));
        SharedPtr< ControllerFunction<Real> > animFunc(new AnimationControllerFunction(sequenceTime));

        return createController(mFrameTimeController, texVal, animFunc);
    }
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createTextureScroller(TextureUnitState* layer, Real uSpeed, Real vSpeed)
    {
        Controller<Real>* ret = 0;

        // Set up 1 or 2 controllers to manage the scrolling texture
        if (uSpeed != 0)
        {
			SharedPtr< ControllerValue<Real> > uVal;
			SharedPtr< ControllerFunction<Real> > uFunc;

            if (uSpeed == vSpeed)
            {
                // Cool, we can do both scrolls with a single controller
                uVal.bind(new TexCoordModifierControllerValue(layer, true, true));
            }
            else
            {
                // Just do u, v will take a second controller
                uVal.bind(new TexCoordModifierControllerValue(layer, true));
            }
            // Create function: use -speed since we're altering texture coords so they have reverse effect
            uFunc.bind(new ScaleControllerFunction(-uSpeed, true));
            ret = createController(mFrameTimeController, uVal, uFunc);
        }

        if (vSpeed != 0 && (uSpeed == 0 || vSpeed != uSpeed))
        {
			SharedPtr< ControllerValue<Real> > vVal;
			SharedPtr< ControllerFunction<Real> > vFunc;

            // Set up a second controller for v scroll
            vVal.bind(new TexCoordModifierControllerValue(layer, false, true));
            // Create function: use -speed since we're altering texture coords so they have reverse effect
            vFunc.bind(new ScaleControllerFunction(-vSpeed, true));
            ret = createController(mFrameTimeController, vVal, vFunc);
        }

        return ret;
    }
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createTextureRotater(TextureUnitState* layer, Real speed)
    {
        SharedPtr< ControllerValue<Real> > val;
        SharedPtr< ControllerFunction<Real> > func;

        // Target value is texture coord rotation
        val.bind(new TexCoordModifierControllerValue(layer, false, false, false, false, true));
        // Function is simple scale (seconds * speed)
        // Use -speed since altering texture coords has the reverse visible effect
        func.bind(new ScaleControllerFunction(-speed, true));

        return createController(mFrameTimeController, val, func);

    }
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createTextureWaveTransformer(TextureUnitState* layer,
        TextureUnitState::TextureTransformType ttype, WaveformType waveType, Real base, Real frequency, Real phase, Real amplitude)
    {
        SharedPtr< ControllerValue<Real> > val;
        SharedPtr< ControllerFunction<Real> > func;

        switch (ttype)
        {
        case TextureUnitState::TT_TRANSLATE_U:
            // Target value is a u scroll
            val.bind(new TexCoordModifierControllerValue(layer, true));
            break;
        case TextureUnitState::TT_TRANSLATE_V:
            // Target value is a v scroll
            val.bind(new TexCoordModifierControllerValue(layer, false, true));
            break;
        case TextureUnitState::TT_SCALE_U:
            // Target value is a u scale
            val.bind(new TexCoordModifierControllerValue(layer, false, false, true));
            break;
        case TextureUnitState::TT_SCALE_V:
            // Target value is a v scale
            val.bind(new TexCoordModifierControllerValue(layer, false, false, false, true));
            break;
        case TextureUnitState::TT_ROTATE:
            // Target value is texture coord rotation
            val.bind(new TexCoordModifierControllerValue(layer, false, false, false, false, true));
            break;
        }
        // Create new wave function for alterations
        func.bind(new WaveformControllerFunction(waveType, base, frequency, phase, amplitude, true));

        return createController(mFrameTimeController, val, func);
    }
    //-----------------------------------------------------------------------
    ControllerManager& ControllerManager::getSingleton(void)
    {
        return Singleton<ControllerManager>::getSingleton();
    }
    //-----------------------------------------------------------------------
    void ControllerManager::destroyController(Controller<Real>* controller)
    {
        ControllerList::iterator i = mControllers.find(controller);
        if (i != mControllers.end())
        {
            mControllers.erase(i);
            delete controller;
        }
    }
	//-----------------------------------------------------------------------
	Real ControllerManager::getTimeFactor(void) const {
		return static_cast<const FrameTimeControllerValue*>(mFrameTimeController.get())->getTimeFactor();
	}
	//-----------------------------------------------------------------------
	void ControllerManager::setTimeFactor(Real tf) {
		static_cast<FrameTimeControllerValue*>(mFrameTimeController.getPointer())->setTimeFactor(tf);
	}
}
