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
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef __SDLRENDERSYSTEM_H__
#define __SDLRENDERSYSTEM_H__

#include "OgrePlatform.h"
#include "OgreRenderSystem.h"

#include <SDL.h>

#if OGRE_PLATFORM == PLATFORM_WIN32
#   include <windows.h>
#   include <wingdi.h>
#   include "gl.h"
#   define GL_GLEXT_PROTOTYPES
#   include "glprocs.h"
#else
#   include <GL/gl.h>
#endif

#include <GL/glu.h>

namespace Ogre {

    /**
      Implementation of SDL as a rendering system.
     */
    class SDLRenderSystem : public RenderSystem
    {
    private:

        // Allowed video modes
        SDL_Rect** mVideoModes;

        // Rendering loop control
        bool mStopRendering;

        // Array of up to 8 lights, indexed as per API
        // Note that a null value indicates a free slot
        #define MAX_LIGHTS 8
        Light* mLights[MAX_LIGHTS];

        // view matrix to set world against
        Matrix4 mViewMatrix;
        Matrix4 mWorldMatrix;
        Matrix4 mTextureMatrix;

        // XXX 8 max texture units?
        int mTextureCoordIndex[OGRE_MAX_TEXTURE_COORD_SETS];


        void initConfigOptions(void);
        void initInputDevices(void);
        void processInputDevices(void);

        void setGLLight(int index, Light* lt);
        void makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m);

        // Store last depth write state
        bool mDepthWrite;

    public:
        // Default constructor / destructor
        SDLRenderSystem();
        ~SDLRenderSystem();



        // ----------------------------------
        // Overridden RenderSystem functions
        // ----------------------------------
        /** See
          RenderSystem
         */
        const String& getName(void) const;
        /** See
          RenderSystem
         */
        ConfigOptionMap& getConfigOptions(void);
        /** See
          RenderSystem
         */
        void setConfigOption(const String &name, const String &value);
        /** See
          RenderSystem
         */
        String validateConfigOptions(void);
        /** See
          RenderSystem
         */
        RenderWindow* initialise(bool autoCreateWindow);
        /** See
          RenderSystem
         */
        void reinitialise(void); // Used if settings changed mid-rendering
        /** See
          RenderSystem
         */
        void shutdown(void);

        /** See
          RenderSystem
         */
        void startRendering(void);
        /** See
          RenderSystem
         */
        void setAmbientLight(float r, float g, float b);
        /** See
          RenderSystem
         */
        void setShadingType(ShadeOptions so);
        /** See
          RenderSystem
         */
        void setTextureFiltering(TextureFilterOptions fo);
        /** See
          RenderSystem
         */
        void setLightingEnabled(bool enabled);
        /** See
          RenderSystem
         */
        RenderWindow* createRenderWindow(const String &name, int width, int height, int colourDepth,
            bool fullScreen, int left = 0, int top = 0, bool depthBuffer = true,
            RenderWindow* parentWindowHandle = 0);

        /** See
          RenderSystem
         */
        void destroyRenderWindow(RenderWindow* pWin);
        /** See
          RenderSystem
         */
        String getErrorDescription(long errorNumber);

        /** See
          RenderSystem
         */
        void convertColourValue(const ColourValue& colour, unsigned long* pDest);

        // -----------------------------
        // Low-level overridden members
        // -----------------------------
        /** See
          RenderSystem
         */
        void _addLight(Light *lt);
        /** See
          RenderSystem
         */
        void _removeLight(Light *lt);
        /** See
          RenderSystem
         */
        void _modifyLight(Light* lt);
        /** See
          RenderSystem
         */
        void _removeAllLights(void);
        /** See
          RenderSystem
         */
        void _pushRenderState(void);
        /** See
          RenderSystem
         */
        void _popRenderState(void);
        /** See
          RenderSystem
         */
        void _setWorldMatrix(const Matrix4 &m);
        /** See
          RenderSystem
         */
        void _setViewMatrix(const Matrix4 &m);
        /** See
          RenderSystem
         */
        void _setProjectionMatrix(const Matrix4 &m);
        /** See
          RenderSystem
         */
        void _setSurfaceParams(ColourValue &ambient,
            ColourValue &diffuse, ColourValue &specular,
            ColourValue &emissive, Real shininess);
        /** See
          RenderSystem
         */
        unsigned short _getNumTextureUnits(void);
        /** See
          RenderSystem
         */
        void _setTexture(int unit, bool enabled, const String &texname);
        /** See
          RenderSystem
         */
        void _setTextureCoordSet(int stage, int index);
        /** See
          RenderSystem
         */
        void _setTextureCoordCalculation(int stage, TexCoordCalcMethod m);
        /** See
          RenderSystem
         */
        void _setTextureBlendMode(int stage, const LayerBlendModeEx& bm);
        /** See
          RenderSystem
         */
        void _setTextureAddressingMode(int stage, Material::TextureLayer::TextureAddressingMode tam);
        /** See
          RenderSystem
         */
        void _setTextureMatrix(int stage, const Matrix4& xform);
        /** See
          RenderSystem
         */
        void _setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor);
        /** See
          RenderSystem
         */
        void _setAlphaRejectSettings(CompareFunction func, unsigned char value);
        /** See
          RenderSystem
         */
        void _setViewport(Viewport *vp);
        /** See
          RenderSystem
         */
        void _beginFrame(void);
        /** See
          RenderSystem
         */
        void _render(RenderOperation& op);
        /** See
          RenderSystem
         */
        void _endFrame(void);
        /** See
          RenderSystem
         */
        void _setCullingMode(CullingMode mode);
        /** See
          RenderSystem
         */
        void _setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL);
        /** See
          RenderSystem
         */
        void _setDepthBufferCheckEnabled(bool enabled = true);
        /** See
          RenderSystem
         */
        void _setDepthBufferWriteEnabled(bool enabled = true);
        /** See
          RenderSystem
         */
        void _setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL);
        /** See
          RenderSystem
         */
        void _setFog(FogMode mode, ColourValue colour, Real density, Real start, Real end);
        /** See
          RenderSystem
         */
        void _makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, Matrix4& dest);
        /** See
          RenderSystem
         */
        void _setRasterisationMode(SceneDetailLevel level);
        // ----------------------------------
        // End Overridden members
        // ----------------------------------



    };
}
#endif

