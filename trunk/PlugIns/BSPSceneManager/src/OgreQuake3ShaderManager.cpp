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
#include "OgreQuake3ShaderManager.h"
#include "OgreQuake3Shader.h"
#include "OgreStringVector.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> Quake3ShaderManager *Singleton<Quake3ShaderManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    Quake3ShaderManager::Quake3ShaderManager()
    {
    }
    //-----------------------------------------------------------------------
    Quake3ShaderManager::~Quake3ShaderManager()
    {
    }
    //-----------------------------------------------------------------------
    void Quake3ShaderManager::parseShaderFile(DataChunk& chunk)
    {
        String line;
        Quake3Shader* pShader;
        char tempBuf[512];

        pShader = 0;

        while(!chunk.isEOF())
        {
            line = chunk.getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (pShader == 0)
                {
                    // No current shader
                    // So first valid data should be a shader name
                    pShader = (Quake3Shader*)create(line);
                    // Skip to and over next {
                    chunk.readUpTo(tempBuf, 511, "{");
                }
                else
                {
                    // Already in a shader
                    if (line == "}")
                    {
                        // Finished shader
                        pShader = 0;
                    }
                    else if (line == "{")
                    {
                        // new pass
                        parseNewShaderPass(chunk, pShader);

                    }
                    else
                    {
                        // Attribute
                        parseShaderAttrib(line.toLowerCase(), pShader);
                    }

                }

            }


        }

    }
    //-----------------------------------------------------------------------
    void Quake3ShaderManager::parseAllSources(const String& extension)
    {
        StringVector shaderFiles;
        DataChunk* pChunk;

        std::vector<ArchiveEx*>::iterator i = mVFS.begin();

        // Specific archives
        for (; i != mVFS.end(); ++i)
        {
            shaderFiles = (*i)->getAllNamesLike( "./", extension);
            for (StringVector::iterator si = shaderFiles.begin(); si!=shaderFiles.end(); ++si)
            {
                DataChunk dat; pChunk = &dat;
                (*i)->fileRead(si[0], &pChunk );
                parseShaderFile(dat);
            }

        }
        // search common archives
        for (i = mCommonVFS.begin(); i != mCommonVFS.end(); ++i)
        {
            shaderFiles = (*i)->getAllNamesLike( "./", extension);
            for (StringVector::iterator si = shaderFiles.begin(); si!=shaderFiles.end(); ++si)
            {
                DataChunk dat; pChunk = &dat;
                (*i)->fileRead(si[0], &pChunk );
                parseShaderFile(dat);
            }
        }


    }
    //-----------------------------------------------------------------------
    Resource* Quake3ShaderManager::create( const String& name)
    {
        Quake3Shader* s = new Quake3Shader(name);
        load(s,1);
        return s;
    }
    //-----------------------------------------------------------------------
    void Quake3ShaderManager::parseNewShaderPass(DataChunk& chunk, Quake3Shader* pShader)
    {
        String line;
        int passIdx;

        passIdx = pShader->numPasses;
        pShader->numPasses++;

        // Default pass details
        pShader->pass[passIdx].animNumFrames = 0;
        pShader->pass[passIdx].blend = LBO_REPLACE;
        pShader->pass[passIdx].blendDest = SBF_ZERO;
        pShader->pass[passIdx].blendSrc = SBF_ONE;
        pShader->pass[passIdx].depthFunc = CMPF_LESS_EQUAL;
        pShader->pass[passIdx].flags = 0;
        pShader->pass[passIdx].rgbGenFunc = SHADER_GEN_IDENTITY;
        pShader->pass[passIdx].tcModRotate = 0;
        pShader->pass[passIdx].tcModScale[0] = pShader->pass[passIdx].tcModScale[1] = 1.0;
        pShader->pass[passIdx].tcModScroll[0] = pShader->pass[passIdx].tcModScroll[1] = 0;
        pShader->pass[passIdx].tcModStretchWave = SHADER_FUNC_NONE;
        pShader->pass[passIdx].tcModTransform[0] = pShader->pass[passIdx].tcModTransform[1] = 0;
        pShader->pass[passIdx].tcModTurbOn = false;
        pShader->pass[passIdx].tcModTurb[0] = pShader->pass[passIdx].tcModTurb[1] =
            pShader->pass[passIdx].tcModTurb[2] = pShader->pass[passIdx].tcModTurb[3] = 0;
        pShader->pass[passIdx].texGen = TEXGEN_BASE;
        pShader->pass[passIdx].addressMode = Material::TextureLayer::TAM_WRAP;
        pShader->pass[passIdx].customBlend = false;
        pShader->pass[passIdx].alphaVal = 0;
        pShader->pass[passIdx].alphaFunc = CMPF_ALWAYS_PASS;

        while (!chunk.isEOF())
        {
            line = chunk.getLine();
            // Ignore comments & blanks
            if (line.length() != 0 && line.substr(0,2) != "//")
            {
                if (line == "}")
                {
                    // end of shader
                    return;
                }
                else
                {
                    parseShaderPassAttrib(line.toLowerCase(), pShader, &pShader->pass[passIdx]);
                }
            }


        }
    }
    //-----------------------------------------------------------------------
    void Quake3ShaderManager::parseShaderAttrib( const String& line, Quake3Shader* pShader)
    {
        StringVector vecparams;

        vecparams = line.split(" ");
        StringVector::iterator params = vecparams.begin();

        if (params[0] == "skyparms")
        {
            if (params[1] != "-")
            {
                pShader->farbox = true;
                pShader->farboxName = params[1];
            }
            if (params[2] != "-")
            {
                pShader->skyDome = true;
                pShader->cloudHeight = atof(params[2].c_str());
            }
            // nearbox not supported
        }
        else if (params[0] == "cull")
        {
            if (params[1] == "diable" || params[1] == "none")
            {
                pShader->cullMode = MANUAL_CULL_NONE;
            }
            else if (params[1] == "front")
            {
                pShader->cullMode = MANUAL_CULL_FRONT;
            }
            else if (params[1] == "back")
            {
                pShader->cullMode = MANUAL_CULL_BACK;
            }
        }
        else if (params[0] == "deformvertexes")
        {
            // TODO
        }
        else if (params[0] == "fogparms")
        {
            Real r,g,b;
            r = atof(params[1].c_str());
            g = atof(params[2].c_str());
            b = atof(params[3].c_str());
            pShader->fog = true;
            pShader->fogColour = ColourValue(r,g,b);
            pShader->fogDistance = atof(params[4].c_str());

        }
    }
    //-----------------------------------------------------------------------
    void Quake3ShaderManager::parseShaderPassAttrib( const String& line, Quake3Shader* pShader, Quake3Shader::Pass* pPass)
    {
        StringVector vecparams;

        vecparams = line.split(" ");
        StringVector::iterator params = vecparams.begin();

        // MAP
        if (params[0] == "map")
        {
            pPass->textureName = params[1];
            if (params[1] == "$lightmap")
                pPass->texGen = TEXGEN_LIGHTMAP;
        }
        // CLAMPMAP
        if (params[0] == "clampmap")
        {
            pPass->textureName = params[1];
            if (params[1] == "$lightmap")
                pPass->texGen = TEXGEN_LIGHTMAP;
            pPass->addressMode = Material::TextureLayer::TAM_CLAMP;
        }
        // ANIMMAP
        else if (params[0] == "animmap")
        {
            pPass->animFps = atof(params[1].c_str());
            pPass->animNumFrames = static_cast<unsigned int>( vecparams.size() - 2 );
            for (unsigned int frame = 0; frame < pPass->animNumFrames; ++frame)
            {
                pPass->frames[frame] = params[frame+2];
            }
        }
        // BLENDFUNC
        else if (params[0] == "blendfunc")
        {
            if (params[1] == "add" || params[1] == "gl_add")
            {
                pPass->blend = LBO_ADD;
                pPass->blendDest = SBF_ONE;
                pPass->blendSrc = SBF_ONE;
            }
            else if (params[1] == "filter" || params[1] == "gl_filter")
            {
                pPass->blend = LBO_MODULATE;
                pPass->blendDest = SBF_ZERO;
                pPass->blendSrc = SBF_DEST_COLOUR;
            }
            else if (params[1] == "blend" || params[1] == "gl_blend")
            {
                pPass->blend = LBO_ALPHA_BLEND;
                pPass->blendDest = SBF_ONE_MINUS_SOURCE_ALPHA;
                pPass->blendSrc = SBF_SOURCE_ALPHA;
            }
            else
            {
                // Manual blend
                pPass->blendSrc = convertBlendFunc(params[1]);
                pPass->blendDest = convertBlendFunc(params[2]);
                // Detect common blends
                if (pPass->blendSrc == SBF_ONE && pPass->blendDest == SBF_ZERO)
                    pPass->blend = LBO_REPLACE;
                else if (pPass->blendSrc == SBF_ONE && pPass->blendDest == SBF_ONE)
                    pPass->blend = LBO_ADD;
                else if ((pPass->blendSrc == SBF_ZERO && pPass->blendDest == SBF_SOURCE_COLOUR) ||
                    (pPass->blendSrc == SBF_DEST_COLOUR && pPass->blendDest == SBF_ZERO))
                    pPass->blend = LBO_MODULATE;
                else if (pPass->blendSrc == SBF_SOURCE_ALPHA && pPass->blendDest == SBF_ONE_MINUS_SOURCE_ALPHA)
                    pPass->blend = LBO_ALPHA_BLEND;
                else
                    pPass->customBlend = true;


                // NB other custom blends might not work due to OGRE trying to use multitexture over multipass
            }
        }
        // RGBGEN
        else if (params[0] == "rgbgen")
        {
            // TODO
        }
        // ALPHAGEN
        else if (params[0] == "alphagen")
        {
            // TODO
        }
        // TCGEN
        else if (params[0] == "tcgen")
        {
            if (params[1] == "base")
            {
                pPass->texGen = TEXGEN_BASE;
            }
            else if (params[1] == "lightmap")
            {
                pPass->texGen = TEXGEN_LIGHTMAP;
            }
            else if (params[1] == "environment")
            {
                pPass->texGen = TEXGEN_ENVIRONMENT;
            }
        }
        // TCMOD
        else if (params[0] == "tcmod")
        {
            if (params[1] == "rotate")
            {
                pPass->tcModRotate = -atof(params[2]) / 360; // +ve is clockwise degrees in Q3 shader, anticlockwise complete rotations in Ogre
            }
            else if (params[1] == "scroll")
            {
                pPass->tcModScroll[0] = atof(params[2]);
                pPass->tcModScroll[1] = atof(params[3]);
            }
            else if (params[1] == "scale")
            {
                pPass->tcModScale[0] = atof(params[2]);
                pPass->tcModScale[1] = atof(params[3]);
            }
            else if (params[1] == "stretch")
            {
                if (params[2] == "sin")
                    pPass->tcModStretchWave = SHADER_FUNC_SIN;
                else if (params[2] == "triangle")
                    pPass->tcModStretchWave = SHADER_FUNC_TRIANGLE;
                else if (params[2] == "square")
                    pPass->tcModStretchWave = SHADER_FUNC_SQUARE;
                else if (params[2] == "sawtooth")
                    pPass->tcModStretchWave = SHADER_FUNC_SAWTOOTH;
                else if (params[2] == "inversesawtooth")
                    pPass->tcModStretchWave = SHADER_FUNC_INVERSESAWTOOTH;

                pPass->tcModStretchParams[0] = atof(params[3]);
                pPass->tcModStretchParams[1] = atof(params[4]);
                pPass->tcModStretchParams[2] = atof(params[5]);
                pPass->tcModStretchParams[3] = atof(params[6]);

            }
        }
        // TURB
        else if (params[0] == "turb")
        {
            pPass->tcModTurbOn = true;
            pPass->tcModTurb[0] = atof(params[2]);
            pPass->tcModTurb[1] = atof(params[3]);
            pPass->tcModTurb[2] = atof(params[4]);
            pPass->tcModTurb[3] = atof(params[5]);
        }
        // DEPTHFUNC
        else if (params[0] == "depthfunc")
        {
            // TODO
        }
        // DEPTHWRITE
        else if (params[0] == "depthwrite")
        {
            // TODO
        }
        // ALPHAFUNC
        else if (params[0] == "alphafunc")
        {
            if (params[1] == "gt0")
            {
                pPass->alphaVal = 0;
                pPass->alphaFunc = CMPF_GREATER;
            }
            else if (params[1] == "ge128")
            {
                pPass->alphaVal = 128;
                pPass->alphaFunc = CMPF_GREATER_EQUAL;
            }
            else if (params[1] == "lt128")
            {
                pPass->alphaVal = 128;
                pPass->alphaFunc = CMPF_LESS;
            }
        }



    }
    //-----------------------------------------------------------------------
    SceneBlendFactor Quake3ShaderManager::convertBlendFunc( const String& q3func)
    {
        if (q3func == "gl_one")
        {
            return SBF_ONE;
        }
        else if (q3func == "gl_zero")
        {
            return SBF_ZERO;
        }
        else if (q3func == "gl_dst_color")
        {
            return SBF_DEST_COLOUR;
        }
        else if (q3func == "gl_src_color")
        {
            return SBF_SOURCE_COLOUR;
        }
        else if (q3func == "gl_one_minus_dest_color")
        {
            return SBF_ONE_MINUS_DEST_COLOUR;
        }
        else if (q3func == "gl_src_alpha")
        {
            return SBF_SOURCE_ALPHA;
        }
        else if (q3func == "gl_one_minus_src_alpha")
        {
            return SBF_ONE_MINUS_SOURCE_ALPHA;
        }

        // Default if unrecognised
        return SBF_ONE;

    }
    //-----------------------------------------------------------------------
    Quake3ShaderManager& Quake3ShaderManager::getSingleton(void)
    {
        return Singleton<Quake3ShaderManager>::getSingleton();
    }

}
