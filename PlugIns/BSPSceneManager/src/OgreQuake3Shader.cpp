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

#include "OgreQuake3Shader.h"
#include "OgreSceneManager.h"
#include "OgreMaterial.h"
#include "OgreMath.h"
#include "OgreLogManager.h"

namespace Ogre {


    //-----------------------------------------------------------------------
    Quake3Shader::Quake3Shader(String name)
    {
        mName = name;
        numPasses = 0;
        deformFunc = DEFORM_FUNC_NONE;
        farbox = false;
        skyDome = false;
        flags = 0;
        fog = false;
        cullMode = MANUAL_CULL_BACK;

    }
    //-----------------------------------------------------------------------
    Quake3Shader::~Quake3Shader()
    {
    }
    //-----------------------------------------------------------------------
    void Quake3Shader::load(void)
    {
        // Do nothing
    }
    //-----------------------------------------------------------------------
    void Quake3Shader::unload(void)
    {
        // Do nothing
    }
    //-----------------------------------------------------------------------
    Material* Quake3Shader::createAsMaterial(SceneManager* sm, int lightmapNumber)
    {
        char matName[72];
        sprintf(matName, "%s#%d", mName.c_str(), lightmapNumber);
        Material* mat = sm->createMaterial(matName);

        char msg[256];
        sprintf(msg, "Using Q3 shader %s", mName.c_str());
        LogManager::getSingleton().logMessage(msg, LML_CRITICAL);
        for (int p = 0; p < numPasses; ++p)
        {
            Material::TextureLayer* t;
            // Create basic texture
            if (pass[p].textureName == "$lightmap")
            {
                char lightmapName[16];
                sprintf(lightmapName, "@lightmap%d", lightmapNumber);
                t = mat->addTextureLayer(lightmapName);
            }
            // Animated texture support
            else if (pass[p].animNumFrames > 0)
            {
                Real sequenceTime = pass[p].animNumFrames / pass[p].animFps;
                /* Pre-load textures
                   We need to know if each one was loaded OK since extensions may change for each
                   Quake3 can still include alternate extension filenames e.g. jpg instead of tga
                   Pain in the arse - have to check for each frame as letters<n>.tga for example
                   is different per frame!
                */
                for (unsigned int alt = 0; alt < pass[p].animNumFrames; ++alt)
                {
                    try {
                        TextureManager::getSingleton().load(pass[p].frames[alt]);
                    }
                    catch (...)
                    {
                        // Try alternate extension
                        pass[p].frames[alt] = getAlternateName(pass[p].frames[alt]);
                        try {
                            TextureManager::getSingleton().load(pass[p].frames[alt]);
                        }
                        catch (...)
                        { // stuffed - no texture
                        }
                    }

                }

                t = mat->addTextureLayer("");
                t->setAnimatedTextureName(pass[p].frames, pass[p].animNumFrames, sequenceTime);
                if (t->isBlank())
                {
                    String altNames[32];
                    for (unsigned int alt = 0; alt < pass[p].animNumFrames; ++alt)
                        pass[p].frames[alt] = getAlternateName(pass[p].frames[alt]);
                    t->setAnimatedTextureName(pass[p].frames, pass[p].animNumFrames, sequenceTime);
                }

            }
            else
            {
                t = mat->addTextureLayer(pass[p].textureName);
                // Quake3 can still include alternate extension filenames e.g. jpg instead of tga
                // Pain in the arse - have to check for failure
                if (t->isBlank())
                {
                    t->setTextureName(getAlternateName(pass[p].textureName));
                }
            }
            // Blending
            if (p == 0)
            {
                // scene blend
                mat->setSceneBlending(pass[p].blendSrc, pass[p].blendDest);
                t->setColourOperation(LBO_REPLACE);
            }
            else
            {
                if (pass[p].customBlend)
                {
                    // Fallback for now
                    t->setColourOperation(LBO_MODULATE);
                }
                else
                {
                    // simple layer blend
                    t->setColourOperation(pass[p].blend);
                }
            }
            // Tex coords
            if (pass[p].texGen == TEXGEN_BASE)
            {
                t->setTextureCoordSet(0);
            }
            else if (pass[p].texGen == TEXGEN_LIGHTMAP)
            {
                t->setTextureCoordSet(1);
            }
            else if (pass[p].texGen == TEXGEN_ENVIRONMENT)
            {
                t->setEnvironmentMap(true, true);
            }
            // Tex mod
            // Scale
            t->setTextureUScale(pass[p].tcModScale[0]);
            t->setTextureVScale(pass[p].tcModScale[1]);
            // Procedural mods
            // Custom - don't use mod if generating environment
            // Because I do env a different way it look horrible
            if (pass[p].texGen != TEXGEN_ENVIRONMENT)
            {
                if (pass[p].tcModRotate)
                {
                    t->setRotateAnimation(pass[p].tcModRotate);
                }
                if (pass[p].tcModScroll[0] || pass[p].tcModScroll[1])
                {
                    if (pass[p].tcModTurbOn)
                    {
                        // Turbulent scroll
                        if (pass[p].tcModScroll[0])
                        {
                            t->setTransformAnimation(Material::TextureLayer::TT_TRANSLATE_U, WFT_SINE,
                                pass[p].tcModTurb[0], pass[p].tcModTurb[3], pass[p].tcModTurb[2], pass[p].tcModTurb[1]);
                        }
                        if (pass[p].tcModScroll[1])
                        {
                            t->setTransformAnimation(Material::TextureLayer::TT_TRANSLATE_V, WFT_SINE,
                                pass[p].tcModTurb[0], pass[p].tcModTurb[3], pass[p].tcModTurb[2], pass[p].tcModTurb[1]);
                        }
                    }
                    else
                    {
                        // Constant scroll
                        t->setScrollAnimation(pass[p].tcModScroll[0], pass[p].tcModScroll[1]);
                    }
                }
                if (pass[p].tcModStretchWave != SHADER_FUNC_NONE)
                {
                    WaveformType wft;
                    switch(pass[p].tcModStretchWave)
                    {
                    case SHADER_FUNC_SIN:
                        wft = WFT_SINE;
                        break;
                    case SHADER_FUNC_TRIANGLE:
                        wft = WFT_TRIANGLE;
                        break;
                    case SHADER_FUNC_SQUARE:
                        wft = WFT_SQUARE;
                        break;
                    case SHADER_FUNC_SAWTOOTH:
                        wft = WFT_SAWTOOTH;
                        break;
                    case SHADER_FUNC_INVERSESAWTOOTH:
                        wft = WFT_INVERSE_SAWTOOTH;
                        break;

                    }
                    // Create wave-based stretcher
                    t->setTransformAnimation(Material::TextureLayer::TT_SCALE_U, wft, pass[p].tcModStretchParams[3],
                        pass[p].tcModStretchParams[0], pass[p].tcModStretchParams[2], pass[p].tcModStretchParams[1]);
                    t->setTransformAnimation(Material::TextureLayer::TT_SCALE_V, wft, pass[p].tcModStretchParams[3],
                        pass[p].tcModStretchParams[0], pass[p].tcModStretchParams[2], pass[p].tcModStretchParams[1]);
                }
            }
            // Address mode
            t->setTextureAddressingMode(pass[p].addressMode);
            // Alpha mode
            t->setAlphaRejectSettings(pass[p].alphaFunc, pass[p].alphaVal);

            assert(!t->isBlank());


        }
        // Do farbox (create new material)

        // Do skydome (use this material)
        if (skyDome)
        {
            // Quake3 is always aligned with Z upwards
            Quaternion q;
            q.FromAngleAxis(Math::HALF_PI, Vector3::UNIT_X);
            // Also draw last, and make close to camera (far clip plane is shorter)
            sm->setSkyDome(true, matName, 20 - (cloudHeight / 256 * 18), 12, 2000, false, q);
        }


        // Set culling mode and lighting to defaults
        mat->setCullingMode(CULL_NONE);
        mat->setManualCullingMode(cullMode);
        mat->setLightingEnabled(false);
        return mat;
    }
    String Quake3Shader::getAlternateName(String texName)
    {
        // Get alternative JPG to TGA and vice versa
        size_t pos;
        String ext, base;

        pos = texName.find_last_of(".");
        ext = static_cast<String>(texName.substr(pos, 4)).toLowerCase();
        base = texName.substr(0,pos);
        if (ext == ".jpg")
        {
            return base + ".tga";
        }
        else
        {
            return base + ".jpg";
        }

    }
}
