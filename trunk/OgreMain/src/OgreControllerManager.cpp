/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreControllerManager.h"

#include "OgreMaterial.h"
#include "OgreLogManager.h"


namespace Ogre {
    //-----------------------------------------------------------------------
    template<> ControllerManager* Singleton<ControllerManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    ControllerManager::ControllerManager()
    {

    }
    //-----------------------------------------------------------------------
    ControllerManager::~ControllerManager()
    {
        clearControllers();
    }
    //-----------------------------------------------------------------------
    Controller* ControllerManager::createController(ControllerValue* src, ControllerValue* dest, ControllerFunction* func)
    {
        Controller* c = new Controller(src, dest, func);

        mControllers.insert(c);
        return c;
    }
    //-----------------------------------------------------------------------
    void ControllerManager::updateAllControllers(void)
    {
        ControllerList::iterator ci;
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
    ControllerValue* ControllerManager::getFrameTimeSource(void)
    {
        return &mFrameTimeController;
    }
    //-----------------------------------------------------------------------
    Controller* ControllerManager::createTextureAnimator(Material::TextureLayer* layer, Real sequenceTime)
    {
        TextureFrameControllerValue* texVal = new TextureFrameControllerValue(layer);
        AnimationControllerFunction* animFunc = new AnimationControllerFunction(sequenceTime);

        return createController(&mFrameTimeController, texVal, animFunc);
    }
    //-----------------------------------------------------------------------
    Controller* ControllerManager::createTextureScroller(Material::TextureLayer* layer, Real uSpeed, Real vSpeed)
    {
        Controller* ret = 0;
        TexCoordModifierControllerValue *uVal, *vVal;
        ScaleControllerFunction *uFunc, *vFunc;

        // Set up 1 or 2 controllers to manage the scrolling texture
        if (uSpeed != 0)
        {
            if (uSpeed == vSpeed)
            {
                // Cool, we can do both scrolls with a single controller
                uVal = new TexCoordModifierControllerValue(layer, true, true);
            }
            else
            {
                // Just do u, v will take a second controller
                uVal = new TexCoordModifierControllerValue(layer, true);
            }
            // Create function: use -speed since we're altering texture coords so they have reverse effect
            uFunc = new ScaleControllerFunction(-uSpeed, true);
            ret = createController(&mFrameTimeController, uVal, uFunc);

        }

        if (vSpeed != 0 && (uSpeed == 0 || vSpeed != uSpeed))
        {
            // Set up a second controller for v scroll
            vVal = new TexCoordModifierControllerValue(layer, false, true);
            // Create function: use -speed since we're altering texture coords so they have reverse effect
            vFunc = new ScaleControllerFunction(-vSpeed, true);
            ret = createController(&mFrameTimeController, vVal, vFunc);
        }

        return ret;
    }
    //-----------------------------------------------------------------------
    Controller* ControllerManager::createTextureRotater(Material::TextureLayer* layer, Real speed)
    {
        TexCoordModifierControllerValue* val;
        ScaleControllerFunction *func;

        // Target value is texture coord rotation
        val = new TexCoordModifierControllerValue(layer, false, false, false, false, true);
        // Function is simple scale (seconds * speed)
        // Use -speed since altering texture coords has the reverse visible effect
        func = new ScaleControllerFunction(-speed, true);

        return createController(&mFrameTimeController, val, func);

    }
    //-----------------------------------------------------------------------
    Controller* ControllerManager::createTextureWaveTransformer(Material::TextureLayer* layer,
        Material::TextureLayer::TextureTransformType ttype, WaveformType waveType, Real base, Real frequency, Real phase, Real amplitude)
    {
        TexCoordModifierControllerValue* val;
        WaveformControllerFunction *func;

        switch (ttype)
        {
        case Material::TextureLayer::TT_TRANSLATE_U:
            // Target value is a u scroll
            val = new TexCoordModifierControllerValue(layer, true);
            break;
        case Material::TextureLayer::TT_TRANSLATE_V:
            // Target value is a v scroll
            val = new TexCoordModifierControllerValue(layer, false, true);
            break;
        case Material::TextureLayer::TT_SCALE_U:
            // Target value is a u scale
            val = new TexCoordModifierControllerValue(layer, false, false, true);
            break;
        case Material::TextureLayer::TT_SCALE_V:
            // Target value is a v scale
            val = new TexCoordModifierControllerValue(layer, false, false, false, true);
            break;
        case Material::TextureLayer::TT_ROTATE:
            // Target value is texture coord rotation
            val = new TexCoordModifierControllerValue(layer, false, false, false, false, true);
            break;
        }
        // Create new wave function for alterations
        func = new WaveformControllerFunction(waveType, base, frequency, phase, amplitude, true);

        return createController(&mFrameTimeController, val, func);
    }
    //-----------------------------------------------------------------------
    ControllerManager& ControllerManager::getSingleton(void)
    {
        return Singleton<ControllerManager>::getSingleton();
    }
    //-----------------------------------------------------------------------
    void ControllerManager::destroyController(Controller* controller)
    {
        ControllerList::iterator i = mControllers.find(controller);
        if (i != mControllers.end())
        {
            mControllers.erase(i);
            delete controller;
        }
    }

}
