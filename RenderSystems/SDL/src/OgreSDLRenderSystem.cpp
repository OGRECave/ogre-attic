/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
http://www.gnu.org/copyleft/lesser.txt.s
-----------------------------------------------------------------------------
*/

#include "OgreSDLRenderSystem.h"
#include "OgreRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreLight.h"
#include "OgreCamera.h"
#include "OgreSDLTextureManager.h"
#include "OgreSDLWindow.h"

#if OGRE_PLATFORM == PLATFORM_WIN32
#   include <windows.h>
#   include <wingdi.h>
#endif

// Hack 
/// TODO: resolve this properly
#ifndef GL_MIRRORED_REPEAT_IBM 
#   define GL_MIRRORED_REPEAT_IBM 0x8370 
#endif 

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

namespace Ogre {

    SDLRenderSystem::SDLRenderSystem()
    {
        OgreGuard( "SDLRenderSystem::SDLRenderSystem" );

        LogManager::getSingleton().logMessage(getName() + " created.");

        if (SDL_Init( SDL_INIT_VIDEO ) < 0)
        {
            /* XXX Blow up */
            fprintf(stderr, "Error starting SDL!!!\n");
        }

        for( int i=0; i<MAX_LIGHTS; i++ )
            mLights[i] = NULL;

        mWorldMatrix = Matrix4::IDENTITY;
        mViewMatrix = Matrix4::IDENTITY;
        

        atexit(SDL_Quit);

        initConfigOptions();

        mStencilFail = mStencilZFail = mStencilPass = GL_KEEP;
        mStencilFunc = GL_ALWAYS;
        mStencilRef = 0;
        mStencilMask = 0xffffffff;


        OgreUnguard();
    }

    SDLRenderSystem::~SDLRenderSystem()
    {
        // Destroy render windows
        RenderTargetMap::iterator i;
        for (i = mRenderTargets.begin(); i != mRenderTargets.end(); ++i)
        {
            delete i->second;
        }
        mRenderTargets.clear();

        SDL_Quit();

    }

    const String& SDLRenderSystem::getName(void) const
    {
        static String strName("OpenGL Rendering Subsystem");
        return strName;
    }

    void SDLRenderSystem::initConfigOptions(void)
    {
        OgreGuard("SDLRenderSystem::initConfigOptions");
        
        mVideoModes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_OPENGL);
        
        if (mVideoModes == (SDL_Rect **)0)
        {
            Except(999, "Unable to load video modes",
                    "SDLRenderSystem::initConfigOptions");
        }

        ConfigOption optFullScreen;
        ConfigOption optVideoMode;

        // FS setting possiblities
        optFullScreen.name = "Full Screen";
        optFullScreen.possibleValues.push_back("Yes");
        optFullScreen.possibleValues.push_back("No");
        optFullScreen.currentValue = "Yes";
        optFullScreen.immutable = false;

        // Video mode possiblities
        optVideoMode.name = "Video Mode";
        optVideoMode.immutable = false;
        for (int i = 0; mVideoModes[i]; i++)
        {
            char szBuf[16];
            snprintf(szBuf, 16, "%d x %d", mVideoModes[i]->w, mVideoModes[i]->h);
            optVideoMode.possibleValues.push_back(szBuf);
            // Make the first one default
            if (i == 0)
            {
                optVideoMode.currentValue = szBuf;
            }
        }
        
        mOptions[optFullScreen.name] = optFullScreen;
        mOptions[optVideoMode.name] = optVideoMode;

        OgreUnguard();
    }
    
    ConfigOptionMap& SDLRenderSystem::getConfigOptions(void)
    {
        return mOptions;
    }

    void SDLRenderSystem::setConfigOption(const String &name, 
                                          const String &value)
    {
        ConfigOptionMap::iterator it = mOptions.find(name);

        if (it != mOptions.end())
            it->second.currentValue = value;
    }

    String SDLRenderSystem::validateConfigOptions(void)
    {
        // XXX Return an error string if something is invalid
        return "";
    }

    RenderWindow* SDLRenderSystem::initialise(bool autoCreateWindow)
    {
        RenderWindow* autoWindow = NULL;

        //The main startup
        RenderSystem::initialise(autoCreateWindow);

        LogManager::getSingleton().logMessage(
            "******************************\n"
            "*** Starting SDL Subsystem ***\n"
            "******************************");

        SDL_Init(SDL_INIT_VIDEO);
        if (autoCreateWindow)
        {
            bool fullscreen = false;

            ConfigOptionMap::iterator opt = mOptions.find("Full Screen");
            if (opt == mOptions.end())
            {
                Except(999, "Can't find full screen options!",
                        "SDLRenderSystem::initialise");
            }

            if (opt->second.currentValue == "Yes")
                fullscreen = true;
            else
                fullscreen = false;

            int w, h;
            opt = mOptions.find("Video Mode");
            if (opt == mOptions.end())
            {
                Except(999, "Can't find full screen options!",
                        "SDLRenderSystem::initialise");
            }

            std::string val = opt->second.currentValue;
            std::string::size_type pos = val.find("x");
            if (pos == std::string::npos)
            {
                Except(999, "Invalid Video Mode provided",
                        "SDLRenderSystem::initialise");
            }

            w = atoi(val.substr(0, pos).c_str());
            h = atoi(val.substr(pos + 1).c_str());
            autoWindow = createRenderWindow("OGRE Render Window", w, h, 32, fullscreen);
        }
        else
        {
            // XXX What is the else?
        }
        
        // XXX Investigate vSync
        
        LogManager::getSingleton().logMessage(
            "*****************************\n"
            "*** SDL Subsystem Started ***\n"
            "*****************************");

        _setCullingMode( mCullingMode );
        
        return autoWindow;
    }

    void SDLRenderSystem::reinitialise(void)
    {
        this->shutdown();
        this->initialise(true);
    }

    void SDLRenderSystem::shutdown(void)
    {
        RenderSystem::shutdown();

        SDL_Quit();
        LogManager::getSingleton().logMessage("-+-+- SDL Shutting down");
        mStopRendering = true;
    }

    void SDLRenderSystem::startRendering(void)
    {
        OgreGuard("SDLRenderSystem::startRendering");

        static clock_t lastStartTime;
        static clock_t lastEndTime;
        RenderTargetMap::iterator i;

        // Init times to avoid large first-frame time
        lastStartTime = lastEndTime = clock();
        
        RenderSystem::startRendering();
        
        mStopRendering = false;
        while( mRenderTargets.size() && !mStopRendering )
        {
            FrameEvent evt;

            // Do frame start event, only if time has advanced
            // Protects us against over-updating when FPS very high
            clock_t fTime = clock(); // Get current time
            if (fTime != lastStartTime || fTime != lastEndTime)
            {
                evt.timeSinceLastFrame = (float)(fTime - lastStartTime) / CLOCKS_PER_SEC;
                evt.timeSinceLastEvent = (float)(fTime - lastEndTime) / CLOCKS_PER_SEC;
                // Stop rendering if frame callback says so
                if(!fireFrameStarted(evt) || mStopRendering)
                    return;

                // We'll also check here if they decided to shut us down
            }
          


            lastStartTime = fTime;

            // Render a frame during idle time (no messages are waiting)
            for( i = mRenderTargets.begin(); i != mRenderTargets.end(); i++ )
            {
                if( i->second->isActive() )
                {
                    i->second->update();
                }
            }

            // Do frame ended event
            fTime = clock(); // Get current time
            if (lastEndTime != fTime || fTime != lastStartTime)
            {
                evt.timeSinceLastFrame = (float)(fTime - lastEndTime) / CLOCKS_PER_SEC;
                evt.timeSinceLastEvent = (float)(fTime - lastStartTime) / CLOCKS_PER_SEC;
                // Stop rendering if frame callback says so
                if(!fireFrameEnded(evt) || mStopRendering)
                    return;
            }

            lastEndTime = fTime;
        }

        OgreUnguard();
    }

    void SDLRenderSystem::setAmbientLight(float r, float g, float b)
    {
        GLfloat lmodel_ambient[] = {r, g, b, 1.0};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    }

    void SDLRenderSystem::setShadingType(ShadeOptions so)
    {
        switch(so)
        {
        case SO_FLAT:
            glShadeModel(GL_FLAT);
            break;
        default:
            glShadeModel(GL_SMOOTH);
            break;
        }
    }

    void SDLRenderSystem::setTextureFiltering(TextureFilterOptions fo)
    {
        OgreGuard( "SDLRenderSystem::setTextureFiltering" );        

        for (int i = 0; i < _getNumTextureUnits(); i++)
        {
            glActiveTextureARB( GL_TEXTURE0_ARB + i );
            switch( fo )
            {
            case TFO_TRILINEAR:
                glTexParameteri(
                    GL_TEXTURE_2D, 
                    GL_TEXTURE_MAG_FILTER, 
                    GL_LINEAR);

                glTexParameteri(
                    GL_TEXTURE_2D, 
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
                break;

            case TFO_BILINEAR:
                glTexParameteri(
                    GL_TEXTURE_2D, 
                    GL_TEXTURE_MAG_FILTER, 
                    GL_LINEAR);

                glTexParameteri(
                    GL_TEXTURE_2D, 
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_NEAREST);
                break;

            case TFO_NONE:
                glTexParameteri(
                    GL_TEXTURE_2D, 
                    GL_TEXTURE_MAG_FILTER, 
                    GL_NEAREST);

                glTexParameteri(
                    GL_TEXTURE_2D, 
                    GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST);
                break;
            }
        }

        glActiveTextureARB( GL_TEXTURE0_ARB );

        OgreUnguard();
    }

    RenderWindow* SDLRenderSystem::createRenderWindow(
            const String & name, int width, int height, int colourDepth,
            bool fullScreen, int left, int top, bool depthBuffer, 
            RenderWindow* parentWindowHandle)
    {
        if (mRenderTargets.find(name) != mRenderTargets.end())
        {
            Except(
                Exception::ERR_INVALIDPARAMS, 
                "Window with name '" + name + "' already exists",
                "SDLRenderSystem::createRenderWindow" );
        }

        RenderWindow* win = new SDLWindow();
        // Create the window
        win->create(name, width, height, colourDepth, fullScreen,
            left, top, depthBuffer, parentWindowHandle);

        mRenderTargets.insert( RenderTargetMap::value_type( name, win ) );

        if (parentWindowHandle == NULL)
        {
            mTextureManager = new SDLTextureManager();
        }

        // XXX Do more?

        return win;
    }

    //-----------------------------------------------------------------------
    void SDLRenderSystem::destroyRenderWindow(RenderWindow* pWin)
    {
        // Find it to remove from list
        RenderTargetMap::iterator i = mRenderTargets.begin();

        while (i != mRenderTargets.end())
        {
            if (i->second == pWin)
            {
                mRenderTargets.erase(i);
                delete pWin;
                break;
            }
        }
    }

    void SDLRenderSystem::_addLight(Light *lt)
    {
        // Find first free slot
        int i;
        for (i =0; i < MAX_LIGHTS; ++i)
        {
            if (!mLights[i])
            {
                mLights[i] = lt;
                break;
            }
        }
        // No space in array?

        if (i == MAX_LIGHTS)
            Except(
                999, 
                "No free light slots - cannot add light.", 
                "SDLRenderSystem::addLight" );

        setGLLight(i, lt);
    }

    void SDLRenderSystem::_modifyLight(Light *lt)
    {
        // Locate light in list
        int lightIndex;
        int i;
        for (i = 0; i < MAX_LIGHTS; ++i)
        {
            if (mLights[i] == lt)
            {
                lightIndex = i;
                break;
            }
        }

        if (i == MAX_LIGHTS)
            Except(
                Exception::ERR_INVALIDPARAMS, 
                "Cannot locate light to modify.",
                "SDLRenderSystem::_modifyLight" );

        setGLLight(lightIndex, lt);
    }

    void SDLRenderSystem::setGLLight(int index, Light* lt)
    {
        GLint gl_index = GL_LIGHT0 + index;

        if (lt->isVisible())
        {
            switch (lt->getType())
            {
            case Light::LT_SPOTLIGHT:
                glLightf( gl_index, GL_SPOT_CUTOFF, lt->getSpotlightOuterAngle() );
                break;
            default:
                glLightf( gl_index, GL_SPOT_CUTOFF, 180.0 );
                break;
            }

            // Color
            ColourValue col;
            col = lt->getDiffuseColour();


            GLfloat f4vals[4] = {col.r, col.g, col.b, col.a};
            glLightfv(gl_index, GL_DIFFUSE, f4vals);
            
            col = lt->getSpecularColour();
            f4vals[0] = col.r;
            f4vals[1] = col.g;
            f4vals[2] = col.b;
            f4vals[3] = col.a;
            glLightfv(gl_index, GL_SPECULAR, f4vals);

            // Disable ambient light for movables
            glLighti(gl_index, GL_AMBIENT, 0);

            // Position (don't set for directional)
            Vector3 vec;
            if (lt->getType() != Light::LT_DIRECTIONAL)
            {
            
                vec = lt->getDerivedPosition();
                f4vals[0] = vec.x;
                f4vals[1] = vec.y;
                f4vals[2] = vec.z;
                f4vals[3] = 1.0;
                glLightfv(gl_index, GL_POSITION, f4vals);
            }
            // Direction (not needed for point lights)
            if (lt->getType() != Light::LT_POINT)
            {
           
                vec = lt->getDerivedDirection();
                f4vals[0] = vec.x;
                f4vals[1] = vec.y;
                f4vals[2] = vec.z;
                f4vals[3] = 0.0;
                glLightfv(gl_index, GL_SPOT_DIRECTION, f4vals);
            }

            // Attenuation
            glLightf(gl_index, GL_CONSTANT_ATTENUATION, lt->getAttenuationConstant());
            glLightf(gl_index, GL_LINEAR_ATTENUATION, lt->getAttenuationLinear());
            glLightf(gl_index, GL_QUADRATIC_ATTENUATION, lt->getAttenuationQuadric());
            // Enable in the scene
            glEnable(gl_index);

        }
        else
        {
            // Disable in the scene
            glDisable(gl_index);
        }

        lt->_clearModified();
    }

    void SDLRenderSystem::_removeLight(Light *lt)
    {
        // Remove & disable light
        for (int i = 0; i < MAX_LIGHTS; ++i)
        {
            if (mLights[i] == lt)
            {
                glDisable(GL_LIGHT0 + i);
                mLights[i] = NULL;
                return;
            }
        }
    }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_removeAllLights(void)
    {
        // Remove & disable all lights
        for (int i = 0; i < MAX_LIGHTS; ++i)
        {
            if (mLights[i])
            {
                glDisable(GL_LIGHT0 + i);
                mLights[i] = NULL;
            }
        }
    }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_pushRenderState(void)
    {
        Except(Exception::UNIMPLEMENTED_FEATURE,
            "Sorry, this feature is not yet available.",
            "SDLRenderSystem::_pushRenderState");
    }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_popRenderState(void)
    {
        Except(Exception::UNIMPLEMENTED_FEATURE,
            "Sorry, this feature is not yet available.",
            "SDLRenderSystem::_popRenderState");
    }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m)
    {
        int x = 0;
        for (int i=0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                gl_matrix[x] = m[j][i];
                x++;
            }
        }
    }

    void SDLRenderSystem::_setWorldMatrix( const Matrix4 &m )
    {
        GLfloat mat[16];
        mWorldMatrix = m;
        makeGLMatrix( mat, mViewMatrix * mWorldMatrix );
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(mat);
    }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setViewMatrix( const Matrix4 &m )
    {
        mViewMatrix = m;

        GLfloat mat[16];
        makeGLMatrix(mat, mViewMatrix);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(mat);

        /* Set the lights here everytime the view
         * matrix changes.
         */
        setLights();
         
        makeGLMatrix(mat, mWorldMatrix);
        glMultMatrixf(mat);
    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setProjectionMatrix(const Matrix4 &m)
    {
        GLfloat mat[16];
        makeGLMatrix(mat, m);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(mat);
        glMatrixMode(GL_MODELVIEW);
    }


    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setSurfaceParams(ColourValue &ambient,
        ColourValue &diffuse, ColourValue &specular,
        ColourValue &emissive, Real shininess)
    {
        // XXX Cache previous values?
        // XXX Front or Front and Back?

        GLfloat f4val[4] = {diffuse.r, diffuse.g, diffuse.b, diffuse.a};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, f4val);
        f4val[0] = ambient.r;
        f4val[1] = ambient.g;
        f4val[2] = ambient.b;
        f4val[3] = ambient.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, f4val);
        f4val[0] = specular.r;
        f4val[1] = specular.g;
        f4val[2] = specular.b;
        f4val[3] = specular.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, f4val);
        f4val[0] = emissive.r;
        f4val[1] = emissive.g;
        f4val[2] = emissive.b;
        f4val[3] = emissive.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, f4val);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }

    //-----------------------------------------------------------------------------
    unsigned short SDLRenderSystem::_getNumTextureUnits(void)
    {
        #ifdef OGRE_SDL_USE_MULTITEXTURING
            GLint units;
            glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &units );
            return (unsigned short)units;
        #else
            return 1;
        #endif

    }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setTexture(int stage, bool enabled, const String &texname)
    {
        SDLTexture* tex = (SDLTexture*)TextureManager::getSingleton().getByName(texname);

        glActiveTextureARB( GL_TEXTURE0_ARB + stage );
        if (enabled && tex)
        {
            glEnable( GL_TEXTURE_2D );
            glBindTexture( GL_TEXTURE_2D, tex->getGLID() );
        }
        else
        {
            glDisable( GL_TEXTURE_2D );
        }
        glActiveTextureARB( GL_TEXTURE0_ARB );
    }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setTextureCoordSet(int stage, int index)
    {
        mTextureCoordIndex[stage] = index;
    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setTextureCoordCalculation(int stage, TexCoordCalcMethod m)
    {
        glActiveTextureARB( GL_TEXTURE0_ARB + stage );

        switch( m )
        {
        case TEXCALC_NONE:
            glDisable( GL_TEXTURE_GEN_S );
            glDisable( GL_TEXTURE_GEN_T );
            break;

        case TEXCALC_ENVIRONMENT_MAP:
#ifdef GL_VERSION_1_3
            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP );
#else
            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
#endif
            glEnable( GL_TEXTURE_GEN_S );
            glEnable( GL_TEXTURE_GEN_T );
            break;

        case TEXCALC_ENVIRONMENT_MAP_PLANAR:            
            // XXX This doesn't seem right?!
#ifdef GL_VERSION_1_3
            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
#else
            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
#endif
            glEnable( GL_TEXTURE_GEN_S );
            glEnable( GL_TEXTURE_GEN_T );
            break;
        }

        glActiveTextureARB( GL_TEXTURE0_ARB );
    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setTextureBlendMode(int stage, const LayerBlendModeEx& bm)
    {       
        glActiveTextureARB(GL_TEXTURE0_ARB + stage);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
//
//        std::cout << "Need to do blend op: " << bm.operation << std::endl;
//        std::cout << "Type: " << bm.blendType << std::endl;
//        std::cout << "Src1: " << bm.source1 << std::endl;
//        std::cout << "Src2: " << bm.source2 << std::endl;

        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR); 
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA); 

        GLenum type, src1, src2, src1op, src2op, cmd;
        if (bm.blendType == LBT_COLOUR)
        {
            type = GL_COMBINE_RGB_EXT;
            src1 = GL_SOURCE0_RGB_EXT;
            src2 = GL_SOURCE1_RGB_EXT;
        }
        else
        {
            type = GL_COMBINE_ALPHA_EXT;
            src1 = GL_SOURCE0_ALPHA_EXT;
            src2 = GL_SOURCE1_ALPHA_EXT;
        }

        switch (bm.source1)
        {
        case LBS_CURRENT:
            src1op = GL_PREVIOUS_EXT;
            break;
        case LBS_TEXTURE:
            src1op = GL_TEXTURE;
            break;
        // XXX
        case LBS_DIFFUSE:
            src1op = 0;
            break;
        case LBS_SPECULAR:
            src1op = 0;
            break;
        case LBS_MANUAL:
            src1op = 0;
        };

        switch (bm.source2)
        {
        case LBS_CURRENT:
            src2op = GL_PREVIOUS_EXT;
            break;
        case LBS_TEXTURE:
            src2op = GL_TEXTURE;
            break;
        // XXX
        case LBS_DIFFUSE:
            src2op = 0;
            break;
        case LBS_SPECULAR:
            src2op = 0;
            break;
        case LBS_MANUAL:
            src2op = 0;
        };

        switch (bm.operation)
        {
        case LBX_SOURCE1:
            cmd = GL_REPLACE;
            break;
        case LBX_SOURCE2:
            cmd = GL_REPLACE;
            break;
        case LBX_MODULATE:
            cmd = GL_MODULATE;
            break;
        case LBX_MODULATE_X2:
            cmd = GL_MODULATE;
            glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ?
                    GL_RGB_SCALE_EXT : GL_ALPHA_SCALE, 2);
            break;
        case LBX_MODULATE_X4:
            cmd = GL_MODULATE;
            glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ?
                    GL_RGB_SCALE_EXT : GL_ALPHA_SCALE, 4);
            break;
        case LBX_ADD:
            cmd = GL_ADD;
            break;
        case LBX_ADD_SIGNED:
            cmd = GL_ADD_SIGNED_EXT;
            break;
#if 0
        case LBX_ADD_SMOOTH:
            value = D3DTOP_ADDSMOOTH;
            break;
        case LBX_SUBTRACT:
            value = D3DTOP_SUBTRACT;
            break;
        case LBX_BLEND_DIFFUSE_ALPHA:
            value = D3DTOP_BLENDDIFFUSEALPHA;
            break;
#endif
        case LBX_BLEND_TEXTURE_ALPHA:
            cmd = GL_INTERPOLATE_EXT;
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_TEXTURE);
            break;
        case LBX_BLEND_CURRENT_ALPHA:
            cmd = GL_INTERPOLATE_EXT;
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PREVIOUS_EXT);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_PREVIOUS_EXT);
            break;
#if 0
        case LBX_BLEND_MANUAL:
            value = D3DTOP_BLENDFACTORALPHA;
            // Set factor in render state
            hr = mlpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR,
                D3DRGBA(0,0,0,bm.factor));
            break;
#endif
        default:
            printf("Unhandled type %d\n", bm.operation);
        }

        //printf("Blending type(%x), cmd(%x), src1(%x), src1op(%x), src2(%x), src2op(%x)\n)", type, cmd, src1, src1op, src2, src2op);
        glTexEnvi(GL_TEXTURE_ENV, type, cmd);
        glTexEnvi(GL_TEXTURE_ENV, src1, src1op);
        glTexEnvi(GL_TEXTURE_ENV, src2, src2op);

        glActiveTextureARB(GL_TEXTURE0_ARB);
    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setTextureAddressingMode(int stage, Material::TextureLayer::TextureAddressingMode tam)
    {
        GLint type;
        switch(tam)
        {
        case Material::TextureLayer::TAM_WRAP:
            type = GL_REPEAT;
            break;
        case Material::TextureLayer::TAM_MIRROR:
            type = GL_MIRRORED_REPEAT_IBM;
            break;
        case Material::TextureLayer::TAM_CLAMP:
            type = GL_CLAMP;
            break;
        }

        glActiveTextureARB( GL_TEXTURE0_ARB + stage );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, type );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, type );
        glActiveTextureARB( GL_TEXTURE0_ARB );
    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setTextureMatrix(int stage, const Matrix4& xform)
    {
        GLfloat mat[16];
        makeGLMatrix(mat, xform);

        if (mat[8] != 0 || mat[9] != 0)
        {
            mat[12] = mat[8];
            mat[13] = mat[9];
            //mat[14] = mat[10];
            //mat[15] = mat[11];

//            mat[8] = 0;
//            mat[9] = 0;
//            mat[10] = 1;
//            mat[11] = 0;
        }

//        for (int j=0; j< 4; j++)
//        {
//            int x = 0;
//            for (x = 0; x < 4; x++)
//            {
//                printf("[%2d]=%0.2f\t", (x*4) + j, mat[(x*4) + j]);
//            }
//            printf("\n");
//        }
//        printf("\n");

        glActiveTextureARB(GL_TEXTURE0_ARB + stage);
        glMatrixMode(GL_TEXTURE);
        glLoadMatrixf(mat);
        glMatrixMode(GL_MODELVIEW);
        glActiveTextureARB(GL_TEXTURE0_ARB);
    }
    //-----------------------------------------------------------------------------
    GLint SDLRenderSystem::getBlendMode(SceneBlendFactor ogreBlend)
    {
        switch(ogreBlend)
        {
        case SBF_ONE:
            return GL_ONE;
        case SBF_ZERO:
            return GL_ZERO;
        case SBF_DEST_COLOUR:
            return GL_DST_COLOR;
        case SBF_SOURCE_COLOUR:
            return GL_SRC_COLOR;
        case SBF_ONE_MINUS_DEST_COLOUR:
            return GL_ONE_MINUS_DST_COLOR;
        case SBF_ONE_MINUS_SOURCE_COLOUR:
            return GL_ONE_MINUS_SRC_COLOR;
        case SBF_DEST_ALPHA:
            return GL_DST_ALPHA;
        case SBF_SOURCE_ALPHA:
            return GL_SRC_ALPHA;
        case SBF_ONE_MINUS_DEST_ALPHA:
            return GL_ONE_MINUS_DST_ALPHA;
        case SBF_ONE_MINUS_SOURCE_ALPHA:
            return GL_ONE_MINUS_SRC_ALPHA;
        };
		// to keep compiler happy
		return GL_ONE;
    }

    void SDLRenderSystem::_setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor)
    {
        GLint sourceBlend = getBlendMode(sourceFactor);
        GLint destBlend = getBlendMode(destFactor);
        
        glEnable(GL_BLEND);
        glBlendFunc(sourceBlend, destBlend);
    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value)
    {
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(convertCompareFunction(func), value / 128.0f);
    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setViewport(Viewport *vp)
    {
        // Check if viewport is different
        if (vp != mActiveViewport || vp->_isUpdated())
        {
              mActiveViewport = vp;
  
              // XXX Rendering target stuff?
              GLsizei x, y, w, h;
  
              RenderTarget* target;
              target = vp->getTarget();
  
              // Calculate the "lower-left" corner of the viewport
              w = vp->getActualWidth();
              h = vp->getActualHeight();
              x = vp->getActualLeft();
              y = target->getHeight() - vp->getActualTop() - h;
  
              glViewport(x, y, w, h);
  
              // Configure the viewport clipping
              glScissor(x, y, w, h);
  
              fprintf(stderr, "Reset perspective\n");
  
              vp->_clearUpdatedFlag();
        }
    }
  void SDLRenderSystem::setLights()
  {
        GLfloat f4vals[4];
        for (int i = 0; i < MAX_LIGHTS; ++i)
        {
            if (mLights[i] != NULL)
            {
                Light* lt = mLights[i];
                // Position (don't set for directional)
                Vector3 vec;
                if (lt->getType() != Light::LT_DIRECTIONAL)
                {
                    vec = lt->getDerivedPosition();
                    f4vals[0] = vec.x;
                    f4vals[1] = vec.y;
                    f4vals[2] = vec.z;
                    f4vals[3] = 1.0;
                    glLightfv(GL_LIGHT0 + i, GL_POSITION, f4vals);
                }
                // Direction (not needed for point lights)
                if (lt->getType() != Light::LT_POINT)
                {
                    vec = lt->getDerivedDirection();
                    f4vals[0] = vec.x;
                    f4vals[1] = vec.y;
                    f4vals[2] = vec.z;
                    f4vals[3] = 0.0;
                    glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, f4vals);
                }
            }
        }

  }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_beginFrame(void)
    {
        OgreGuard( "SDLRenderSystem::_beginFrame" );
        
        if (!mActiveViewport)
            Except(999, "Cannot begin frame - no viewport selected.",
                "SDLRenderSystem::_beginFrame");

        // Clear the viewport if required
        if (mActiveViewport->getClearEveryFrame())
        {
            // Activate the viewport clipping
            glEnable(GL_SCISSOR_TEST);

            ColourValue col = mActiveViewport->getBackgroundColour();
            
            glClearColor(col.r, col.g, col.b, col.a);
            // Enable depth buffer for writing if it isn't
         
            if (!mDepthWrite)
            {
              glDepthMask( GL_TRUE );
            }
            // Clear buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Reset depth write state if appropriate
            // Enable depth buffer for writing if it isn't
            if (!mDepthWrite)
            {
              glDepthMask( GL_FALSE );
            }

        }        

        setLights();
        OgreUnguard();
    }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_render(RenderOperation& op)
    {
        OgreGuard("SDLRenderSystem::_render");
        
        RenderSystem::_render(op);

        if (op.vertexOptions == 0)
        {
            // Must include at least vertex normal, colour or tex coords
            Except(999, 
                "You must specify at least vertex normals, "
                "vertex colours or texture co-ordinates to render.", 
                "SDLRenderSystem::_render" );
        }

        // Setup the vertex array
        glEnableClientState( GL_VERTEX_ARRAY );
        unsigned short stride = op.vertexStride ? 
            op.vertexStride + (sizeof(GL_FLOAT) * 3) : 0;
        glVertexPointer( 3, GL_FLOAT, stride, op.pVertices );

        // Normals if available
        if (op.vertexOptions & RenderOperation::VO_NORMALS)
        {
            glEnableClientState( GL_NORMAL_ARRAY );
            stride = op.normalStride ?  op.normalStride + (sizeof(GL_FLOAT) * 3) : 0;
            glNormalPointer( GL_FLOAT, stride, op.pNormals );
        }
        else
        {
            glDisableClientState( GL_NORMAL_ARRAY );
        }

        // Color
        if (op.vertexOptions & RenderOperation::VO_DIFFUSE_COLOURS)
        {
            glEnableClientState(GL_COLOR_ARRAY);
            stride = op.diffuseStride ?  
                            op.diffuseStride + (sizeof(unsigned char) * 4) : 0;
            glColorPointer( 4, GL_UNSIGNED_BYTE, stride, op.pDiffuseColour );
        }
        else
        {
            glDisableClientState(GL_COLOR_ARRAY);
            glColor4f(1,1,1,1);
        }
        
        // Textures if available
        if (op.vertexOptions & RenderOperation::VO_TEXTURE_COORDS)
        {
            GLint index = GL_TEXTURE0_ARB;

            for (int i = 0; i < _getNumTextureUnits(); i++)
            {
                if( i < op.numTextureCoordSets )
                {                
                    glClientActiveTextureARB(index + i);
                    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
                    stride = op.texCoordStride[mTextureCoordIndex[i]] ?  
                        op.texCoordStride[mTextureCoordIndex[i]] +
                        (sizeof(GL_FLOAT) * 
                         op.numTextureDimensions[mTextureCoordIndex[i]])
                        : 0;
                    glTexCoordPointer(
                        op.numTextureDimensions[mTextureCoordIndex[i]],
                        GL_FLOAT, stride, 
                        op.pTexCoords[mTextureCoordIndex[i]] );
                }
                else
                {
                   glClientActiveTextureARB( index + i );
                   glDisableClientState( GL_TEXTURE_COORD_ARRAY );
                }
            }

            // Reset the texture to 0
            glClientActiveTextureARB(index);
        }
        else
        {
            glClientActiveTextureARB( GL_TEXTURE0_ARB );
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        }

        // Find the correct type to render
        GLint primType;
        switch (op.operationType)
        {
        case RenderOperation::OT_POINT_LIST:
            primType = GL_POINTS;
            break;
        case RenderOperation::OT_LINE_LIST:
            primType = GL_LINES;
            break;
        case RenderOperation::OT_LINE_STRIP:
            primType = GL_LINE_STRIP;
            break;
        case RenderOperation::OT_TRIANGLE_LIST:
            primType = GL_TRIANGLES;
            break;
        case RenderOperation::OT_TRIANGLE_STRIP:
            primType = GL_TRIANGLE_STRIP;
            break;
        case RenderOperation::OT_TRIANGLE_FAN:
            primType = GL_TRIANGLE_FAN;
            break;
        }

        if (op.useIndexes)
        {
            glDrawElements(
                primType, 
                op.numIndexes, 
                GL_UNSIGNED_SHORT,
                op.pIndexes);
        }
        else
        {
            glDrawArrays( primType, 0, op.numVertices );
        }

     
        OgreUnguard();
    }
    
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_endFrame(void)
    {
        // XXX Do something?
    }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setCullingMode(CullingMode mode)
    {
        GLint cullMode;

        switch( mode )
        {
        case CULL_NONE:
            glDisable( GL_CULL_FACE );
            return;
        case CULL_CLOCKWISE:
            cullMode = GL_CCW;
            break;
        case CULL_ANTICLOCKWISE:
            cullMode = GL_CW;
            break;
        }

        glEnable( GL_CULL_FACE );
        glFrontFace( cullMode );
    }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction)
    {
        _setDepthBufferCheckEnabled(depthTest);
        _setDepthBufferWriteEnabled(depthWrite);
        _setDepthBufferFunction(depthFunction);

    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setDepthBufferCheckEnabled(bool enabled)
    {
        if (enabled)
        {
            glClearDepth(1.0f);
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setDepthBufferWriteEnabled(bool enabled)
    {
        GLboolean flag = enabled ? GL_TRUE : GL_FALSE;
        glDepthMask( flag );  
        // Store for reference in _beginFrame
        mDepthWrite = enabled;
    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setDepthBufferFunction(CompareFunction func)
    {
        glDepthFunc(convertCompareFunction(func));
    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setDepthBias(ushort bias)
    {
        if (bias > 0)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glEnable(GL_POLYGON_OFFSET_POINT);
            glEnable(GL_POLYGON_OFFSET_LINE);
            // Bias is in {0, 16}, scale the unit addition appropriately
            glPolygonOffset(1.0f, bias / 16);
        }
        else
        {
            glDisable(GL_POLYGON_OFFSET_FILL);
            glDisable(GL_POLYGON_OFFSET_POINT);
            glDisable(GL_POLYGON_OFFSET_LINE);
        }
    }
    //-----------------------------------------------------------------------------
    String SDLRenderSystem::getErrorDescription(long errCode)
    {
        // XXX FIXME

        return String("Uknown Error");
    }

    //-----------------------------------------------------------------------------
    void SDLRenderSystem::setLightingEnabled(bool enabled)
    {
        if (enabled)
            glEnable(GL_LIGHTING);
        else
            glDisable(GL_LIGHTING);
    }
    //-----------------------------------------------------------------------------
    void SDLRenderSystem::_setFog(FogMode mode, ColourValue colour, Real density, Real start, Real end)
    {

        GLint fogMode;
        switch (mode)
        {
        case FOG_EXP:
            fogMode = GL_EXP;
            break;
        case FOG_EXP2:
            fogMode = GL_EXP2;
            break;
        case FOG_LINEAR:
            fogMode = GL_LINEAR;
            break;
        default:
            // Give up on it
            glDisable(GL_FOG);
            return;
        }

        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, fogMode);
        GLfloat fogColor[4] = {colour.r, colour.g, colour.b, colour.a};
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_DENSITY, density);
        glFogf(GL_FOG_START, start);
        glFogf(GL_FOG_END, end);
        // XXX Hint here?
    }

    void SDLRenderSystem::convertColourValue(const ColourValue& colour, unsigned long* pDest)
    {
        // GL accesses by byte, so use ABGR so little-endian format will make it RGBA in byte mode
        *pDest = colour.getAsLongABGR();
    }
    
    void SDLRenderSystem::_makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, 
        Real farPlane, Matrix4& dest)
    {
        Real thetaY = Math::DegreesToRadians(fovy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);
        //Real thetaX = thetaY * aspect;
        //Real tanThetaX = Math::Tan(thetaX);

        // Calc matrix elements
        Real w = (1.0f / tanThetaY) / aspect;
        Real h = 1.0f / tanThetaY;
        Real q = -(farPlane + nearPlane) / (farPlane - nearPlane);
        //Real qn= q * mNearDist;
        Real qn = -2 * (farPlane * nearPlane) / (farPlane - nearPlane);

        // NB This creates Z in range [-1,1]
        //
        // [ w   0   0   0  ]
        // [ 0   h   0   0  ]
        // [ 0   0   q   qn ]
        // [ 0   0   -1  0  ]

        dest = Matrix4::ZERO;
        dest[0][0] = w;
        dest[1][1] = h;
        dest[2][2] = q;
        dest[2][3] = qn;
        dest[3][2] = -1;

    }
    
    void SDLRenderSystem::_setRasterisationMode(SceneDetailLevel level)
    {
        GLenum glmode;
        switch(level)
        {
        case SDL_POINTS:
            glmode = GL_POINT;
            break;
        case SDL_WIREFRAME:
            glmode = GL_LINE;
            break;
        case SDL_SOLID:
            glmode = GL_FILL;
            break;

            // Deactivate the viewport clipping.
            glDisable(GL_SCISSOR_TEST);
        }

        glPolygonMode(GL_FRONT_AND_BACK, glmode);

    }
    //---------------------------------------------------------------------
    void SDLRenderSystem::setStencilCheckEnabled(bool enabled)
    {
        if (enabled)
        {
            glEnable(GL_STENCIL_TEST);
        }
        else
        {
            glDisable(GL_STENCIL_TEST);
        }
        
    }
    //---------------------------------------------------------------------
    bool SDLRenderSystem::hasHardwareStencil(void)
    {
        // TODO
        return true;
    }
    //---------------------------------------------------------------------
    ushort SDLRenderSystem::getStencilBufferBitDepth(void)
    {
        return 8;
    }
    //---------------------------------------------------------------------
    void SDLRenderSystem::setStencilBufferFunction(CompareFunction func)
    {
        // Have to use saved values for other params since GL doesn't have 
        // individual setters
        mStencilFunc = convertCompareFunction(func);
        glStencilFunc(mStencilFunc, mStencilRef, mStencilMask);
    }
    //---------------------------------------------------------------------
    void SDLRenderSystem::setStencilBufferReferenceValue(ulong refValue)
    {
        // Have to use saved values for other params since GL doesn't have 
        // individual setters
        mStencilRef = refValue;
        glStencilFunc(mStencilFunc, mStencilRef, mStencilMask);
    }
    //---------------------------------------------------------------------
    void SDLRenderSystem::setStencilBufferMask(ulong mask)
    {
        // Have to use saved values for other params since GL doesn't have 
        // individual setters
        mStencilMask = mask;
        glStencilFunc(mStencilFunc, mStencilRef, mStencilMask);
    }
    //---------------------------------------------------------------------
    void SDLRenderSystem::setStencilBufferFailOperation(StencilOperation op)
    {
        // Have to use saved values for other params since GL doesn't have 
        // individual setters
        mStencilFail = convertStencilOp(op);
        glStencilOp(mStencilFail, mStencilZFail, mStencilPass);
    }
    //---------------------------------------------------------------------
    void SDLRenderSystem::setStencilBufferDepthFailOperation(StencilOperation op)
    {
        // Have to use saved values for other params since GL doesn't have 
        // individual setters
        mStencilZFail = convertStencilOp(op);
        glStencilOp(mStencilFail, mStencilZFail, mStencilPass);
    }
    //---------------------------------------------------------------------
    void SDLRenderSystem::setStencilBufferPassOperation(StencilOperation op)
    {
        // Have to use saved values for other params since GL doesn't have 
        // individual setters
        mStencilPass = convertStencilOp(op);
        glStencilOp(mStencilFail, mStencilZFail, mStencilPass);
    }
    //---------------------------------------------------------------------
    GLint SDLRenderSystem::convertCompareFunction(CompareFunction func)
    {
        switch(func)
        {
        case CMPF_ALWAYS_FAIL:
            return GL_NEVER;
        case CMPF_ALWAYS_PASS:
            return GL_ALWAYS;
        case CMPF_LESS:
            return GL_LESS;
        case CMPF_LESS_EQUAL:
            return GL_LEQUAL;
        case CMPF_EQUAL:
            return GL_EQUAL;
        case CMPF_NOT_EQUAL:
            return GL_NOTEQUAL;
        case CMPF_GREATER_EQUAL:
            return GL_GEQUAL;
        case CMPF_GREATER:
            return GL_GREATER;
        };
        // to keep compiler happy
        return GL_ALWAYS;
    }
    //---------------------------------------------------------------------
    GLint SDLRenderSystem::convertStencilOp(StencilOperation op)
    {
        switch(op)
        {
        case SOP_KEEP:
            return GL_KEEP;
        case SOP_ZERO:
            return GL_ZERO;
        case SOP_REPLACE:
            return GL_REPLACE;
        case SOP_INCREMENT:
            return GL_INCR;
        case SOP_DECREMENT:
            return GL_DECR;
        case SOP_INVERT:
            return GL_INVERT;
        };
        // to keep compiler happy
        return SOP_KEEP;
    }
    //---------------------------------------------------------------------
    void SDLRenderSystem::setStencilBufferParams(CompareFunction func, ulong refValue, 
        ulong mask, StencilOperation stencilFailOp, 
        StencilOperation depthFailOp, StencilOperation passOp)
    {
        // optimise this into 2 calls instead of many
        mStencilFunc = convertCompareFunction(func);
        mStencilRef = refValue;
        mStencilMask = mask;
        mStencilFail = convertStencilOp(stencilFailOp);
        mStencilZFail = convertStencilOp(depthFailOp);
        mStencilPass = convertStencilOp(passOp);
        glStencilFunc(mStencilFunc, mStencilRef, mStencilMask);
        glStencilOp(mStencilFail, mStencilZFail, mStencilPass);

    }
}
